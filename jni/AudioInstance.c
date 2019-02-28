/*
 * AudioInstance.c
 * Modify by LinZh107 @topvs.com
 * 2015-06-17
 */

#include "AudioInstance.h"
#include "SipAPI.h"
#include "netglobal.h"
#include "st_cuapi.h"
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <android/log.h>
#define null 0
#define  LOG_TAG    "AudioInstance.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
//int RtpSendData(RTP_FIXED_HEADER* RtpHead, SOCKET sock, LPSOCKADDR pAddr, const char *frmData, int len);
//////////////////////////////////////////////////////////////////////////
static void *Thread_AudioRecv(void* lParam);

void *audioi_CreateInstance(void)
{
    P_AUDIOINSTANCE_T pAInst = (P_AUDIOINSTANCE_T)malloc(sizeof(AUDIOINSTANCE_T));
    if (NULL == pAInst) {
        return NULL;
    }
    memset((void*)pAInst, 0, sizeof(AUDIOINSTANCE_T));
 
    pAInst->m_hTcpSocket.m_hSocket = -1;
    pAInst->m_hTcpDataSocket.m_hSocket = -1;
    pAInst->m_pThread_RecvAudioData = 0;
    return pAInst;
}

void audioi_DeleteInstance(P_AUDIOINSTANCE_T pAInst)
{
	LOGI(" >>> in %s\n",__FUNCTION__);
    if (pAInst) {
        free(pAInst);
    }
    return ;
}

int audioi_InitInstance(P_AUDIOINSTANCE_T pAInst, int nProtocolType)
{
    if(pAInst->m_hTcpSocket.m_hSocket != -1)
    {
        close(pAInst->m_hTcpSocket.m_hSocket);
        pAInst->m_hTcpSocket.m_hSocket = -1;
    }
    
    if(pAInst->m_hTcpDataSocket.m_hSocket != -1)
    {
        close(pAInst->m_hTcpDataSocket.m_hSocket);
        pAInst->m_hTcpDataSocket.m_hSocket = -1;
    }
    
    if (nProtocolType != 1)
        return -1;

    if( init_p_socket_tcp(&pAInst->m_hTcpSocket) < 0)
    {
        return -1;
    }
    if( init_p_socket_tcp(&pAInst->m_hTcpDataSocket) < 0)
    {
        return -1;
    }
    
    pAInst->nProtocolType = nProtocolType;
    
	return 0;
}

int audioi_ConnectToServer(P_AUDIOINSTANCE_T pAInst, void* pLibInstance, const char* lpszMduIp, int uMduPort)
{
	LOGI(" >>> in %s\n",__FUNCTION__);
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA)pLibInstance;
    pAInst->m_nMduPort = ST_MDU_VOICE_OFFSET + uMduPort;
    strcpy(pAInst->m_szMduIp, lpszMduIp);
    
    //LOGE("audioi_ConnectToServer lpszMduIp = %s\n",lpszMduIp);
    int iRet = pAInst->m_hTcpSocket.set_tcp_link(&pAInst->m_hTcpSocket, lpszMduIp, pAInst->m_nMduPort);
    if(0 != iRet)
		return iRet;

    iRet = pAInst->m_hTcpSocket.m_connect(&pAInst->m_hTcpSocket, pPara->CmdTimeOut);
    
    return iRet;
}

int audioi_InviteMedia(P_AUDIOINSTANCE_T pAInst, void* pLibInstance, MDU_ROUTE_INFO mduInfo, GUINFO *pGuInfo)
{
    LOGI(" >>> in %s\n",__FUNCTION__);
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA)pLibInstance;
    
    GUINFO guInfo = pAInst->m_guInfo;
    
    CreateCallID(pAInst->m_callId);
    
	char buf[1024] = {0};
	int  nSeqNum = 1;
	int  nBufLen = CreatePack_MDU_VoiceChart(buf, nSeqNum, pAInst->m_callId, &guInfo, pAInst->nProtocolType, mduInfo);
	pAInst->m_hTcpSocket.m_senddata(&pAInst->m_hTcpSocket, buf, nBufLen);

	bzero(buf, sizeof(buf));
	int nRecv = pAInst->m_hTcpSocket.recv_sippackage(&pAInst->m_hTcpSocket, buf, sizeof(buf)-1, pPara->CmdTimeOut);
	if( nRecv <=0 )
	{
		pAInst->m_ErrCode = 0x80008022;
		return -1;
	}
	buf[nRecv] = '\0';

	sip_cmd_t  SipCmd;
	bzero(&SipCmd, sizeof(SipCmd));
	char  szXmlBuf[512] = {0};

	st_sip_parseSip(buf, (int)strlen(buf), &SipCmd, szXmlBuf);

	if(SipCmd.answerNum != 200)
	{
		switch(SipCmd.answerNum)
		{
		case 404:
			pAInst->m_ErrCode = 0x80008041;
			break;
		case 486:
			pAInst->m_ErrCode = 0x80008044;
			break;
		default:
			pAInst->m_ErrCode = 0x80008042;
			break;
		}
        LOGE("pAInst->m_ErrCode = %0x\n", pAInst->m_ErrCode);
		return -1;
	}

	char szPort[128];
	char* pStr = GetMiddleString(buf, "mediaPort>", "mediaPort", szPort, sizeof(szPort));
    if( pStr == NULL)
	{
		pAInst->m_ErrCode = 0x80008043;
		return -1;
	}
	int  nUdpPort = atoi(szPort);
	if( nUdpPort <= 0 )
	{
		pAInst->m_ErrCode = 0x80008043;
		return -1;
	}
    
    if (pAInst->nProtocolType != 1){
        return -1;
    }
    
	bzero(buf, sizeof(buf));
	nBufLen = CreatePack_MDU_RealStartACK(buf, 0, pAInst->m_callId, &guInfo);
	pAInst->m_hTcpSocket.m_senddata(&pAInst->m_hTcpSocket, buf, nBufLen);

    char szMduIp[256] = {0};
    int nPortTemp;
    
    // 10-如果请求通过, 设置UDP连接参数, 发送探测数据包, 然后开启相应的线程 ;如果TCP连接，直接发起connect
    pAInst->m_hTcpSocket.get_tcp_link(&pAInst->m_hTcpSocket, szMduIp, &nPortTemp);
    if (pAInst->m_hTcpDataSocket.set_tcp_link(&pAInst->m_hTcpDataSocket, szMduIp, nUdpPort) < 0)
        return -1;
    if(pAInst->m_hTcpDataSocket.m_connect( &pAInst->m_hTcpDataSocket, 3000)!=0)
    {
        LOGE("connect DataSock error!\n");
        return -1;
    }
    
    return 0;
}

#define STILL_ACTIVE 1
int audioi_QuitInstance(P_AUDIOINSTANCE_T pAInst)
{
	LOGE(" >>> in %s\n",__FUNCTION__);
	if(!pAInst)
		return -1;

    if(pAInst->m_RecvAudioThreadflag >= 2)
    {
        pAInst->m_RecvAudioThreadflag = 10;
        int cnt = 300;
        while (cnt > 0)
        {
            if (pAInst->m_RecvAudioThreadflag == 1)
                break;
            usleep(5000); //modify 900*1000 to 5000
            cnt --;
        }
    }
    
    if(pAInst->m_hTcpSocket.m_hSocket != -1)
    {
        close(pAInst->m_hTcpSocket.m_hSocket);
        pAInst->m_hTcpSocket.m_hSocket = -1;
    }
   
    if(pAInst->m_hTcpDataSocket.m_hSocket != -1)
    {
        close(pAInst->m_hTcpDataSocket.m_hSocket);
        pAInst->m_hTcpDataSocket.m_hSocket = -1;
    }
    
    if (pAInst->m_RecvAudioThreadflag != 1 )
        return -1;
    pAInst->m_RecvAudioThreadflag = 0;
    
    return 0;
}

int audioi_SendVoiceData(P_AUDIOINSTANCE_T pAInst, const void* lpBuf, int nLen)
{
	//LOGI(" >>> in %s\n",__FUNCTION__);
    if (!pAInst) {
        return -1;
    }
	if (pAInst->nProtocolType != 1)
        return -1;
    int nSent = audioi_RtpSendData(&pAInst->RtpHead, pAInst->m_hTcpDataSocket.m_hSocket, NULL,(const char*)lpBuf, nLen);

	return nSent;
}

int audioi_RtpSendData(RTP_FIXED_HEADER* pHead, int sock, void* pAddr, const char *frmData, int len)
{
    /* struct sockaddr_in* pAddr */
	int nCount = 0;
	int nFixSize = 0;
	int nActualLen = 0;
	int i=0;
	const unsigned int nRtpHeader = sizeof(RTP_FIXED_HEADER)+sizeof(RTP_HEADER_EXT); 

	RTP_FIXED_HEADER* pRtpHdr = pHead;
	RTP_HEADER_EXT RtpExt;
	bzero(&RtpExt, sizeof(RTP_HEADER_EXT));
	char glbRtpPkt[3*1024];

	pRtpHdr->frmType = RTP_AUDIO_FRAME;
	pRtpHdr->session = RTP_AUDIO_SSRC;
	pRtpHdr->x = 1;
	pRtpHdr->cc = 1;	
	pRtpHdr->ts++;
 	pRtpHdr->channel = 1;

	nCount = len/MTU_LEN + 1;
	nFixSize = len/nCount;
	
	int nSentAll = 0;
	int nSendOnce = 0;
	for(i=0; i<nCount; i++){		
		nActualLen = nFixSize;
		if(i == nCount-1)
			nActualLen = len - i*nFixSize;
		
		pRtpHdr->seq++;
		RtpExt.len = nActualLen;
        char * pRtpPkt = (char*)&glbRtpPkt;
		memcpy(pRtpPkt, pRtpHdr,  sizeof(RTP_FIXED_HEADER));
		memcpy(pRtpPkt+sizeof(RTP_FIXED_HEADER), &RtpExt, sizeof(RTP_HEADER_EXT));
		memcpy(pRtpPkt+nRtpHeader, frmData + i*nFixSize, nActualLen);

		int nLost =1;
		if(nLost!=0)
		{
			if (pAddr)
			{
                //udp protocol
				nSendOnce = (int)sendto(sock,pRtpPkt,nActualLen+nRtpHeader, 0, pAddr,sizeof(struct sockaddr_in));
			}
			else
			{
				nSendOnce = Send(sock, pRtpPkt, nActualLen+nRtpHeader, 0);
				if (nSendOnce != (int)(nActualLen+nRtpHeader))
                    LOGE("send audio data to pu error\n");
			}
			if( nSendOnce >0)
				nSentAll += nSendOnce;
            else{
                LOGE("audioi_RtpSendData len = %d\n",nSendOnce);
				return -1;
            }
		}
	}
	return nSentAll;
}

int SaveFrameData(P_AUDIOINSTANCE_T pAInst, char* pszFrame, int nFrameSize, char* pszNode, int nNodeLen, RTP_FIXED_HEADER RtpHead, int bHead)
{
    const RTP_FIXED_HEADER* const pRtpNode   = (RTP_FIXED_HEADER*)pszNode;
    
    int nRtpHeadLen = 0;
    if (pRtpNode->x > 0)
    {
        nRtpHeadLen = sizeof(RTP_FIXED_HEADER) + sizeof(RTP_HEADER_EXT)*pRtpNode->cc;
    }
    else
    {
        nRtpHeadLen = sizeof(RTP_FIXED_HEADER);
    }
    if(pRtpNode->seq < RtpHead.seq)
    {
        if((nNodeLen - nRtpHeadLen) * (RtpHead.seq - pRtpNode->seq + 1) >= nFrameSize)
            return -1;
        
        memcpy(pszFrame + ((nNodeLen - nRtpHeadLen) * (RtpHead.seq - pRtpNode->seq)), pszFrame,
               nFrameSize - (nNodeLen - nRtpHeadLen));
        memcpy(pszFrame, pszNode + nRtpHeadLen, nNodeLen - nRtpHeadLen);
        
        bHead = 0;
    }
    else
    {
        if((nNodeLen - nRtpHeadLen) * (pRtpNode->seq - RtpHead.seq + 1) >= nFrameSize)
            return -1;
        
        memcpy(pszFrame+((nNodeLen - nRtpHeadLen) * (pRtpNode->seq - RtpHead.seq)),
               pszNode + nRtpHeadLen, (nNodeLen - nRtpHeadLen));
        
        bHead = -1;
    }
    
    return 0;
}

#define FRAME_MAX_SIZE_T 64*1024
int audioi_StartAudioRecv(P_AUDIOINSTANCE_T pAInst, void* pLibInstance)
{
	LOGI(" >>> in %s\n",__FUNCTION__);
    if (!pAInst || !pLibInstance) {
        return -1;
    }
    pAInst->thrdPara.pLibInstance = pLibInstance;
    pAInst->thrdPara.pAInstance = pAInst;
    
    int ret = 0;//*/pthread_create(&pAInst->m_pThread_RecvAudioData, NULL, Thread_AudioRecv, &pAInst->thrdPara);
    if(ret < 0 )
    {
        pAInst->m_RecvAudioThreadflag = 0;
        return -1;
    }
    return 0;
}

int audioi_StopAudioRecv(P_AUDIOINSTANCE_T pAInst)
{
    if(!pAInst)
        return -1;

	char buf[1024] = {0};
	int  nBufLen = CreatePack_Voice_Bye(buf, 0,  pAInst->m_callId, &pAInst->m_guInfo);
    pAInst->m_hTcpSocket.m_senddata(&pAInst->m_hTcpSocket, buf, nBufLen);

	return 0;
}

static void *Thread_AudioRecv(void* lParam)
{
	LOGI("Create----->Thread_AudioRecv Thread!\n");
	pthread_detach(pthread_self());

    THREAD_AUDIOINSTANCE_PARA* pParam = (THREAD_AUDIOINSTANCE_PARA*)lParam;
	P_AUDIOINSTANCE_T pAInst = (P_AUDIOINSTANCE_T)pParam->pAInstance;
    
    if (pAInst->nProtocolType != 1)
        return 0;

	unsigned int     nLastSeq = 0;  //
	unsigned int     nLastTs  = 0;  //
	
	//char* full_frame_data[FRAME_MAX_SIZE_T];
    char* full_frame_data = malloc(FRAME_MAX_SIZE_T * sizeof(char));
	memset(full_frame_data, 0, FRAME_MAX_SIZE_T);
	if(full_frame_data == NULL){
		return 0;
	}
	
	char  bufPack[1400] = {0};
	DWORD  nBufSize = 0;
	
	unsigned int nRtpHeadLen = sizeof(RTP_FIXED_HEADER) + sizeof(RTP_HEADER_EXT);
	const unsigned int nFrameHeadLen = sizeof(frame_head_t);
	const FRAMHEAD_T*  const  pFrameHead = (FRAMHEAD_T*)(char*)full_frame_data;
	const RTP_FIXED_HEADER* const pRtpHead   = (RTP_FIXED_HEADER*)bufPack;
	const RTP_HEADER_EXT* const pRtpExt = (RTP_HEADER_EXT*)(bufPack + sizeof(RTP_FIXED_HEADER));
	DWORD nFullFrameSize = 0;
    
    struct timeval tv;
    long long dwLastRecv;
    long long dwLastSend;
    
    memset(full_frame_data, 0, FRAME_MAX_SIZE_T);// sizeof(full_frame_data)//modified by zhusy
    
    dwLastRecv = (long long)tv.tv_sec*1000+(long long)tv.tv_usec/1000;
    dwLastSend = (long long)tv.tv_sec*1000+(long long)tv.tv_usec/1000;

	RTP_FIXED_HEADER RtpHead;
	memset(&RtpHead, 0, sizeof(RTP_FIXED_HEADER));
    
    //tell app to close when exit
    pAInst->m_RecvAudioThreadflag = 2;
    
    while(1)
	{
        // 1. 是否要求退出
        if(pAInst->m_RecvAudioThreadflag == 10){
            break;
        }
        int nRecv;
        gettimeofday(&tv, NULL);
        
        nRecv = pAInst->m_hTcpDataSocket.m_receive(&pAInst->m_hTcpSocket, bufPack, nRtpHeadLen, 500);
        if (nRecv <= 0 || nRecv != (int)nRtpHeadLen)
        {
            goto RECV_ERR;
        }
        nRecv = pAInst->m_hTcpDataSocket.m_receive(&pAInst->m_hTcpSocket, bufPack+nRtpHeadLen, pRtpExt->len, 500);
        if (nRecv <= 0 || nRecv != (int)pRtpExt->len)
        {
            goto RECV_ERR;
        }
        if (pRtpHead->x > 0){
            nRtpHeadLen = sizeof(RTP_FIXED_HEADER) + sizeof(RTP_HEADER_EXT)*pRtpHead->cc;
        }
        else{
            nRtpHeadLen = sizeof(RTP_FIXED_HEADER);
        }	
        nRecv += nRtpHeadLen;
        
        // 3.不能正常接收的处理
        if( nRecv <= 0)
        {
    RECV_ERR:
            if( ((long long)tv.tv_sec*1000+(long long)tv.tv_usec/1000) - dwLastRecv > 8000
               || ((long long)tv.tv_sec*1000+(long long)tv.tv_usec/1000) < dwLastRecv)
            {
                pAInst->m_bLinked = 0;
                //LOGE("不能正常接收的处理！1\n");
                memset(full_frame_data, 0, FRAME_MAX_SIZE_T);
                nBufSize = 0;
            }
            continue;
        }
        
        // 4. 正常接收后的处理
        dwLastRecv = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000; //记录接收时间

        // 5. 接收长度不够一个RPT头的话丢弃重来
        if(nRecv < (int)nRtpHeadLen)
        {
            LOGE("Error__media Length: nRecv=%d(<nRtpHeadLen), contune...\n\n",nRecv);
            continue;
        }
        
        // 6. 如果包长度大于RPT头，则拷贝数据
        else
        {
            // 序号正确 ts和上一个是一样的话就是同一侦数据，继续接收
			if(nLastTs == pRtpHead->ts)
			{
				nLastSeq = pRtpHead->seq;
				if(pAInst->nProtocolType == 1)
				{
					memcpy(full_frame_data+nBufSize, bufPack+ nRtpHeadLen, nRecv- nRtpHeadLen);
                    LOGE("continue recv the audio data, because frame not finish.\n");
				}
				else
				{
					int bFrameHead = 0;
					SaveFrameData(pAInst, (char*)full_frame_data, FRAME_MAX_SIZE_T, bufPack, 1400, RtpHead, bFrameHead);
					if(bFrameHead)
						memcpy(&RtpHead, bufPack, sizeof(RTP_FIXED_HEADER));
				}
				
				nBufSize += nRecv- nRtpHeadLen;
				if (nBufSize > sizeof(full_frame_data) - MTU_LEN)
				{
					nBufSize = 0;
				}
				continue;
			}
			else
			{
				if( nBufSize >= nFrameHeadLen)
				{
					nFullFrameSize = pFrameHead->frame_size + nFrameHeadLen;
					if(nBufSize == nFullFrameSize)
					{
                        pAInst->lpFunc(pAInst->lHandle, full_frame_data, nFullFrameSize, 0);
                        LOGE("nLastSeq:%d, pRtpHead->seq:%d, nLastTs:%d, pRtpHead->ts:%d, recv_frame_size:%d\n",
                               nLastSeq, pRtpHead->seq, nLastTs, pRtpHead->ts, nFullFrameSize);
						nBufSize = 0;
					}
				}
				nLastSeq = pRtpHead->seq;
				nLastTs = pRtpHead->ts;
				nBufSize = 0;
				
				memset(full_frame_data, 0, FRAME_MAX_SIZE_T);
				memcpy(full_frame_data+ nBufSize, bufPack+ nRtpHeadLen, nRecv- nRtpHeadLen);
				nBufSize = nRecv - nRtpHeadLen;
				memcpy(&RtpHead, bufPack, sizeof(RTP_FIXED_HEADER));
			}
		}
	}
    pAInst->m_RecvAudioThreadflag = 1;
	free(full_frame_data);
	LOGI("Exit---->Thread_AudioRecv Thread!");
	return 0;
}
