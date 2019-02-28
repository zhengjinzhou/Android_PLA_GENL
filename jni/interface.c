/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "clientlink.h"
#include "VideoInstance.h"
#include "st_cuapi.h"
#include "playlist.h"
#include "playlib.h"
#include "mp4v2export.h"
#include "g711.h"

#include <jni.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <android/log.h>
#include <android/bitmap.h>

#define	null 0
#define	LOG_TAG    "interface.c"
#define	LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define	LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef unsigned int size_t;

//the native library instance
int pLibInstParam = 0;

static int iAreaNodeCnt;
static int iDevNodeCount;

#define AREA_GROUP_NUM	100			//max area node number
#define CHILD_NODE_NUM	200			//max child node number
#define MAX_NODE_NUM 	2000		//max device number

AREA_LIST *pAreaList = NULL;
DEVICE_LIST *pDeviceList = NULL;

#define MAX_FRAMESIZE (1920*1080*3)	//the max buffer to store RGB565 format BMP
void *g_videoInstArray[MAX_ITEM];	//the video instance array for multiple video player
void *g_frameBufferOut[MAX_ITEM];	//every video instance need a frame buffer to store the decode data

/**
 * Every video instance need a keyFrame buffer for record used.
 * And we just need the head of the frame, it contain what out needed.
 * */
char keyFrameBuf[FRAME_MAX_SIZE_T];
int isSetKeyFrame;
int keyFrameLen;

/**
 * Every video instance need a audioFrame buffer for record used.
 * And we just need the head of the frame, it contain what out needed.
 * */
char audioHead[32];

static	int  g_AlarmDevID = 0;

typedef struct __audio_pk_t
{
    int		pkt_len;
    char	hisi[4];
    char	data[0];
}audio_pk_t;


static int data_Callback(void* context, void *buf, int size, void* handle)
{
	return st_inputFrameListTail( (tp_playlistinst_t *)handle, (char *) buf, size);
}

/**
 * Video Record_CallBack function
 * */
static void mp4Record_cb(void* fileHandle, int track_video, int track_audio,
		void *databuf, int size, int keyframe)
{  
	/**
	 * offset to remove the tp_header, sizeof(tp_head) = 32.
	 * */
	char *pData = (char*)databuf;
	pData = &pData[32];
	size = size - 32;

	if(SANTACHI_I_FRAME_TYPE == keyframe){
		//int sp1;
		//memcpy(&sp1, &pData[4], sizeof(int));
		//LOGI("key sp1 = %#010x", sp1);
		//MP4WriteSample(fileHandle, track_video, pData, size, MP4_INVALID_DURATION, 0, 1);
		tp_mp4_write_videoFrame(fileHandle, track_video, pData, size, 1);
	}
	else if(SANTACHI_P_FRAME_TYPE == keyframe){
		if(0 == isSetKeyFrame){
			tp_mp4_write_videoFrame(fileHandle, track_video, keyFrameBuf, keyFrameLen, 1);
			isSetKeyFrame = 1;
		}
		else
			tp_mp4_write_videoFrame(fileHandle, track_video, pData, size, 0);
	}
	else if(SANTACHI_A_FRAME_TYPE == keyframe)
	{
	    int	pkt_cnt;
	    memcpy(&pkt_cnt, pData, sizeof(int));
	    audio_pk_t *a_pkt = (audio_pk_t *)(pData + sizeof(int));
	    while(pkt_cnt-- > 0)
	    {
	    	if(RetFalse == tp_mp4_write_audioFrame(fileHandle, track_audio,
	    			a_pkt->data, a_pkt->pkt_len-4, 1))
	    	{
	            LOGE("MP4WriteAudioSample fail ...");
	            return ;
	        }
	        a_pkt += a_pkt->pkt_len + sizeof(int);
	    }
	}
}

jint Java_com_topvs_platform_LibAPI_InitLibInstance(JNIEnv* env, jclass jobject)
{
	if (init_ffmpeg_lib() != 0){
		LOGE("init_ffmpeg_lib failed.");
		return -1;
	}

	pLibInstParam = InitilLibInstance();

	int viindex = 0;
	while(viindex < MAX_ITEM){
		g_videoInstArray[viindex] = NULL;
		g_frameBufferOut[viindex] = NULL;
		viindex++;
	}

	return pLibInstParam;
}

jint Java_com_topvs_platform_LibAPI_DeleteLibInstance(JNIEnv* env, jclass jobject)
{
	int ret = -1;

	if(pAreaList){
		free(pAreaList);
		pAreaList = NULL;
	}

	if(pDeviceList != NULL){
		if(pDeviceList->nodearray != NULL){
			free(pDeviceList->nodearray);
			pDeviceList->nodearray = NULL;
		}
		free(pDeviceList);
		pDeviceList = NULL;
	}

	if (pLibInstParam != 0)
		ret = DeleteLibInstance(pLibInstParam);

	return ret;
}

jint Java_com_topvs_platform_LibAPI_RequestLogout(JNIEnv* env, jclass jobject)
{
	return RequestLogout(pLibInstParam);
}

jint Java_com_topvs_platform_LibAPI_RequestLogin(JNIEnv* env, jclass jobject, jstring ip_addr, jint port, jstring user_name,
		jstring userpasword)
{
	int m_dwTimeout = 6000; //login timeout LinZh107
	int nLoginedNum = 0;
	char szDomanid[32] = { 0 };
	char szRemoteIP[128] = { 0 };

	jboolean iscopy = JNI_TRUE;
	const char *addr = (*env)->GetStringUTFChars(env, ip_addr, &iscopy);
	const char *name = (*env)->GetStringUTFChars(env, user_name, &iscopy);
	const char *password = (*env)->GetStringUTFChars(env, userpasword, &iscopy);

	SetCmdTimeOut(pLibInstParam, m_dwTimeout);

	int ret = RequestLogin(pLibInstParam, addr, port, name, password, szDomanid, szRemoteIP, &nLoginedNum);
	if(ret == 0)
		ret = SubNotifyInfo(pLibInstParam);  //add notify information alarm ,success ret == 0
	return ret;
}

//get ack cmd code
jint Java_com_topvs_platform_LibAPI_GetCheckCode(JNIEnv* env, jclass jobject, jstring ip_addr, jint port, jstring user_name,
		jstring userpasword)
{
	jboolean iscopy = JNI_TRUE;
	const char *addr = (*env)->GetStringUTFChars(env, ip_addr, &iscopy);
	const char *name = (*env)->GetStringUTFChars(env, user_name, &iscopy);
	const char *password = (*env)->GetStringUTFChars(env, userpasword, &iscopy);
	SetCmdTimeOut(pLibInstParam, 10000);
	return GetCheckCode(pLibInstParam, addr, port, name, password);
}

#define _mark_c0 0

//login with ack code
jint Java_com_topvs_platform_LibAPI_RequestLoginEx(JNIEnv* env, jclass jobject, jstring ip_addr, jint port, jstring user_name,
		jstring userpasword, jstring checkcode)
{
	int m_dwTimeout = 10000;
	int nLoginedNum = 0;
	char szDomanid[32] = { 0 };
	char szRemoteIP[128] = { 0 };

	jboolean iscopy = JNI_TRUE;
	const char *addr = (*env)->GetStringUTFChars(env, ip_addr, &iscopy);
	const char *name = (*env)->GetStringUTFChars(env, user_name, &iscopy);
	const char *password = (*env)->GetStringUTFChars(env, userpasword, &iscopy);
	const char *code = (*env)->GetStringUTFChars(env, checkcode, &iscopy);

	SetCmdTimeOut(pLibInstParam, m_dwTimeout);
	return RequestLoginEx(pLibInstParam, addr, port, name, password, szDomanid, szRemoteIP, &nLoginedNum, code);
}

static void InsertNodeToList(DEVICE_NODE * srcList, int srcIndex,
		AREA_LIST *areaList, int areaIndex, int nodeType)
{
	areaList[areaIndex].childNum = 0;
	if(nodeType == 0){
		sprintf(areaList[areaIndex].AreaID, "%s", srcList[srcIndex].DomainID);
		int ret = sprintf(areaList[areaIndex].AreaName, "%s", srcList[srcIndex].DomainName);
		if(ret <= 0)
			sprintf(areaList[areaIndex].AreaName, "%s", srcList[srcIndex].AreaName);
	}
	else if(nodeType == 1){
		sprintf(areaList[areaIndex].AreaID, "%s", srcList[srcIndex].AreaID);
		sprintf(areaList[areaIndex].AreaName, "%s", srcList[srcIndex].AreaName);
	}
	return ;
}

static void GetGUInfosList(const char* queryID, int iType) //LinZh107
{
	LOGI("Call GetGUInfosList function");
	int ret = 0;
    int iNodeCnt = 0;
    DEVICE_NODE * tmpList = NULL;

    //indicated the top level , also is the domain level
    if(queryID == NULL && iType == 0)
    {
        tmpList = (DEVICE_NODE*) malloc(sizeof(DEVICE_NODE) * AREA_GROUP_NUM);
        if(tmpList == NULL)
        	return;
        memset(tmpList, 0, sizeof(DEVICE_NODE) * AREA_GROUP_NUM);
        ret = GetDeviceList(pLibInstParam, queryID, iType, tmpList, AREA_GROUP_NUM, &iNodeCnt);
        if (ret < 0 || iNodeCnt <= 0)
        {
            if (NULL != tmpList){
                free(tmpList);
                tmpList = NULL;
            }
            return;
        }

        //create the default area from the domain info, to store the device below the domain level.
        int defaultGtoupIndex = iAreaNodeCnt;
        InsertNodeToList(tmpList, 0, pAreaList, defaultGtoupIndex, 0);
		iAreaNodeCnt++ ;

		int i;
		for (i=0; i<iNodeCnt; i++)
		{
			switch (tmpList[i].nType)
			{
			//case 0:	//save the domain node to area list
			//	if(iAreaNodeCnt < AREA_GROUP_NUM)
			//	{
			//		InsertNodeToList(tmpList, i, pAreaList, iAreaNodeCnt, 0);
			//		iAreaNodeCnt++ ;
			//	}
			//	break;
			case 1:	//save the area node to area list
				if(iAreaNodeCnt < AREA_GROUP_NUM)
				{
					InsertNodeToList(tmpList, i, pAreaList, iAreaNodeCnt, 1);
					iAreaNodeCnt++ ;
					LOGI("[%2d] :AreaID=%s, AreaName=%s\n", i,	tmpList[i].AreaID, tmpList[i].AreaName);
				}
				break;
			case 2:	//save device node to the list
				pAreaList[defaultGtoupIndex].childNum ++;
				memcpy(&pDeviceList->nodearray[iDevNodeCount],  &tmpList[i], sizeof(DEVICE_NODE));
				iDevNodeCount ++;
				//LOGI("case 2: tmpList[%2d] :GUName=%s\n", i, tmpList[i].guInfo.GUName);
				break;
			default :
				break;
			}
		}
    }
    //indicated the second level , also is the area level
    else if(queryID != NULL && iType == 1)
    {
        tmpList = (DEVICE_NODE*) malloc(sizeof(DEVICE_NODE) * CHILD_NODE_NUM);
        if(tmpList == NULL)
            return;
        memset(tmpList, 0, sizeof(DEVICE_NODE) * CHILD_NODE_NUM);

        // get device node from XML
        ret = GetDeviceList(pLibInstParam, queryID, iType, tmpList, CHILD_NODE_NUM, &iNodeCnt);
        if (ret < 0 || iNodeCnt <= 0)
        {
            if (NULL != tmpList)
            {
                free(tmpList);
                tmpList = NULL;
            }
            return;
        }

        //set the AreaList's every child count to iNodeCnt
        int AListIndex = atoi(tmpList[0].AreaID);
        pAreaList[AListIndex].childNum = iNodeCnt;

        //save device node to the list
		memcpy(&pDeviceList->nodearray[iDevNodeCount],  tmpList,  sizeof(DEVICE_NODE) * iNodeCnt);
		iDevNodeCount += iNodeCnt;
    }

    int i;
    for (i = 1; i < iNodeCnt; i++)
    {
        switch (tmpList[i].nType)
        {
            case 0:
                GetGUInfosList(tmpList[i].DomainID, 0);
                break;
            case 1:
                GetGUInfosList(tmpList[i].AreaID, 1);
                break;
            default:
                break;
        }
    }

    if (NULL != tmpList)
    {
        free(tmpList);
        tmpList = NULL;
    }
}

//jobjectArray Java_com_topvs_platform_LibAPI_GetDeviceList(JNIEnv* env, jclass jobj, jintArray pOutArray)
int Java_com_topvs_platform_LibAPI_GetDeviceList(JNIEnv* env, jclass jobj,
	jintArray DevNumArray, jintArray CamStatusArray, jintArray SWiStatusArray, jintArray SWoStatusArray,
	jintArray AreaChildArray, jobjectArray ArrayAreaStr, jobjectArray ArrayGUID, jobjectArray ArrayGUName)
{
	jstring str = NULL;
	jclass strCls = (*env)->FindClass(env, "java/lang/String");
	jint *DevArray = (*env)->GetIntArrayElements(env, DevNumArray, 0);
	jint *AreaArray = (*env)->GetIntArrayElements(env, AreaChildArray, 0);
	jint *CamStatus = (*env)->GetIntArrayElements(env, CamStatusArray, 0);
	jint *SWiStatus = (*env)->GetIntArrayElements(env, SWiStatusArray, 0);
	jint *SWoStatus = (*env)->GetIntArrayElements(env, SWoStatusArray, 0);

	/////////////////////////////////////////////////////////////////////
	char *GUID[MAX_NODE_NUM];
	char *GUName[MAX_NODE_NUM];
	iAreaNodeCnt = 0;
	iDevNodeCount = 0;

	//create areaList to store the area and domain node.
	pAreaList = (AREA_LIST*)malloc(sizeof(AREA_LIST) * AREA_GROUP_NUM);
	memset(pAreaList, 0, sizeof(AREA_LIST) * AREA_GROUP_NUM);

	//create deviceList to store the device node.
	pDeviceList = (DEVICE_LIST*)malloc(sizeof(*pDeviceList));
	memset(pDeviceList, 0, sizeof(*pDeviceList));
	pDeviceList->nodearray = (DEVICE_NODE*)malloc(sizeof(DEVICE_NODE) * MAX_NODE_NUM);
	memset(pDeviceList->nodearray, 0, sizeof(DEVICE_NODE) * MAX_NODE_NUM);

	//get data from server, param null & 0 will indicated the first query
	GetGUInfosList(NULL, 0);

	//copy device status array to java devinfoArray
	int i, j=0, k=0, l=0;
	for (i = 0; i < iDevNodeCount; i++)
	{
		DEVICE_NODE *pNpde = &pDeviceList->nodearray[i];
		GUINFO *pInfo = &pNpde->guInfo;

		switch(pNpde->nType)
		{
		#if 0
			case 0: //Domain
			{
				CamStatus[j] = 0x1000;
				sprintf(GUName[j], "[%s]", pNpde->DomainName);
				j++;
				LOGI("GetDeviceList Domain i = %d",i);
				break;
			}
			case 1: //Area
			{
				CamStatus[j] = 0x1001;
				GUName[j] = malloc(sizeof(char) * 128);
				memset(GUName[j], 0, 128);
				sprintf(GUName[j], "[%s]", (unsigned char*)pNpde->AreaName);
				LOGI("GetDeviceList Area i=%d, GUName[j]=%s", i, GUName[j]);
				j++;
				break;
			}
		#endif
			case 2: //device
			{
				if(pInfo->GUType == GU_TYPE_AV_MASTER
					|| pInfo->GUType == GU_TYPE_AV_SLAVE)//video stream
				{
					if ( pInfo->bState == 1)
						CamStatus[j] = i;       //online
					else
						CamStatus[j] = 0x1002;  //off line

					GUName[j] = malloc(sizeof(char) * 128);
					memset(GUName[j], 0, 128);
					sprintf(GUName[j], "%s",(unsigned char*)pInfo->GUName);

					GUID[j] = malloc(sizeof(char) * 64);
					memset(GUID[j], 0, 64);
					sprintf(GUID[j], "%s",(unsigned char*)pInfo->GUID);
					//LOGI("GetDeviceList DevNode i=%d, GUName[j]=%s", i, GUName[j]);
					j++;
				}
				////////////////////////////////////////////
				else if(pInfo->GUType == GU_TYPE_ALARM_INPUT)
				{
					SWiStatus[k] = i;
					k++;
				}
				else if(pInfo->GUType == GU_TYPE_ALARM_OUTPUT)
				{
					SWoStatus[l] = i;
					l++;
				}
				else
					LOGI("GetDeviceList DevNode not valid");
				break;
			}
			default:
				break;
		}
	}

	DevArray[0] = j;
	DevArray[1] = k;
	DevArray[2] = l;

	//copy area info and child number array to java AreaArray
	//the first(index=0) indicated the domain info, and the real area info will begin from the second member
	AreaArray[100] = iAreaNodeCnt;
	for(i = 0; i < iAreaNodeCnt; i++)
	{
		AreaArray[i] = pAreaList[i].childNum;
		str = (*env)->NewStringUTF(env, pAreaList[i].AreaName);
		(*env)->SetObjectArrayElement(env, ArrayAreaStr, i, str);
		(*env)->DeleteLocalRef(env, str);
	}

	//copy device info to java AreaArray
	for(i = 0; i < j; i++)
	{
		str = (*env)->NewStringUTF(env, GUName[i]);
		(*env)->SetObjectArrayElement(env, ArrayGUName, i, str);
		(*env)->DeleteLocalRef(env, str);
		free(GUName[i]);

		str = (*env)->NewStringUTF(env, GUID[i]);
		(*env)->SetObjectArrayElement(env, ArrayGUID, i, str);
		(*env)->DeleteLocalRef(env, str);
		free(GUID[i]);
	}

	(*env)->ReleaseIntArrayElements(env, DevNumArray, DevArray, 0);
	(*env)->ReleaseIntArrayElements(env, AreaChildArray, AreaArray, 0);
	(*env)->ReleaseIntArrayElements(env, CamStatusArray, CamStatus, 0);
	(*env)->ReleaseIntArrayElements(env, SWiStatusArray, SWiStatus, 0);
	(*env)->ReleaseIntArrayElements(env, SWoStatusArray, SWoStatus, 0);
}

jint Java_com_topvs_platform_LibAPI_StartPlay(JNIEnv* env, jclass jobject, jint iNodeIndex, jint viindex)
{
	//LOGI("\n----> Create and Start viindex = %d",viindex);
	tp_playlistinst_t *playinst = malloc(sizeof(tp_playlistinst_t));
	memset(playinst, 0, sizeof(tp_playlistinst_t));

	st_initFrameList(playinst);

	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));

	CLIENTINFO iclient;
	iclient.lpFunc = data_Callback; //lpFunc;
	iclient.lHandle = NULL;
	iclient.nProtocolType = 1; 		//TCP
	iclient.playinst = playinst;	//add by LinZh107 for multiplayView
	//memcpy(&iclient.guInfo, g_GuInfoArray[iNodeIndex], sizeof(GUINFO));
	memcpy(&iclient.guInfo, &pDeviceList->nodearray[iNodeIndex].guInfo, sizeof(GUINFO));

	if(g_videoInstArray[viindex] != NULL)
		return 0;

	g_videoInstArray[viindex] = CreateVideoInstance(pLibInstParam, iclient);
	if(g_videoInstArray[viindex] == NULL){
		LOGE("g_videoInstArray[viindex=%d](NULL): -2",viindex);
		return -2;
	}

	VideoInstance_t *pInst = (VideoInstance_t*)g_videoInstArray[viindex];
	pthread_mutex_init(&pInst->dataList_mutex, NULL);

	if(g_frameBufferOut[viindex] == NULL){
		g_frameBufferOut[viindex] = (void*)malloc(MAX_FRAMESIZE * sizeof(char));
		memset(g_frameBufferOut[viindex], 0, MAX_FRAMESIZE * sizeof(char));
	}

	// ask MDU information from CMU
	int ret = QueryPuRealRoute(pLibInstParam, iclient.guInfo, &mduInfo);
	if (ret < 0){
		LOGE("QueryPuRealRoute return :%d, viindex=%d", ret, viindex);
		return ret;
	}

	//2014-6-4 LinZh107
	ret = RealVideoPreviewStart(pLibInstParam, pInst, mduInfo);
	if (ret < 0)
	{ 
		LOGE("RealVideoPreviewStart: %d, viindex=%d",ret, viindex);
		return ret;
	}

	// initialize the ffmpeg decoder
	return init_decoder(viindex);
}

jint Java_com_topvs_platform_LibAPI_StopPlay(JNIEnv* env, jclass jobject, jint viindex)
{
	//LOGI("StopPlay have been call %d.",viindex);
	if(g_videoInstArray[viindex] == NULL)
		return 0;

	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];

	RealVideoPreviewStop(pLibInstParam, pVInst);

	pthread_mutex_lock(&pVInst->dataList_mutex);

	CancelVideoInstance(pLibInstParam, pVInst);

	st_destroyFrameList(pVInst->playinst);
	pVInst->playinst = NULL;

	pthread_mutex_unlock(&pVInst->dataList_mutex);
	pthread_mutex_destroy(&pVInst->dataList_mutex);

	free(pVInst);
	g_videoInstArray[viindex] = NULL;

	//release the ffmpeg decoder
	release_decoder(viindex);

	if(NULL != g_frameBufferOut[viindex]){
		free(g_frameBufferOut[viindex]);
		g_frameBufferOut[viindex] = NULL;
	}

	//LOGI("StopPlay have been call %d...",viindex);
	return 0;
}

jint Java_com_topvs_platform_LibAPI_GetFrameNum(JNIEnv* env, jclass jobject, jint viindex)
{
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	return getFrameNum(pVInst->playinst, VIDEO_FRAME);
}

//Data: 2014-5-8 modify by LinZh107
jint Java_com_topvs_platform_LibAPI_GetVideoFrame(JNIEnv* env, jclass jobj,
		jintArray pOutArray, jobject bitmap, jint viindex)
{
	int iRet = -1;
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	if(!pVInst)
		return iRet;

	pthread_mutex_lock(&pVInst->dataList_mutex);

	//LOGI("in LibAPI getVideoFrame");
	Hi3510_frame_t *pVideoFrame = st_getFrameListHead(pVInst->playinst, VIDEO_FRAME);
	if (pVideoFrame == NULL || pVideoFrame->data == NULL){
		//LOGE("pVideoFrame->data == NULL %d",viindex);
		pthread_mutex_unlock(&pVInst->dataList_mutex);
		return iRet;
	}

	jint *outArray = (*env)->GetIntArrayElements(env, pOutArray, 0);

	//begin to decode the frame
	int ret = decode_H264_frame(pVideoFrame->frame_rate, pVideoFrame->frame_size,
			pVideoFrame->data, outArray, g_frameBufferOut[viindex], viindex);
	if (ret != 0){
		//LOGE("%d decodeH264video failed with %d",viindex, ret);
		iRet = -4;
		goto error;
	}

	//save key frame for the record work
	if(pVideoFrame->frame_type == 1 && pVInst->m_bRecord == 0){
		keyFrameLen = pVideoFrame->frame_size;
		memcpy(keyFrameBuf, (char*)pVideoFrame->data, keyFrameLen);
	}

	//finish decoding the frame
	AndroidBitmapInfo bmpinfo;
	if (AndroidBitmap_getInfo(env, bitmap, &bmpinfo) < 0)
	{
		LOGE("AndroidBitmap_getInfo() failed !");
		iRet = -2;
		goto error;
	}

	void* pixels;
	if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0)
	{
		//LOGE("AndroidBitmap_lockPixels() failed !");
		iRet = -3;
		goto error;
	}

	//copy RGB565 to a bitmap
	uint8_t *frameLine;
	int nlist = 0;
	int size_char = sizeof(char);
	while(nlist < bmpinfo.height)
	{
		if(nlist*outArray[0]*2 > MAX_FRAMESIZE*size_char)
			break;

		frameLine = (uint8_t *) g_frameBufferOut[viindex] + (nlist * outArray[0]*2);
		//RGB565 must multiply 2, while RGB24 x 3
		memcpy((char*)pixels, frameLine, bmpinfo.stride/*info.width * 2*/);

		pixels += bmpinfo.stride;
		nlist++;
	}
	AndroidBitmap_unlockPixels(env, bitmap);

	iRet = pVideoFrame->frame_rate;
	if(pVInst->bInit_params == 0 && pVideoFrame->frame_type == 1)
	{
		pVInst->video_width = outArray[0];
		pVInst->video_height = outArray[1];
		pVInst->frame_rate = pVideoFrame->frame_rate;
		memcpy(pVInst->sps_pps, &pVideoFrame->data[4], 32);

		pVInst->bInit_params = 1;
	}

error:
	st_removeFrameListHead(pVInst->playinst, VIDEO_FRAME);
	pthread_mutex_unlock(&pVInst->dataList_mutex);
	(*env)->ReleaseIntArrayElements(env, pOutArray, outArray, 0);
	return iRet;
}

//2014-5-29 add_by linzh107
jint Java_com_topvs_platform_LibAPI_GetAudioParam(JNIEnv *env, jobject obj,
		jintArray pOutArray, jint viindex)
{
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	if(!pVInst)
		return -1;

	Hi3510_frame_t *pAudioFrame = st_getFrameListHead(pVInst->playinst, AUDIO_FRAME);
	if (pAudioFrame == NULL || pAudioFrame->data == NULL)
	{
		//LOGE("GetAudioFrame pAudioFrame->data == NULL");
		return -1;
	}
	char *pAudioData = pAudioFrame->data;
	int iPackNum = (int)(pAudioData[0] | pAudioData[1]<<8 | pAudioData[2]<<16 | pAudioData[3]<<24);
	//LOGI("GetAudioParam iPackNum = %d",iPackNum);
	int iPackLen = (int)(pAudioData[4] | pAudioData[5]<<8 | pAudioData[6]<<16 | pAudioData[7]<<24);
	if (iPackLen < 0 || iPackLen > HI_VOICE_MAX_FRAME_SIZE)
		return -2;
	int iDataLen = iPackLen - 4; //HASI frame pure data length
	//LOGI("GetAudioParam iDataLen = %d",iDataLen);

	jint *intArray = (*env)->GetIntArrayElements(env, pOutArray, 0);
	intArray[0] = iPackNum;
	intArray[1] = iDataLen;
	(*env)->ReleaseIntArrayElements(env, pOutArray, intArray, 0);

	return 0;
}

//add at 2014-5-30 LinZh107
jshortArray Java_com_topvs_platform_LibAPI_GetAudioFrame(JNIEnv *env, jobject obj,
		jint frame_cnt, jint viindex)
{
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	if(!pVInst)
		return NULL;

	Hi3510_frame_t *pAudioFrame = st_getFrameListHead(pVInst->playinst, AUDIO_FRAME);
	//LOGI("Hi3510_frame_t *pAudioFrame!");
	if (pAudioFrame == NULL || pAudioFrame->data == NULL)
	{
		//LOGE("GetAudioFrame pAudioFrame->data == NULL");
		return NULL;
	}
	unsigned char *pAudioData = pAudioFrame->data;

	int iPackNum = (int)(pAudioData[0] | pAudioData[1]<<8 | pAudioData[2]<<16 | pAudioData[3]<<24);
		//LOGI("GetAudioFrame iPackNum=%d",iPackNum);
	int iPackLen = (int)(pAudioData[4] | pAudioData[5]<<8 | pAudioData[6]<<16 | pAudioData[7]<<24);
	if (iPackLen < 0 || iPackLen > HI_VOICE_MAX_FRAME_SIZE)
		return NULL;

	int iDataLen = iPackLen - 4;
	//LOGI("GetAudioFrame iDataLen=%d",iDataLen);
	int dataChunkLen = iPackNum * iDataLen;
	int buffersize = dataChunkLen * frame_cnt;
	unsigned char *bArray = malloc(buffersize);
	if(bArray==NULL)
		return NULL;
	int cpySize = 0;
	int index = 0;
	pAudioData += 12;
	while(index < frame_cnt)
	{
		if(!pVInst || !pVInst->playinst){
			LOGE("APP logout cause the decode audio function exit!");
			return -1;
		}

		if((buffersize - cpySize) >= dataChunkLen && pAudioFrame != NULL)
		{
			int i = 0;
			while( i < iPackNum )
			{
				memcpy(bArray + cpySize, pAudioData, iDataLen);
				cpySize += iDataLen;
				pAudioData += iDataLen + 8;
				i++;
			}
			st_removeFrameListHead(pVInst->playinst, AUDIO_FRAME);
		}

		pAudioFrame = st_getFrameListHead(pVInst->playinst, AUDIO_FRAME);
		if(pAudioFrame != NULL && pAudioFrame->data != NULL)
		{
			pAudioData = pAudioFrame->data;
			pAudioData += 12;
			index++;
		}
		else
		{
			//LOGE("wait for data !");
			usleep(5000);
		}
	}

	int shortArrsize = cpySize;
	jshortArray sArray = (*env)->NewShortArray(env, shortArrsize);
	short *pArray = (short*)malloc(shortArrsize * sizeof(jshort));
	if(pArray ==NULL)
		return NULL;

	index = 0;
	switch (pAudioFrame->video_standard)
	{
	case AUDIO_FORMAT_G711A:
		while (index < shortArrsize){
			pArray[index] = Snack_Alaw2Lin(bArray[index]);				//g711.h
			index++;
		}
		break;
	case AUDIO_FORMAT_G711Mu:
		while (index < shortArrsize){
			pArray[index] = Snack_Mulaw2Lin(bArray[index]);				//g711.h
			index++;
		}
		break;
	case AUDIO_FORMAT_ADPCM:
		while (index < shortArrsize){
			pArray[index] = 0; //unable decode
			index++;
		}
		break;
	case AUDIO_FORMAT_G726_16:
		while (index < shortArrsize){
			pArray[index] = 0; //unable decode
			index++;
		}
		break;
	case AUDIO_FORMAT_AMR:
		while (index < shortArrsize){
			pArray[index] = 0; //unable decode
			index++;
		}
		break;
	default:
		break;
	}

	//LOGI("GetAudioFrame cpySize = %d",cpySize);
	(*env)->SetShortArrayRegion(env, sArray, 0, shortArrsize, pArray);

	free(bArray);
	free(pArray);
	return sArray;
}

#define _mark_c1	0

jint Java_com_topvs_platform_LibAPI_StartRecord(JNIEnv* env, jclass jobject, jstring filePath, jint viindex)
{
	LOGI("++++ Try to start the video Recorder.");
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	if(!pVInst)
		return -1;

	isSetKeyFrame = 0;

	//init the recorder and output file
	jboolean iscopy = JNI_TRUE;
	const char* path = (*env)->GetStringUTFChars(env, filePath, &iscopy);

	int ret = tp_mp4_create_mp4file(path, g_videoInstArray[viindex]);
	if(ret == RetTrue){
		tp_mp4_set_audio_ulawPro(pVInst->record_file, pVInst->track_audio);
		pVInst->rcMp4Func = (RECORDCALLBACK)mp4Record_cb;
		pVInst->m_bRecord = 1;
	}
	(*env)->ReleaseStringUTFChars(env, filePath, path);

	return ret;
}

jint Java_com_topvs_platform_LibAPI_StopRecord(JNIEnv* env, jclass jobject, jint viindex)
{
	LOGI("---- Try to stop the video Recorder.");
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	if(!pVInst)
		return -1;

	if(pVInst->m_bRecord == 1)
	{
		pVInst->m_bRecord = 0;
		usleep(10*1000);
		tp_mp4_finish_mp4file(pVInst->record_file);
		//CloseMp4File(pVInst->pFmtContext);
	}

	return 0;
}

#if 0
jint Java_com_topvs_platform_LibAPI_StartPlayRecord(JNIEnv* env, jclass jobj,
		jstring filePath, jintArray pOutArray)
{
	//LOGI("startPlayRecord start");
	jboolean iscopy = JNI_TRUE;
	const char *path = (*env)->GetStringUTFChars(env, filePath, &iscopy);
	record_file = open(path, O_RDONLY);
	if (record_file != 0)
		return -1;

	frame_head_t head;
	int headsize = sizeof(frame_head_t);
	int readsize = read(record_file, &head, headsize);
	if (readsize != headsize)
	{
		//if (feof(record_file)!=0)
		//	return -2;//reach the end of file
		return -1;
	}
	jint *intArray = (*env)->GetIntArrayElements(env, pOutArray, 0);
	intArray[0] = head.video_resolution;
	intArray[1] = head.frame_rate;
	(*env)->ReleaseIntArrayElements(env, pOutArray, intArray, 0);
	lseek(record_file, 0, SEEK_SET);

	return 0;
}

//modify  2014-5-23  LinZh107
jint Java_com_topvs_platform_LibAPI_GetRecordFrame(JNIEnv* env, jclass jobj,
		jbooleanArray isExit, jint viindex)
{
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	if(!pVInst)
		return -1;

	int ret = 0;
	int readn = 0;
	if (record_file == -1)
		return -1;

	frame_head_t pHead;
	int headsize = sizeof(frame_head_t);

	readn = read(record_file, &pHead, headsize);
	if (readn != headsize)
	{
		//if (feof(record_file)!=0)
		//	return -2;//reach the end of file
		return -2;
	}

	char *pFrame = malloc((headsize+pHead.frame_size) * sizeof(char));
	if (pFrame == NULL)
		return -3;
	memcpy(pFrame, (char*)&pHead, headsize);

	readn = read(record_file, pFrame+headsize, pHead.frame_size);
	if (readn != pHead.frame_size)
	{
		free(pFrame);
		return -4;
	}
	//LOGI("frame_type=%d, size=%d, rate=%d",
	//pHead.frame_type, pHead.frame_size,  pHead.frame_rate);

	jboolean *bisExitAddr = (*env)->GetBooleanArrayElements(env, isExit, 0);
	while( !bisExitAddr[0] )	//list is full, wait for space after decode is done
	{
		ret = st_inputFrameListTail(pVInst->playinst, pFrame, headsize+pHead.frame_size);
		if(ret != 0)
		{
			usleep(50*1000);
			//LOGE("input frame to tail failed!");
		}
		else
			break;
	}
	(*env)->ReleaseBooleanArrayElements(env, isExit, bisExitAddr, 0);
	return ret;
}

jint Java_com_topvs_platform_LibAPI_StopPlayRecord(JNIEnv* env, jclass jobj, jint viindex)
{
	VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
	if(!pVInst)
		return -1;

	st_destroyFrameList(pVInst->playinst);
	if (record_file == 0)
		return -1;
	close(record_file);
	return 0;
}

#else

jint Java_com_topvs_platform_LibAPI_OpenRecordFile(JNIEnv* env, jclass jobj,
		jstring filePath, jint viindex)
{
	//LOGI("startPlayRecord start");
	jboolean iscopy = JNI_TRUE;
	const char *path = (*env)->GetStringUTFChars(env, filePath, &iscopy);

	void *trparam = malloc(sizeof(void*));
	if(RetFalse == tp_mp4_open_mp4file(path, &trparam))
		return -1;

	if(0 != init_decoder(viindex))
		return -1;

	if(g_frameBufferOut[viindex] == NULL){
		g_frameBufferOut[viindex] = (void*)malloc(MAX_FRAMESIZE * sizeof(char));
		memset(g_frameBufferOut[viindex], 0, MAX_FRAMESIZE * sizeof(char));
	}

	return (int)trparam;
}

jint Java_com_topvs_platform_LibAPI_GetRecordVideoFrame(JNIEnv* env, jclass jobj,
		jint params, jintArray pOutArray, jobject bitmap, jint viindex)
{
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;

	int nSize = MAX_VIDEO_FRAME_SIZE;
	int iRet = tp_mp4_read_videoFrame((void*)params, &nSize);
	if(iRet != RetTrue)
		return iRet;

	jint *outArray = (*env)->GetIntArrayElements(env, pOutArray, 0);

	//begin to decode the frame
	iRet = decode_H264_frame(25/*frame_rate*/, nSize,
			trparam->vBuffer, outArray, g_frameBufferOut[viindex], viindex);

	if (iRet != 0 || outArray[0] <= 0 || outArray[1] <= 0){
		//LOGE("%d decodeH264video failed with %d", viindex, iRet);
		iRet = RetFalse;
		goto ret_out;
	}

	AndroidBitmapInfo bmpinfo;
	if (AndroidBitmap_getInfo(env, bitmap, &bmpinfo) < 0)
	{
		LOGE("AndroidBitmap_getInfo() failed !");
		iRet = RetFalse;
		goto ret_out;
	}

	void* pixels;
	if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0)
	{
		LOGE("AndroidBitmap_lockPixels() failed !");
		iRet = RetFalse;
		goto ret_out;
	}

	//copy RGB565 to a bitmap
	uint8_t *frameLine;
	int nlist = 0;
	int size_char = sizeof(char);
	while(nlist < bmpinfo.height)
	{
		if(nlist*outArray[0]*2 > MAX_FRAMESIZE * size_char)
			break;

		frameLine = (uint8_t *) g_frameBufferOut[viindex] + (nlist * outArray[0]*2);
		memcpy((char*)pixels, frameLine, bmpinfo.stride);

		pixels += bmpinfo.stride;
		nlist++;
	}
	AndroidBitmap_unlockPixels(env, bitmap);

	iRet = RetTrue;

ret_out:
	(*env)->ReleaseIntArrayElements(env, pOutArray, outArray, 0);
	return iRet;
}

jshortArray Java_com_topvs_platform_LibAPI_GetRecordAudioFrame(JNIEnv* env, jclass jobj,
		jint params, jint frame_cnt, jint viindex)
{
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;
	int iRet;
	int nSize = MAX_AUDIO_FRAME_SIZE;
	int copySize = 0;
	char bArray[frame_cnt * 160];
	char *tmp = bArray;
	while(frame_cnt--)
	{
		iRet = tp_mp4_read_audioFrame((void*)params, &nSize);
		if(RetTrue == iRet && nSize != 0){
			memcpy(tmp, trparam->aBuffer, nSize);
			copySize += nSize;
			tmp += nSize;
		}
		else{
			//LOGE("Get record audio frame error .nszie = 0");
			continue;
		}
	}

	jshortArray sArray = (*env)->NewShortArray(env, copySize);
	short *pArray = (short*)malloc(copySize * sizeof(jshort));
	if(pArray ==NULL)
		return NULL;

	int index = 0;
	//case AUDIO_FORMAT_G711Mu:
	while (index < copySize){
		pArray[index] = Snack_Mulaw2Lin(bArray[index]);				//g711.h
		index++;
	}

	//LOGI("GetAudioFrame cpySize = %d",cpySize);
	(*env)->SetShortArrayRegion(env, sArray, 0, copySize, pArray);

	free(pArray);
	return sArray;
}

jint Java_com_topvs_platform_LibAPI_CloseRecordFile(JNIEnv* env, jclass jobj,
		jint params, jint viindex)
{
	release_decoder(viindex);

	if(NULL != g_frameBufferOut[viindex]){
		free(g_frameBufferOut[viindex]);
		g_frameBufferOut[viindex] = NULL;
	}

	return tp_mp4_close_mp4file((void*)params);
}
#endif

#define _mark_c2 0

//2014-6-4 modify LinZh107
jint Java_com_topvs_platform_LibAPI_DomeControl(JNIEnv* env, jclass jobject, jint iNodeIndex,
	jint state, jint iSpeed, jstring strcmd)
{
	GUINFO *pGuInfo = &pDeviceList->nodearray[iNodeIndex].guInfo;

	/**
	 * Removed by LinZh107 on 2015-8-06
	 * Optimize the control speed, remove the request
	 */
	//*int code;
	//*if(0 != DomeCtrlRequest(pLibInstParam, pGuInfo->DomainID, pGuInfo->GUID, &code) )
	//*	return -2;

	jboolean iscopy = JNI_TRUE;
	const char *cmd = (*env)->GetStringUTFChars(env, strcmd, &iscopy);
	if (0 == strcmp(cmd, ""))
	{
		LOGE("DomeCtrl cmd was wrong!");
		return -1;
	}

	PTZControl ptzCtl;
	memset(&ptzCtl, 0, sizeof(ptzCtl));
	sprintf(ptzCtl.msgtype, "ControlPTZ");
	ptzCtl.speed = iSpeed;
	if (state)
	{
		memset(ptzCtl.param, 0, sizeof(ptzCtl.param));
		strcpy(ptzCtl.cmd, cmd);
	}
	else //stop move
	{
		strcpy(ptzCtl.cmd, "STOP");
		strncpy(ptzCtl.param, cmd, strlen(cmd));
	}

	return DomeControl(pLibInstParam, *pGuInfo,	ptzCtl.msgtype,	ptzCtl.speed, ptzCtl.cmd, ptzCtl.param);
}

jint Java_com_topvs_platform_LibAPI_GetPuImageDisplayPara(JNIEnv* env, jclass jobject,
		jint iNodeIndex, jintArray pOutArray)
{
	//GUINFO *pGuInfo = g_GuInfoArray[iNodeIndex];
	GUINFO *pGuInfo = &pDeviceList->nodearray[iNodeIndex].guInfo;
	jint *intArray = (*env)->GetIntArrayElements(env, pOutArray, 0);

	int ret = GetPuImageDisplayPara(pLibInstParam, *pGuInfo, &intArray[0], &intArray[1],
			&intArray[2], &intArray[3]);

	(*env)->ReleaseIntArrayElements(env, pOutArray, intArray, 0);
	if (ret != 0)
		return -1;
	else
		return 0;
}

jint Java_com_topvs_platform_LibAPI_SetPuImageDisplayPara(JNIEnv* env, jclass jobject, jint iNodeIndex, jintArray pOutArray)
{
//	GUINFO *pGuInfo = g_GuInfoArray[iNodeIndex];
	GUINFO *pGuInfo = &pDeviceList->nodearray[iNodeIndex].guInfo;
	jint *intArray = (*env)->GetIntArrayElements(env, pOutArray, 0);

	int ret = SetPuImageDisplayPara(pLibInstParam, *pGuInfo, intArray[0], intArray[1], intArray[2], intArray[3]);

	(*env)->ReleaseIntArrayElements(env, pOutArray, intArray, 0);
	if (ret != 0)
		return -1;
	else
		return 0;
}

jint Java_com_topvs_platform_LibAPI_GetPuImageEncodePara(JNIEnv* env, jclass obj,
		jint iNodeIndex, jintArray pOutArray)
{
	jint *intArray = (*env)->GetIntArrayElements(env, pOutArray, 0);
//	GUINFO *pGuInfo = g_GuInfoArray[iNodeIndex];
	GUINFO *pGuInfo = &pDeviceList->nodearray[iNodeIndex].guInfo;
	IMAGEENCODEPARAM encode;
	int ret = GetPuImageEncodePara(pLibInstParam, *pGuInfo, &encode);
	if (ret == 0)
	{
		intArray[0] = encode.video_format;
		intArray[1] = encode.resolution;
		intArray[2] = encode.bitrate_type;
		intArray[3] = encode.level;
		intArray[4] = encode.frame_rate;
		intArray[5] = encode.Iframe_interval;
		intArray[6] = encode.prefer_frame;
		intArray[7] = encode.Qp;
		intArray[8] = encode.encode_type;
	}

	(*env)->ReleaseIntArrayElements(env, pOutArray, intArray, 0);

	if (ret != 0)
		return -1;
	else
		return 0;
}

jint Java_com_topvs_platform_LibAPI_SetPuImageEncodePara(JNIEnv* env, jclass obj,
		jint iNodeIndex, jintArray pOutArray)
{
	jint *intArray = (*env)->GetIntArrayElements(env, pOutArray, 0);
//	GUINFO *pGuInfo = g_GuInfoArray[iNodeIndex];
	GUINFO *pGuInfo = &pDeviceList->nodearray[iNodeIndex].guInfo;
	IMAGEENCODEPARAM encode;

	encode.video_format = intArray[0];
	encode.resolution = intArray[1];
	encode.bitrate_type = intArray[2];
	encode.level = intArray[3];
	encode.frame_rate = intArray[4];
	encode.Iframe_interval = intArray[5];
	encode.prefer_frame = intArray[6];
	encode.Qp = intArray[7];
	encode.encode_type = intArray[8];

	int ret = SetPuImageEncodePara(pLibInstParam, *pGuInfo, encode);
	(*env)->ReleaseIntArrayElements(env, pOutArray, intArray, 0);

	return ret;
}

jint Java_com_topvs_platform_LibAPI_RemoteAlarmEnable(JNIEnv* env, jclass jobj, jint iNodeIndex,
		jint bEnable)
{
	//GUINFO *pGuInfo = g_GuInfoArray[iNodeIndex];
	GUINFO *pGuInfo = &pDeviceList->nodearray[iNodeIndex].guInfo;
	int ret;
	if (bEnable > 0)
		ret = RemoteAlarmEnable(pLibInstParam, *pGuInfo, 1);
	else
		ret = RemoteAlarmEnable(pLibInstParam, *pGuInfo, 0);
	return ret;
}

#define _mark_c3 0

// Add Date: 2014-6-5  Author: LinZh107
jint Java_com_topvs_platform_LibAPI_GetdevAlarmInfo(JNIEnv* env, jclass jcls, jobject jobj_out)
{
	//LOGI("begin copy native to java!");
	ck_alarminfo netAlarminfo = { 0 };
	if (0 != GetAlarmInfo(pLibInstParam, &netAlarminfo, g_AlarmDevID))
		return -1;

	g_AlarmDevID++;

	jclass AlarmInfoCls = (*env)->GetObjectClass(env, jobj_out);

	jfieldID out_devid = (*env)->GetFieldID(env, AlarmInfoCls, "devid", "I");
	jfieldID out_alarmtime = (*env)->GetFieldID(env, AlarmInfoCls, "alarmtime", "I");
	jfieldID out_airtemp = (*env)->GetFieldID(env, AlarmInfoCls, "airtemp", "F");
	jfieldID out_airhumi = (*env)->GetFieldID(env, AlarmInfoCls, "airhumi", "F");
	jfieldID out_soiltemp = (*env)->GetFieldID(env, AlarmInfoCls, "soiltemp", "F");
	jfieldID out_soilhumi = (*env)->GetFieldID(env, AlarmInfoCls, "soilhumi", "F");
	jfieldID out_CO2density = (*env)->GetFieldID(env, AlarmInfoCls, "CO2density", "F");
	jfieldID out_illuminance = (*env)->GetFieldID(env, AlarmInfoCls, "illuminance", "F");

	jfieldID out_water_sat = (*env)->GetFieldID(env, AlarmInfoCls, "water_sat","F");	//soil water saturation
	jfieldID out_daily_rain = (*env)->GetFieldID(env, AlarmInfoCls, "daily_rain","F");	//daily rainfall
	jfieldID out_anion = (*env)->GetFieldID(env, AlarmInfoCls, "anion","F");
	jfieldID out_pm25 = (*env)->GetFieldID(env, AlarmInfoCls, "pm25","F");
	jfieldID out_wind_speed = (*env)->GetFieldID(env, AlarmInfoCls, "wind_speed","F");

	(*env)->SetIntField(env, jobj_out, out_devid, netAlarminfo.devid);
	(*env)->SetIntField(env, jobj_out, out_alarmtime, netAlarminfo.alarmtime);
	(*env)->SetFloatField(env, jobj_out, out_airtemp, netAlarminfo.airtemp);
	(*env)->SetFloatField(env, jobj_out, out_airhumi, netAlarminfo.airhumi);
	(*env)->SetFloatField(env, jobj_out, out_soiltemp, netAlarminfo.soiltemp);
	(*env)->SetFloatField(env, jobj_out, out_soilhumi, netAlarminfo.soilhumi);
	(*env)->SetFloatField(env, jobj_out, out_CO2density, netAlarminfo.CO2density);
	(*env)->SetFloatField(env, jobj_out, out_illuminance, netAlarminfo.illuminance);
	(*env)->SetFloatField(env, jobj_out, out_water_sat, netAlarminfo.water_sat);
	(*env)->SetFloatField(env, jobj_out, out_daily_rain, netAlarminfo.daily_rain);
	(*env)->SetFloatField(env, jobj_out, out_anion, netAlarminfo.anion);
	(*env)->SetFloatField(env, jobj_out, out_pm25, netAlarminfo.pm25);
	(*env)->SetFloatField(env, jobj_out, out_wind_speed, netAlarminfo.wind_speed);

	return 0;
}

jint Java_com_topvs_platform_LibAPI_StartChart(JNIEnv* env, jclass jcls, jint iNodeIndex, jint viindex)
{
    LOGI("in %s\n",__FUNCTION__);
    if (IsChartOpen(pLibInstParam)) {
        StopChart(pLibInstParam);
    }

    VideoInstance_t *pVInst = (VideoInstance_t *)g_videoInstArray[viindex];
    if(!pVInst){
        LOGE("video didn't start before this action");
        return -1;
    }

    CLIENTINFO client;
    client.lpFunc = data_Callback;//lpFunc;
    client.lHandle = NULL;
    client.nProtocolType = 1;//TCP
	//memcpy(&client.guInfo, g_GuInfoArray[iNodeIndex], sizeof(GUINFO));
    memcpy(&client.guInfo, &pDeviceList->nodearray[iNodeIndex].guInfo, sizeof(GUINFO));
    client.playinst = pVInst->playinst;

    // start the chart thread nProtocolType 0-UDP, 1-TCP
    int ret = ChartWithOne(pLibInstParam, &client);
    return ret;
}

//int API_sendAudioData(void *user_id, short* sArray, int dataLen)
jint Java_com_topvs_platform_LibAPI_SendVoiceData(JNIEnv* env, jclass jcls, jshortArray AudioArray, jint dataLen)
{
	//LOGI("in %s\n",__FUNCTION__);
	jshort* sArray = (*env)->GetShortArrayElements(env, AudioArray, 0);
	char *pArray = malloc(dataLen);
	memset(pArray, 0, dataLen);
	int index = 0;
	int ret = 0;
	while(index < dataLen)
	{
		pArray[index] = Snack_Lin2Alaw(sArray[index]);			//g711.h
		//pArray[index] = Snack_Lin2Mulaw(sArray[index]);			//g711.h
		index++;
	}

	/*
	 * Create HASI audio frame
	 * 		begin with two short integer, 256 and 40
	 * 		after that is the pure audio data
	 * 		short Hi_head[2];
	 *		Hi_head[0] = 256;
	 *		Hi_head[1] = 40;
	 * So
	 * 		data_len = 4 + 80 (HASI length)
	 * and than	send length will be:
	 * 		RealLength = (data_len + 4) * num + 4;
	 * 		while num is the frame count
	 * */
	static int frameno = 0;
	int num = dataLen / 80;
	int data_len = 84;//(encBuffer[1]&0x00ff)*2+4;

	//begin with four byte
	short Hi_head[2];
	Hi_head[0] = 256;
	Hi_head[1] = 40;

	int RealLength = (data_len + 4) * num + 4;

	//while AUDIO_FORMAT_G711A
	int audio_talk_format = 1;
	int irate = 8000;
	int audio_talk_width = 16;
	int audio_amr_mode = 7;

	char *sendbuf = malloc(sizeof(frame_head_t) + RealLength);

	frame_head_t pframe_head;
	pframe_head.device_type = 0;
	pframe_head.frame_size = RealLength;
	pframe_head.frame_no = frameno;
	frameno++;
	pframe_head.video_resolution = (audio_talk_format & 0x03) | (audio_amr_mode << 2);
	pframe_head.frame_type = 3;
	pframe_head.frame_rate = (audio_talk_width & 0x03) | (irate << 2);
	pframe_head.video_standard= audio_talk_format;

	pframe_head.sec = 0;
	pframe_head.usec = 0;

	memcpy(sendbuf, (char *)&pframe_head, sizeof(frame_head_t));
	memcpy(&sendbuf[sizeof(frame_head_t)], (char*)&num, 4);
	int bufoffset = sizeof(frame_head_t) + 4;
	int cnt=0;
	while(cnt<num)
	{
		memcpy(&sendbuf[bufoffset], (char*)&data_len, 4);
		bufoffset += 4;
		//because	84 = data_len = (encBuffer[1]&0x00ff)*2+4;
		//so there are 4 byte was retain
		memcpy(&sendbuf[bufoffset], (char*)&Hi_head, 4);
		bufoffset += 4;
		memcpy(&sendbuf[bufoffset], (char*)&pArray[cnt * 80], 80);
		bufoffset += 80;
		cnt++;
	}
	ret = SendVoiceData(pLibInstParam, sendbuf, sizeof(frame_head_t) + RealLength);
//	static int cntpri = 0;
//	if(cntpri % 100 == 0)
//		LOGI("send data to pu Length = %d",ret);
//	cntpri++;

	(*env)->ReleaseShortArrayElements(env, AudioArray, sArray, 0);
	free(pArray);
	free(sendbuf);
	return ret;
}

jint Java_com_topvs_platform_LibAPI_StopChart(JNIEnv* env, jclass jcls){
    LOGI("in %s",__FUNCTION__);
    return StopChart(pLibInstParam);
}

jint Java_com_topvs_platform_LibAPI_IoControl(JNIEnv* env, jclass jcls,
		jint iNodeIndex, jint bStatus, jint bReturn)
{
	//GUINFO *pGuInfo = g_GuInfoArray[iNodeIndex];
	GUINFO *pGuInfo = &pDeviceList->nodearray[iNodeIndex].guInfo;
	return IoControl(pLibInstParam, *pGuInfo, bStatus, bReturn);
}

//add by LinZh107 for ffmpegLib to decode HD-frame without delay
jint Java_com_topvs_platform_LibAPI_setDecoderParam(JNIEnv* env, jclass jcls, jint viindex,
		jint videoMaxWidth, jint decodRate)
{
	return set_decoder_param(viindex, videoMaxWidth, decodRate);
}
