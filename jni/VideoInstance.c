/*
 * VideoInstance.c
 *
 *  Created on: 2009-10-16
 *      Author: pc
 */

#include "SipAPI.h"
#include "VideoInstance.h"
#include "netglobal.h"
#include "st_cuapi.h"
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <android/log.h>
#define null 0
#define  LOG_TAG    "videoInstance.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define FRAME_MAX_SIZE_T       512 * 1024 //接收缓冲buffer的最大值  //同步 playList。c 的大小

void st_media_countLost(void *data, unsigned int *lastseq, unsigned int *cnt, unsigned int *rate, unsigned int *lost,
		unsigned int *lostrate);

static void * Thread_MDUHeart(void* lParam);      // 实例的MDU心跳线程
static void * Thread_MduRealVideo(void *lParam);  // 视频接收线程

inline void videoi_SetLinkState(VideoInstance_t *pIns, int bLinked)
{
    //m_csLinked.Lock();
	pIns->m_bLinked = bLinked;
	//m_csLinked.Unlock();
}

void videoi_SetCameraInfo(VideoInstance_t *pIns, const GUINFO guInfo)
{
	pIns->m_guInfo = guInfo;
}

void* videoi_CreateInstance(const CLIENTINFO* pClient)
{
	VideoInstance_t *pIns = (VideoInstance_t *) malloc(sizeof(VideoInstance_t));
	if (pIns == NULL){
		LOGE("CreateVideoInstance fail!");
		return NULL;
	}

	memset(pIns, 0, sizeof(*pIns));
    memset(&pIns->thrdPara, 0, sizeof(THREAD_VIDEOINSTANCE_PARA));
    
	pIns->m_lpFunc = NULL;
	pIns->m_lHandle = NULL;
	pIns->m_pThread_MduHeart = 0;
	pIns->m_pThread_RecvData = 0;
	pIns->m_hTcpSocket.m_hSocket = -1;
	pIns->m_hTcpDataSocket.m_hSocket = -1;
    
	pIns->m_lHandle = pClient->lHandle;
	pIns->m_lpFunc = (RECVCALLBACK)pClient->lpFunc;
	pIns->m_nProtocolType = pClient->nProtocolType;
	pIns->playinst = pClient->playinst;

	videoi_SetCameraInfo(pIns, pClient->guInfo);
	videoi_SetLinkState(pIns, 0);
	
	pthread_mutex_init(&pIns->dataList_mutex, NULL);

	return pIns;
}

inline void videoi_DeleteInstance(VideoInstance_t *pIns)
{
	videoi_QuitInstance(pIns);
	pthread_mutex_destroy(&pIns->dataList_mutex);

	free(pIns);
	pIns = NULL;
}

int videoi_InitInstance(VideoInstance_t *pIns)
{
	//int ret;
	if(pIns->m_hTcpSocket.m_hSocket != -1)
    {
		close(pIns->m_hTcpSocket.m_hSocket);
		pIns->m_hTcpSocket.m_hSocket = -1;
    }
	if(pIns->m_hTcpDataSocket.m_hSocket != -1)
    {
		close(pIns->m_hTcpDataSocket.m_hSocket);
		pIns->m_hTcpDataSocket.m_hSocket = -1;
    }
		
	if( init_p_socket_tcp(&pIns->m_hTcpSocket) < 0)
		return -1;

	if (pIns->m_nProtocolType == 1)
	{
		if( init_p_socket_tcp(&pIns->m_hTcpDataSocket) < 0)
			return -1;
	}
	else
		return -1;
	
	return 0;
}

inline int  videoi_GetLinkState(VideoInstance_t *pIns)
{    
	return pIns->m_bLinked;
}

void  videoi_SetMediaDataInfo(VideoInstance_t *pIns, const MEDIA_DATA_INFO* pInfo)
{
	pIns->m_MediaDataInfo = *pInfo;
}

void  videoi_GetMediaDataInfo(VideoInstance_t *pIns, MEDIA_DATA_INFO* pInfo)
{
	*pInfo = pIns->m_MediaDataInfo;
}

void videoi_GetCameraInfo(VideoInstance_t *pIns, GUINFO* guInfo)
{
	*guInfo = pIns->m_guInfo;
}


// 线程函数为静态的成员变量,需要保证可重入
static void * Thread_MDUHeart(void* lParam)
{
    LOGI("Create----->Thread_MDUHeart Thread!");
	pthread_detach(pthread_self());

	THREAD_VIDEOINSTANCE_PARA* pPara = (THREAD_VIDEOINSTANCE_PARA*) lParam;
	VideoInstance_t *pIns = pPara->pClient;
	GUINFO guInfo;
	char buf[1024];
	int nSeqNum = 0;
	int nSend = 0;
	int nRecv = 0;
	long long dwLastRecv;
	int i;
	struct timeval tv;
	
	videoi_GetCameraInfo(pIns, &guInfo);
	nSeqNum = 0;
	gettimeofday(&tv, NULL);
	dwLastRecv = (long long)tv.tv_sec*1000+(long long)tv.tv_usec/1000;

	pIns->m_MduHeartThreadflag = 2;
	while(1)
	{
		for(i = 0; i < 50; i++)
		{
			if(pIns->m_MduHeartThreadflag == 10)
            {
				goto retout;
            }
			usleep(100*1000);
		}
		gettimeofday(&tv, NULL);
        
        // 检查连接标志,如果未连接(中断),则等待连接标志为TRUE;
		if(0 ==  pIns->m_bLinked)
		{
			dwLastRecv = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;  // 复位计时

            //add LinZh107  2014-09-05
            MDU_ROUTE_INFO mduInfo;
            memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
            if(voidei_Reconnect(pIns, pPara->pServerPara) == 0)
            {
            	LOGI("++++++++++++++++++++++++++++");
               	LOGI("Thread_MDURealVideo Reconnect Succees!");
            }
            //add reconnect function, but it seem not so effective  still need to improve
            continue;
		}
		
		if( ((long long)tv.tv_sec*1000+(long long)tv.tv_usec/1000) - dwLastRecv > MDU_KEEP_CIRCUIT)
		{
			// 连接已经断开
			dwLastRecv = tv.tv_sec * 1000 + tv.tv_usec / 1000;  // 复位计时
			pIns->m_bLinked = 0;
			//pIns->DoNotifyMsg(pPara->pdwServerPara, 0);
			continue;
		}

		if( ((long long)tv.tv_sec*1000+(long long)tv.tv_usec/1000) - dwLastRecv >= MDU_KEEP_CIRCUIT/3)
		{
			// 发送心跳数据包
			nSeqNum++;  // 包序号递增
			nSend = CreatePack_MDU_Heart(buf, nSeqNum, pIns->m_callId, &guInfo);
			pIns->m_hTcpSocket.m_senddata(&pIns->m_hTcpSocket, buf, nSend);

			// 等待服务器响应
			memset(buf, 0, sizeof(buf));
			nRecv = pIns->m_hTcpSocket.recv_sippackage(&pIns->m_hTcpSocket, buf, sizeof(buf) - 1, 500);
			if (nRecv > 0)
			{
				gettimeofday(&tv, NULL);
				dwLastRecv = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000; // 记录接收时间
				buf[nRecv] = '\0';
			}
		}
		//LOGI("Send a MDUHeart succeeding!");
	}//end while(1)

retout:
	LOGI("Exit---->Thread_MDUHeart!");
	pIns->m_MduHeartThreadflag = 1;
	return NULL;
}

static void *Thread_MduRealVideo(void* lParam)
{
    LOGI("Create----->Thread_MduRealVideo Thread!");
	pthread_detach(pthread_self());

	THREAD_VIDEOINSTANCE_PARA* pPara = (THREAD_VIDEOINSTANCE_PARA*) lParam;
	VideoInstance_t *pIns = pPara->pClient;
	char* full_frame_data = malloc(FRAME_MAX_SIZE_T);	// 接收数据缓冲区
	if ((pIns == NULL) || (full_frame_data == NULL)){
		return NULL;
	}
	memset(full_frame_data, 0, FRAME_MAX_SIZE_T); // sizeof(full_frame_data)//modified by zhusy

	//GUINFO guInfo;
	//videoi_GetCameraInfo(pIns, &guInfo);

	MEDIA_DATA_INFO  mediaInfo = {0,0,0,0,0,0};
	unsigned int     nLastSeq = 0;
	unsigned int     nLastTs  = 0;

	char net_Pack[MTU_LEN] = { 0 };	//LinZh107

	int nBufSize = 0;
	unsigned int nRtpHeadLen = sizeof(RTP_FIXED_HEADER) + sizeof(RTP_HEADER_EXT);
	const unsigned int nFrameHeadLen = sizeof(frame_head_t);
	const FRAMHEAD_T* const pFrameHead = (FRAMHEAD_T*) (char*) full_frame_data;
	const RTP_FIXED_HEADER* const pRtpHead = (RTP_FIXED_HEADER*) net_Pack;
	const RTP_HEADER_EXT* const pRtpExt = (RTP_HEADER_EXT*) (net_Pack + sizeof(RTP_FIXED_HEADER));
	int recv_frame_size = 0;

	long long dwLastRecv;
	long long dwLastSend;
	long long dwLast;
	int nRecv = 0;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	dwLast = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;
	dwLastRecv = dwLast;
	dwLastSend = dwLast;

	pIns->m_RecvDataThreadflag = 2;
	while (1)
	{
		// 这里可以改善一下: 用标志位来确定是否退出; 在断开后, 再挂起
		// 1. 是否要求退出
		if (pIns->m_RecvDataThreadflag == 10)
		{
			//LOGI("Request to cancle Thread_MduRealVideo!");
			break;
		}

		gettimeofday(&tv, NULL);

		// 2. 检查连接标志,如果未连接(中断),则等待连接标志为TRUE;
		if (0 == pIns->m_bLinked)
		{
			mediaInfo.packages_rate = 0;
			mediaInfo.lostrate =0;
			mediaInfo.rate = 0;
			videoi_SetMediaDataInfo(pIns, &mediaInfo);
			dwLastRecv = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000; // 如果自动重连,需要用当前时间来复位这个参数
			// 需要释放CPU时间
			usleep(5000);
			continue;
		}

		// 2. 接收TCP 、UDP数据,有超时设置
		if (pIns->m_nProtocolType == 1)
		{
			nRecv = pIns->m_hTcpDataSocket.m_receive(&pIns->m_hTcpDataSocket, net_Pack, nRtpHeadLen, 500);
			if (nRecv <= 0 || nRecv != (int) nRtpHeadLen){
				//LOGE("Thread_MduRealVideo read head error");
				goto RECV_ERR;
			}
			nRecv = pIns->m_hTcpDataSocket.m_receive(&pIns->m_hTcpDataSocket, net_Pack + nRtpHeadLen, pRtpExt->len, 500);
			if (nRecv <= 0 || nRecv != (int) pRtpExt->len){
                //LOGE("Thread_MduRealVideo read data error");
				goto RECV_ERR;
			}
			if (pRtpHead->x > 0)
				nRtpHeadLen = sizeof(RTP_FIXED_HEADER) + sizeof(RTP_HEADER_EXT) * pRtpHead->cc;
			else
				nRtpHeadLen = sizeof(RTP_FIXED_HEADER);

			nRecv += nRtpHeadLen;
		}

		// 3.不能正常接收的处理
	RECV_ERR:
		if (nRecv <= 0)
		{
			int curtime = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;
			if (curtime - dwLastRecv > 10000) // 超时判断
			{
				pIns->m_bLinked = 0;
				memset(full_frame_data, 0, FRAME_MAX_SIZE_T); //modified by zhusy
				nBufSize = 0;
				// 网络事件通知
				//pIns->DoNotifyMsg(pPara->pdwServerPara, 0);

			}
			continue;
		}

		// 4. 正常接收后的处理
		dwLastRecv = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000; // 记录接收时间

		// 5. 接收长度若不够一个RTP头长度,则丢弃重来
		if (nRecv < (int) nRtpHeadLen)
		{
			LOGE("Error__media Length: nRecv=%d(<nRtpHeadLen), contune...");
			continue;
		}
		// 6. 如果包长度大于RTP头长度,则拷贝数据
		else
		{
			// 若序号正确 ts和上一个一致, 则是同一帧, 继续接收数据
			//LOGI("pRtpHead->seq:%d & pRtpHead->ts:%d", pRtpHead->seq, pRtpHead->ts);
			if (((nLastSeq + 1) == pRtpHead->seq) && (nLastTs == pRtpHead->ts))
			{
				nLastSeq = pRtpHead->seq;
				memcpy(full_frame_data + nBufSize, net_Pack + nRtpHeadLen, nRecv - nRtpHeadLen);
				nBufSize += nRecv - nRtpHeadLen;
				// 判断BUF是否越界
				if (nBufSize > FRAME_MAX_SIZE_T - MTU_LEN)	//modified by zhusy
					nBufSize = 0;
				continue;
			}
			else  //满一帧, 执行之前缓冲的数据检查, 检查通过的数据送入解码缓冲区
			{
				if (nBufSize >= nFrameHeadLen)  // full_frame_data中的长度大于一个头长度
				{
					recv_frame_size = pFrameHead->frame_size + nFrameHeadLen;
					if (nBufSize == recv_frame_size)
					{
						//LOGI("pFrameHead->frame_type:%d", pFrameHead->frame_type);
						//只播放解析视频帧
						if (pFrameHead->frame_type == SANTACHI_I_FRAME_TYPE
								|| pFrameHead->frame_type == SANTACHI_P_FRAME_TYPE
								|| pFrameHead->frame_type == SANTACHI_A_FRAME_TYPE)
						{
							//调回调，送解码
							pIns->m_lpFunc(pIns->m_lHandle, full_frame_data, recv_frame_size, pIns->playinst);
							if (pIns->m_bRecord > 0 && pIns->rcMp4Func != NULL)
							{
								pIns->rcMp4Func(pIns->record_file, pIns->track_video, pIns->track_audio,
										full_frame_data, recv_frame_size, pFrameHead->frame_type);
							}
						}
						nBufSize = 0;
					}
				}
				nLastSeq = pRtpHead->seq;    // 设置seq为当前包的seq
				nLastTs = pRtpHead->ts;      // 设置nLastTs为当前包的ts
				// 原缓冲区数据应丢弃
				nBufSize = 0;                // 设置缓冲区有效长度为0
				memcpy(full_frame_data + nBufSize, net_Pack + nRtpHeadLen, nRecv - nRtpHeadLen);
				nBufSize = nRecv - nRtpHeadLen;
			}
		}
	}

	if (NULL != full_frame_data)
	{
		free(full_frame_data);
		full_frame_data = NULL;
	}

	pIns->m_RecvDataThreadflag = 1;
	LOGI("Exit---->Thread_MduRealVideo");
	return NULL;
}

// 启动工作者线程,线程对象不自动退出,需要手动delete
void videoi_StartThreads(VideoInstance_t *pIns, int pServerPara)
{
	int ret;

	pIns->thrdPara.pServerPara = pServerPara;
	pIns->thrdPara.pClient = pIns;

	ret = pthread_create(&pIns->m_pThread_MduHeart, NULL, Thread_MDUHeart, &pIns->thrdPara);
	if(ret < 0 )
	{
		pIns->m_MduHeartThreadflag = 0;
		LOGE("Thread_MDUHeart error!");
		return;
	}

	ret = pthread_create(&pIns->m_pThread_RecvData, NULL, Thread_MduRealVideo, &pIns->thrdPara);
	if(ret < 0 )
	{
		pIns->m_RecvDataThreadflag = 0;
		LOGE("Thread_MduRealVideo error !");
		return;
	}
	//LOGI("videoi_StartThreads success!");
	return;
}

// 结束会话, 发送BYE指令
void  videoi_BreakSession(VideoInstance_t *pIns)
{
	char buf[1024];
	int nBufLen;

	if(pIns->m_hTcpSocket.m_hSocket == -1)
		return;

	memset(buf, 0, sizeof(buf));
	nBufLen = CreatePack_MDU_Bye(buf, pIns->m_nInviteSeq,  pIns->m_callId, &pIns->m_guInfo);
	//LOGI("videoi_BreakSession send &pIns->m_hTcpSocket:\n");
	pIns->m_hTcpSocket.m_senddata(&pIns->m_hTcpSocket, buf, nBufLen);
}

void videoi_CancelInvite(VideoInstance_t *pIns)
{
	if(pIns->m_hTcpSocket.m_hSocket <= 0){
        //printf("\nvideoi_cancleInvite\n");
        return;
    }
	char buf[1024] = {0};
	int nBufLen = CreatePack_MDU_Bye(buf, pIns->m_nInviteSeq,  pIns->m_callId, &pIns->m_guInfo);
	pIns->m_hTcpSocket.m_senddata(&pIns->m_hTcpSocket, buf, nBufLen);
}

// 退出实例的线程对象并删除线程对象,主要是为了正常退出线程
void videoi_QuitInstance(VideoInstance_t *pIns)
{
	int cnt = 300;
	if( pIns->m_MduHeartThreadflag >= 2)
	{
		pIns->m_MduHeartThreadflag = 10;
		while(cnt > 0)
		{
			if(pIns->m_MduHeartThreadflag == 1)
			{
				break;
			}
			usleep(5000); //modify 900 to 10
			cnt--;
		}
		pIns->m_MduHeartThreadflag = 0;
	}

	if(pIns->m_RecvDataThreadflag >= 2)
	{
		pIns->m_RecvDataThreadflag = 10;
		cnt = 300;
		while (cnt > 0)
		{
			if (pIns->m_RecvDataThreadflag == 1 ){
				break;
			}
			usleep(5000); //modify 900 to 10
			cnt--;
		}
		pIns->m_RecvDataThreadflag = 0;
	}
	usleep(100);
	if(pIns->m_hTcpSocket.m_hSocket != -1)
    {
		close(pIns->m_hTcpSocket.m_hSocket);
		pIns->m_hTcpSocket.m_hSocket = -1;
    }
	if(pIns->m_hTcpDataSocket.m_hSocket != -1)
    {
		close(pIns->m_hTcpDataSocket.m_hSocket);
		pIns->m_hTcpDataSocket.m_hSocket = -1;
    }
}

void st_media_countLost(void *data, unsigned int *lastseq, unsigned int *cnt, unsigned int *rate, unsigned int *lost,
		unsigned int *lostrate)
{
	RTP_FIXED_HEADER *pRtp = (RTP_FIXED_HEADER*) data;

	if (*lastseq == 0)
		*lastseq = pRtp->seq;
	else
	{
		if (pRtp->seq != (*lastseq + 1))
		{
			if (pRtp->seq > *lastseq)
			{
				*lostrate += (pRtp->seq - *lastseq - 1);
				*lost += *lostrate;
			}
			//TRACE("pack lost......%d...%d.\n",pRtp->seq,*lastseq);// DEBUG
		}
	}
	*lastseq = pRtp->seq;

	(*cnt)++;
	(*rate)++;
}

int videoi_ConnectToServer(VideoInstance_t * handle, int pdwServerPara, const char* lpszMduIp, unsigned int uMduPort) //LPCTSTR
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pdwServerPara;

	if (handle->m_hTcpSocket.set_tcp_link(&handle->m_hTcpSocket, lpszMduIp, uMduPort) < 0)
	{
		//int iCount = -1;
		//LOGE("m_hTcpSocket.set_tcp_link:%d", iCount);
		return -1;
	}
	if (handle->m_hTcpSocket.m_connect(&handle->m_hTcpSocket, pPara->CmdTimeOut) >= 0)
	{
		return 0;
	}
	else
	{
		//int iCount = -1;
		//LOGE("m_hTcpSocket.m_connect:%d", iCount);
		return -1;
	}
}

// 请求视频  	 前提: 已经与服务器建立连接
int videoi_InviteMedia(VideoInstance_t * handle, int pdwServerPara, MDU_ROUTE_INFO mduInfo, GUINFO* pGuInfo)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pdwServerPara;
	GUINFO guInfo;
	char buf[1024] = { 0 };
	int nBufLen = 0;
	int nRecv;
	struct timeval tv;
	sip_cmd_t SipCmd;
	char szXmlBuf[1024] = { 0 };
	int nUdpPort = 0;
	char szMediaPort[128];
	char *pStr;
	char szMduIp[256] = { 0 };
	unsigned int nPortTemp;

	if (pGuInfo == NULL)
		guInfo = handle->m_guInfo;
	else
		guInfo = *pGuInfo;

	handle->m_nInviteSeq = CreateSeqByTime();
	CreateCallID(handle->m_callId);

	nBufLen = CreatePack_MDU_RealStartReq(buf, handle->m_nInviteSeq, handle->m_callId,
			&guInfo, handle->m_nProtocolType, mduInfo);

	if (handle->m_hTcpSocket.m_senddata(&handle->m_hTcpSocket, buf, nBufLen) < 0)
	{
		//LOGE("in videoi_InviteMedia ST_ERR_SENDDATA - 1");
		handle->m_ErrCode = ST_ERR_SENDDATA;
		return -1;
	}

	// 2-超时recv调用,如果未收到信息,则返回失败,收到则转到第9步
	gettimeofday(&tv, NULL);
	memset(buf, 0, sizeof(buf));
	//LOGI("****** videoi_InviteMedia m_senddata buf2 = %.*s ******", 7, buf);
	nRecv = handle->m_hTcpSocket.recv_sippackage(&handle->m_hTcpSocket, buf, sizeof(buf), pPara->CmdTimeOut);
	if (nRecv <= 0)
	{
		//LOGE("videoi_InviteMedia recv_sippackage  fail");
		handle->m_ErrCode = 0x80008022;
		return -2;
	}
	*(buf + nRecv) = '\0';
	//LOGI("****** videoi_InviteMedia recv_sippackage buf = %.*s, %d ******", 7, buf, strlen(buf));

	// 3-分析信息,如果请求未通过,则返回失败信息,如果通过则返回
	memset(&SipCmd, 0, sizeof(SipCmd));

	st_sip_parseSip(buf, strlen(buf), &SipCmd, szXmlBuf);

	if (SipCmd.answerNum != 200)
	{
		switch (SipCmd.answerNum)
		{
		case 404:
			handle->m_ErrCode = 0x80008041;
			break;
		case 486:
			handle->m_ErrCode = 0x80008044;
			break;
		default:
			handle->m_ErrCode = 0x80008042;
			break;
		}
		//LOGE("in videoi_InviteMedia ST_ERR_SENDDATA - 3");
		return -3;
	}
	pStr = GetMiddleString(buf, "mediaPort>", "/mediaPort", szMediaPort, sizeof(szMediaPort));

	if (pStr == NULL){
		handle->m_ErrCode = 0x80008043;
		//LOGE("in videoi_InviteMedia ST_ERR_SENDDATA - 4");
		return -4;
	}

	nUdpPort = atoi(szMediaPort);
	if (nUdpPort <= 0){
		handle->m_ErrCode = 0x80008043;
		return -1;
	}

	//==========得到正确的SIP回应后,回应一个ACK SIP包===========//
	memset(buf, 0, sizeof(buf));
	nBufLen = CreatePack_MDU_RealStartACK(buf, handle->m_nInviteSeq, handle->m_callId, &guInfo);

	if (handle->m_hTcpSocket.m_senddata(&handle->m_hTcpSocket, buf, nBufLen) < 0){
		handle->m_ErrCode = ST_ERR_SENDDATA;
		//LOGE("in videoi_InviteMedia ST_ERR_SENDDATA - 5");
		return -5;
	}

	// 10-如果请求通过, 设置UDP连接参数, 发送探测数据包, 然后开启相应的线程 ;如果TCP连接，直接发起connect
	handle->m_hTcpSocket.get_tcp_link(&handle->m_hTcpSocket, szMduIp, (int*) &nPortTemp);

#if 0
	//udp protocol
	if (handle->m_nProtocolType == 0)
	{
		usleep(10000);
		m_hUdpSocket.SetUdpLink(szMduIp, nUdpPort);
		char bufTest[128];
		sprintf(bufTest, "Udp test callID=%s", this->m_callId);
		int nSent = m_hUdpSocket.SendDataUdp(bufTest, strlen(bufTest));
		if (nSent != (int)strlen(bufTest))
		{
			m_ErrCode = ST_ERR_SENDDATA;
			//TRACE("Send Test pack, nSent=%d, ErrorCode=%d\n", nSent, WSAGetLastError());
			return FALSE;
		}
	}
	//tcp protocol
	else
#endif

	if (handle->m_nProtocolType == 1)
	{
		if (handle->m_hTcpDataSocket.set_tcp_link(&handle->m_hTcpDataSocket, szMduIp, nUdpPort) < 0)
			return -1;
		handle->m_hTcpDataSocket.m_connect(&handle->m_hTcpDataSocket, 1000);
	}

	else{
		LOGE("in videoi_InviteMedia ST_ERR_SENDDATA - 6");
		return -6;
	}

	// 设置连接成功标志并返回
	LOGI("exit videoi_InviteMedia succeed!");
	handle->m_bLinked = 1;
	return 0;
}

int voidei_Reconnect(VideoInstance_t * handle, int pdwServerPara)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pdwServerPara;
	int bRet;
	videoi_BreakSession(handle);
	if (handle->m_hTcpSocket.m_hSocket != -1){
		close(handle->m_hTcpSocket.m_hSocket);
		handle->m_hTcpSocket.m_hSocket = -1;
	}
	if (handle->m_hTcpDataSocket.m_hSocket != -1){
		close(handle->m_hTcpDataSocket.m_hSocket);
		handle->m_hTcpDataSocket.m_hSocket = -1;
	}

	if (videoi_InitInstance(handle))
	{
		bRet = -1;
		handle->m_ErrCode = 0;
	}
	else
	{
		//	char szSerIp[128]={0};
		MDU_ROUTE_INFO mduInfo;
		memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
		if (QueryPuRealRoute(pdwServerPara, handle->m_guInfo, &mduInfo) < 0){
			bRet = -1;
			handle->m_ErrCode = 0x80008034;
		}
		if (handle->m_hTcpSocket.set_tcp_link(&handle->m_hTcpSocket, mduInfo.szLocalMduIp, mduInfo.uLocalPort) < 0){
			bRet = -1;
			handle->m_ErrCode = 0x80008012;
		}
		else
		{
			if (handle->m_hTcpSocket.m_connect(&handle->m_hTcpSocket, pPara->CmdTimeOut) < 0){
				bRet = -1;
				handle->m_ErrCode = 0x80008012;
			}
			else
				bRet = videoi_InviteMedia(handle, pdwServerPara, mduInfo, NULL);
		}
	}
#if 0
	if(!bRet)
	DoNotifyMsg(pdwServerPara, 2);
	else
	DoNotifyMsg(pdwServerPara, 1);
#endif
	return bRet;
}

#if 0
void VideoInstance::DoNotifyMsg(DWORD pdwServerPara, int nStatus)
{
	P_LIB_INSTANCE_PARA pPara = NULL;
	pPara = (P_LIB_INSTANCE_PARA)pdwServerPara;

	AlarmInfo *pAlarm = new AlarmInfo;
	if(pAlarm != NULL)
	{
		ZeroMemory(pAlarm, sizeof(AlarmInfo));
		char szContent[TEXT_MAXLEN] =
		{	0};
		sprintf(pAlarm->szPuid, m_guInfo.PUID);
		sprintf(pAlarm->szGuid, m_guInfo.GUID);
		pAlarm->dwAlarmTime = GetTickCount(); //time(NULL);//modified by zhusy
		sprintf(pAlarm->szDeviceName , m_guInfo.GUName);
		if( 0 == nStatus)
		{
			pAlarm->nAlarmType = NET_ALARM_VIDEO_INTERMIT;
			::LoadStringFromID(AfxGetInstanceHandle(), ID_ALARMINFO_104028, szContent, sizeof(szContent)-1);
			sprintf(pAlarm->szAlarmInfo , szContent);
		}
		else if( 1 == nStatus)
		{
			pAlarm->nAlarmType = NET_ALARM_VIDEO_RECONNECT_SUCCEED;
			::LoadStringFromID(AfxGetInstanceHandle(), ID_ALARMINFO_104029, szContent, sizeof(szContent)-1);
			sprintf(pAlarm->szAlarmInfo , szContent);
		}
		else if( 2 == nStatus)
		{
			pAlarm->nAlarmType = NET_ALARM_VIDEO_RECONNECT_FAILED;
			::LoadStringFromID(AfxGetInstanceHandle(), ID_ALARMINFO_104030, szContent, sizeof(szContent)-1);
			pAlarm->dwParam1 = this->m_ErrCode;
			sprintf(pAlarm->szAlarmInfo , szContent);
		}
		else
		{
			delete pAlarm;
			return;
		}

		//pPara->csAlarmInfoList.Lock();
		//pPara->AlarmInfoList.AddTail(pAlarm);
		//	pPara->csAlarmInfoList.Unlock();
		//::SendNotifyMessage(pPara->hMsgWnd, pPara->nNetLibMsg, 0, 0);
	}
}
#endif
