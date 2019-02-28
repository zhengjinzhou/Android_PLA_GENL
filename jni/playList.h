#ifndef __PLAY_LIST_H__
#define __PLAY_LIST_H__

#ifdef  __cplusplus
extern "C"
{
#endif
#include "dev_type.h"


#define MAX_FRAME_LIST_NUM 200		//frame list maxNum
#define FRAME_MAX_SIZE_T 512*1024   //same to VideoInstance.c size . add LinZh107
#define LOST_FRAME_NUM	25			//when lost frames larger then this num
									//remove some of frame list for new frame stored

#define VIDEO_FORMAT_D1         0x01
#define VIDEO_FORMAT_HD1        0x02
#define VIDEO_FORMAT_CIF        0x03
#define VIDEO_FORMAT_QCIF       0x04

#define MAX_NAME_LENGH			256
#define PLAY_TYPE_LOCAL_FILE	1
#define PLAY_TYPE_NET_STREAM	2
#define PLAY_TYPE_PLAY_BACK		3
#define PROTOCOL_TCP            0x01
#define PROTOCOL_UDP            0x02
#define PROTOCOL_MCAST          0x03


	typedef struct __Hi3510_frame_t
	{
		unsigned int device_type;
		unsigned int frame_size;
		unsigned int frame_no;
		unsigned char video_format;
		unsigned char frame_type;
		unsigned char frame_rate;
		unsigned char video_standard;
		unsigned int sec;
		unsigned int usec;
		unsigned long long pts;

		unsigned char *data;

		struct __Hi3510_frame_t *next;
	}Hi3510_frame_t;

	typedef struct __frame_list_t
	{
		Hi3510_frame_t *pframe;
	}frame_list_t;


	// add by LinZh107 at 2015-4-27
	// for multiple playView
	typedef	struct __TP_PlayListInstance_t
	{
		char *pframe;
		unsigned int prev_Vframe_no;
		Hi3510_frame_t *VframeList;
		pthread_mutex_t Video_mutex;

		Hi3510_frame_t *AframeList;
		pthread_mutex_t Audio_mutex;

		//pthread_cond_t cond_var;
	}tp_playlistinst_t;


	int st_initFrameList(tp_playlistinst_t *playinst);
	int st_destroyFrameList(tp_playlistinst_t *playinst);
	int st_inputFrameListTail(tp_playlistinst_t *playinst, char *inputFrame, int dataSize);
	Hi3510_frame_t *st_getKeyFramefromList(tp_playlistinst_t *playinst);
	Hi3510_frame_t *st_getFrameListHead(tp_playlistinst_t *playinst, int flags);
	void st_removeFrameListHead(tp_playlistinst_t *playinst, int flags);
	int getFrameNum(tp_playlistinst_t *playinst, int flags);

#ifdef  __cplusplus
}
#endif
#endif//__PLAY_LIST_H__

