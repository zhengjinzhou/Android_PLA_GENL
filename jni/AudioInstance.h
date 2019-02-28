/*
 * AudioInstance.h
 * Modify by LinZh107 @topvs.com
 * 2015-06-17
 */

#ifndef __AUDIOINSTANCE_H__
#define __AUDIOINSTANCE_H__

#include <pthread.h>
#include "netdatatype.h"
#include "st_cuapi.h"
#include "tcpsock.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
    typedef struct atag_Thrd_Para{
        struct _AudioInstance* pAInstance;
        void* pLibInstance;
    }THREAD_AUDIOINSTANCE_PARA;
        
    typedef struct _AudioInstance{
        THREAD_AUDIOINSTANCE_PARA thrdPara;
        void*           lHandle;        // 窗口句柄
        RECVCALLBACK    lpFunc;         // 对应的回调函数指针
        int             nProtocolType;  // 协议类型 0-UDP 1-TCP
        tcpsocket_t     m_hTcpSocket;
        tcpsocket_t     m_hTcpDataSocket;
        GUINFO          m_guInfo;       // 摄像机描述
        char            m_callId[32];
        char            m_szMduIp[128];
        int             m_nMduPort;
        int             m_ErrCode;
        int             m_nCount;
        
        RTP_FIXED_HEADER RtpHead;
        pthread_t       m_pThread_RecvAudioData;   //接收音频数据的线程 的指针
        pthread_mutex_t recv_mutex;
        int             m_RecvAudioThreadflag;

        int             m_bLinked;
        void*           playinst;		//add By LinZh107 for MultiplayView
        
    }AUDIOINSTANCE_T, *P_AUDIOINSTANCE_T;
    
    void*   audioi_CreateInstance(void);
    void    audioi_DeleteInstance(P_AUDIOINSTANCE_T pAInst);
    
    int     audioi_InitInstance(P_AUDIOINSTANCE_T pAInst, int nProtocolType);
    int     audioi_QuitInstance(P_AUDIOINSTANCE_T pAInst);
    
    int     audioi_ConnectToServer(P_AUDIOINSTANCE_T pAInst, void* pLibInstance, const char* lpszMduIp, int uMduPort);
    int     audioi_InviteMedia(P_AUDIOINSTANCE_T pAInst, void* pLibInstance, MDU_ROUTE_INFO mduInfo, GUINFO *pGuInfo);
    
    int     audioi_StartAudioRecv(P_AUDIOINSTANCE_T pAInst, void* pLibInstance);
    int		audioi_StopAudioRecv(P_AUDIOINSTANCE_T pAInst);
    
	int     audioi_SendVoiceData(P_AUDIOINSTANCE_T pAInst, const void* lpBuf, int nLen);
    int     audioi_RtpSendData(RTP_FIXED_HEADER* pHead, int sock, void* pAddr, const char *frmData, int len);    

	int     audioi_SaveFrameData(P_AUDIOINSTANCE_T pAInst, char* pszFrame, int nFrameSize, char* pszNode, int nNodeLen,
                          RTP_FIXED_HEADER RtpHead, int bHead);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AUDIOINSTANCE_H__ */
