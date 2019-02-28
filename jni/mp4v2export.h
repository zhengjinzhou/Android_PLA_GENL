/*
 * mp4v2export.c
 * Create by LinZh107 on 2015-8-20.
 * Comp.right LinZh107@topvs 2015
 * Beyond of mp4v2-2.0.0 open source.
 * Use for writing video and audio stream to a mp4 file.
 * AVCoder is H264 and U/ALaw(G711).
 * */

#ifndef __MP4V2EXPORT_H__
#define __MP4V2EXPORT_H__

#include "include/mp4v2/mp4v2.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RetFalse 0
#define RetTrue 1
#define RetEOF	2

#define MAX_VIDEO_FRAME_SIZE (512*1024)
#define MAX_AUDIO_FRAME_SIZE (64*1024)

typedef struct mp4_tracks_params_
{
    MP4FileHandle 	pHfile;
    MP4TrackId  	aTrack;
    MP4TrackId  	vTrack;
    MP4SampleId   	aFrameId;
    MP4SampleId   	vFrameId;
    uint32_t		aFrameNum;
    uint32_t		vFrameNum;
    int 			isIFrame;
    uint8_t*		vBuffer;
    uint8_t*		aBuffer;
}mp4TracksParam_t;

int tp_mp4_create_mp4file(const char* path, void *pHandle);
int tp_mp4_write_videoFrame(void* fileHandle, int track_video,
		void *pData, int size, int keyframe);
int tp_mp4_write_audioFrame(void* fileHandle, int track_audio,
		void *pData, int size, int reverse);
int tp_mp4_finish_mp4file(void*	record_file);

#define _mark_h_ 0

int tp_mp4_open_mp4file(const char* path, void** params);
int tp_mp4_set_indexFrame(void* params, int destIndex);
int tp_mp4_read_videoFrame(void* params, int *nSize);
int tp_mp4_read_audioFrame(void* params, int *nSize);
int tp_mp4_close_mp4file(void* params);


#ifdef __cplusplus
}
#endif

#endif
