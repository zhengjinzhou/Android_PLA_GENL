/*
 * VideoInstance.h
 *
 *  Created on: 2009-10-16
 *      Author: pc
 */

#ifndef VIDEOINSTANCE_H_
#define VIDEOINSTANCE_H_

#include <pthread.h>
#include <stdio.h>
#include <include/libavformat/avformat.h>
#include "st_cuapi.h"
#include "tcpsock.h"
#include "list.h"

#  ifdef __cplusplus
extern "C"
{
#  endif /* __cplusplus */

//	struct _VideoInstance;

	typedef struct vtag_Thrd_Para {
		struct _VideoInstance* pClient;
		int pServerPara;
	} THREAD_VIDEOINSTANCE_PARA;


	typedef struct _VideoInstance {
		THREAD_VIDEOINSTANCE_PARA thrdPara;
		RECVCALLBACK m_lpFunc;				// callback function
		void* m_lHandle;            	 //params for callback

		tcpsocket_t		m_hTcpSocket;
		//UdpSocket		m_hUdpSocket;
		tcpsocket_t		m_hTcpDataSocket;

		int 			m_nProtocolType;
		int 			m_nInviteSeq;
		char 			m_callId[32];
		int 			m_ErrCode;

		MEDIA_DATA_INFO m_MediaDataInfo;
		GUINFO          m_guInfo;
		
		pthread_t   m_pThread_MduHeart;   // MDU heartbeat Thread's pointer
		pthread_t   m_pThread_RecvData;   // recieve data Thread's pointer
		int       	m_MduHeartThreadflag;
		int       	m_RecvDataThreadflag;

		//if the video recevie thread is ready
		int       	m_bLinked;

		//if this video instance is recording
		int			m_bRecord;
		int 		bInit_params;
		int 		video_width;
		int 		video_height;
		int 		frame_rate;
		char 		sps_pps[32];

		/**
		 * record the H264 raw data to a .mp4 file
		 * Author LinZh107
		 * */
		/*
		 * work with mp4v2-2.0.0 library
		 * */
		RECORDCALLBACK	rcMp4Func;
		void*			record_file;
		int				track_video;
		int 			track_audio;

		void*			playinst;			//add By LinZh107 for MultiplayView
		pthread_mutex_t dataList_mutex;
	} VideoInstance_t;

	typedef struct _VideoInstance_list {
		VideoInstance_t videoi;
		struct list_head list;
	} VideoInstance_list;

	int videoi_VideoInstance(VideoInstance_t *pIns, const CLIENTINFO* pClient);

	inline void videoi_delVideoInstance(VideoInstance_t *pIns);

	inline void videoi_SetLinkState(VideoInstance_t * pIns, int bLinked);
	inline int videoi_GetLinkState(VideoInstance_t * pIns);

	void videoi_SetMediaDataInfo(VideoInstance_t * pIns, const MEDIA_DATA_INFO* pInfo);
	void videoi_GetMediaDataInfo(VideoInstance_t * pIns, MEDIA_DATA_INFO* pInfo);

	inline void videoi_SetCameraInfo(VideoInstance_t * pIns, const GUINFO guInfo);

	void videoi_StartThreads(VideoInstance_t * pIns, int pServerPara);

	void videoi_BreakSession(VideoInstance_t * pIns);

	void videoi_CancelInvite(VideoInstance_t * pIns);
	void videoi_QuitInstance(VideoInstance_t * pIns);
	
	void st_media_countLost(void *data, unsigned int *lastseq, unsigned int *cnt, unsigned int *rate,
	        unsigned int *lost, unsigned int *lostrate);
	
	int videoi_ConnectToServer(VideoInstance_t * pIns, int pServerPara, const char* lpszMduIp, unsigned int uMduPort);
	int videoi_InviteMedia(VideoInstance_t * pIns, int pServerPara, MDU_ROUTE_INFO mduInfo, GUINFO* pGuInfo);
	int voidei_Reconnect(VideoInstance_t * pIns, int pServerPara);

//CCriticalSection    m_csLinked;
#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* VIDEOINSTANCE_H_ */
