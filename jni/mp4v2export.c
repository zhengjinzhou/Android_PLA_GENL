/*
 * mp4v2export.c
 * Create by LinZh107 on 2015-8-20.
 * Comp.right LinZh107@topvs 2015
 * Beyond of mp4v2-2.0.0 open source.
 * Use for writing video and audio stream to a mp4 file.
 * AVCoder is H264 and U/ALaw(G711).
 * */
#include "mp4v2export.h"
#include "VideoInstance.h"

#include <android/log.h>
#define	null 0
#define	LOG_TAG    "mp4v2export.c"
#define	LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define	LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

int tp_mp4_create_mp4file(const char* path, void *pHandle)
{
	VideoInstance_t *pVInst = (VideoInstance_t *)pHandle;

	//create mp4 file
	MP4FileHandle fileHandle = MP4Create(path, 0);
	if(fileHandle == MP4_INVALID_FILE_HANDLE){
		return RetFalse;
	}

	MP4SetTimeScale(fileHandle, 90000);

	char sps1 = pVInst->sps_pps[1];
	char sps2 = pVInst->sps_pps[2];
	char pps1 = pVInst->sps_pps[3];

	//track //ISO/IEC 14496-10 AVCProfileIndication,profile_compat,AVCLevelIndication
	MP4TrackId VideoTrack = MP4AddH264VideoTrack(fileHandle, 90000, 90000/(pVInst->frame_rate),
			pVInst->video_width, pVInst->video_height, sps1, sps2, pps1, 3);

	if(VideoTrack == MP4_INVALID_TRACK_ID)
	{
		MP4Close(fileHandle, 0);
		pVInst->record_file = NULL;
		return RetFalse;
	}

	//MP4TrackId AudioTrack = MP4AddAudioTrack(fileHandle, 16000, 8000, MP4_MPEG2_AAC_LC_AUDIO_TYPE);
	MP4TrackId AudioTrack = MP4AddULawAudioTrack(fileHandle, 8000);
	if(AudioTrack == MP4_INVALID_TRACK_ID)
	{
		MP4Close(fileHandle, 0);
		pVInst->record_file = NULL;
		return RetFalse;
	}

	int sps = (pVInst->sps_pps[0]<<24) + (sps1<<16) + (sps2<<8) + pps1;
	LOGI("width=%d, sps=%#x",pVInst->video_width, sps);

	pVInst->record_file = fileHandle;
	pVInst->track_video = VideoTrack;
	pVInst->track_audio = AudioTrack;

	//set sps and pps
	MP4AddH264SequenceParameterSet(fileHandle, VideoTrack, pVInst->sps_pps, 10);
	MP4AddH264PictureParameterSet(fileHandle, VideoTrack, &pVInst->sps_pps[10], 4);

	MP4SetVideoProfileLevel(fileHandle, 0x7F);
	MP4SetAudioProfileLevel(fileHandle, 0x02);
	//MP4SetTrackESConfiguration(fileHandle, audio, &ubuffer[0], 2);

	return RetTrue;
}

int tp_mp4_set_audio_alawPro(void* fileHandle, int track_audio)
{
	if(!MP4SetTrackIntegerProperty(fileHandle,	track_audio,
			"mdia.minf.stbl.stsd.alaw.channels", 1))
		return RetFalse;

	return RetTrue;
}

int tp_mp4_set_audio_ulawPro(void* fileHandle, int track_audio)
{
	if(!MP4SetTrackIntegerProperty(fileHandle,	track_audio,
			"mdia.minf.stbl.stsd.ulaw.channels", 1))
		return RetFalse;

	return RetTrue;
}

int tp_mp4_write_videoFrame(void* fileHandle, int track_video,
		void *pData, int size, int keyframe)
{
	if(!MP4WriteSample(fileHandle, track_video, pData, size, MP4_INVALID_DURATION, 0, keyframe))
	{
		LOGE("MP4WriteVideoSample fail ...");
		return RetFalse;
	}
	else
		return RetTrue;
}

int tp_mp4_write_audioFrame(void* fileHandle, int track_audio,
		void *pData, int size, int reverse)
{
	//while the frame is audio frame the keyFrame allway is true.
	if(!MP4WriteSample(fileHandle, track_audio, pData, size, MP4_INVALID_DURATION, 0, reverse))
	{
		LOGE("MP4WriteAudioSample fail ...");
		return RetFalse;
	}
	else
		return RetTrue;
}

int tp_mp4_finish_mp4file(void*	record_file)
{
	MP4Close(record_file, 0);
	return RetTrue;
}

#define _mark_c_ 0

int tp_mp4_open_mp4file(const char* path, void** params)
{
	if (!path){
		LOGE("path is INVALID_HANDLE ...");
		return RetFalse;
	}

	mp4TracksParam_t *trparam = malloc(sizeof(mp4TracksParam_t));
	memset(trparam, 0, sizeof(mp4TracksParam_t));

	trparam->vFrameId = 1;
	trparam->aFrameId = 1;

	trparam->pHfile = MP4Read(path);
	if(trparam->pHfile == MP4_INVALID_FILE_HANDLE)
	{
		LOGE("MP4Read fail ...");
		return RetFalse;
	}

	trparam->vTrack = MP4FindTrackId(trparam->pHfile, 0, MP4_VIDEO_TRACK_TYPE, 0);
	if (trparam->vTrack != MP4_INVALID_TRACK_ID){
		trparam->vFrameNum = MP4GetTrackNumberOfSamples(trparam->pHfile, trparam->vTrack);
	}
	else{
		LOGE("MP4GetVideoTrack fail ...");
	}

	trparam->aTrack = MP4FindTrackId(trparam->pHfile, 0, MP4_AUDIO_TRACK_TYPE, 0);
	if (trparam->aTrack != MP4_INVALID_TRACK_ID){
		trparam->aFrameNum = MP4GetTrackNumberOfSamples(trparam->pHfile, trparam->aTrack);
	}
	else{
		LOGE("MP4GetAudioTrack fail ...");
	}

	if((trparam->vFrameNum <= 0) && (trparam->aFrameNum <= 0)){
		LOGE("tp_mp4_open_mp4file failed ...");
		return RetFalse;
	}

	trparam->vBuffer = (uint8_t*)malloc(MAX_VIDEO_FRAME_SIZE * sizeof(char));
	trparam->aBuffer = (uint8_t*)malloc(MAX_AUDIO_FRAME_SIZE * sizeof(char));
	memset(trparam->vBuffer, 0, MAX_VIDEO_FRAME_SIZE * sizeof(char));
	memset(trparam->aBuffer, 0, MAX_AUDIO_FRAME_SIZE * sizeof(char));

	*params = (void*)trparam;
	LOGI("open_mp4file *params = %#010x", *params);

	return RetTrue;
}

int tp_mp4_set_indexFrame(void* params, int destIndex)
{
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;
	if(!trparam	)
		return RetFalse;

	trparam->vFrameId = destIndex;
	return RetTrue;
}

int tp_mp4_get_audio_alawPro(void* params)
{
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;
	if(!MP4SetTrackIntegerProperty(trparam->pHfile, trparam->aTrack,
			"mdia.minf.stbl.stsd.alaw.channels", 1))
		return RetFalse;

	return RetTrue;
}

int tp_mp4_get_audio_ulawPro(void* params)
{
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;
	if(!MP4SetTrackIntegerProperty(trparam->pHfile, trparam->aTrack,
			"mdia.minf.stbl.stsd.ulaw.channels", 1))
		return RetFalse;

	return RetTrue;
}

int tp_mp4_read_videoFrame(void* params, int *nSize)
{
	//LOGI("read_videoFrame *params = %#010x", params);
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;
	if(!trparam	|| (trparam->vTrack == MP4_INVALID_TRACK_ID))
		return RetFalse;

	if(trparam->vFrameId > trparam->vFrameNum){
		//LOGE("MP4 Read Sample video complete ...");
		return RetEOF;
	}

	if(!MP4ReadSample(trparam->pHfile, trparam->vTrack, trparam->vFrameId,
			&trparam->vBuffer, (uint32_t*)nSize,
			NULL, NULL, NULL, NULL ) )
	{
		//LOGE("MP4 Read Video Sample fail ..VSID = %d", trparam->vFrameId);
		return RetFalse;
	}

	trparam->vFrameId ++;

	return RetTrue;
}

int tp_mp4_read_audioFrame(void* params, int *nSize)
{
	//LOGI("read_audioFrame *params = %#010x", params);
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;
	if(!trparam	|| (trparam->aTrack == MP4_INVALID_TRACK_ID))
		return RetFalse;

	if(trparam->aFrameId > trparam->aFrameNum){
		//LOGE("MP4 Read Sample video complete ...");
		return RetEOF;
	}

	if(!MP4ReadSample(trparam->pHfile, trparam->aTrack, trparam->aFrameId,
			&trparam->aBuffer, (uint32_t*)nSize,
			NULL, NULL, NULL, NULL ) )
	{
		//LOGE("MP4 Read Audio Sample fail ..ASID = %d",trparam->aFrameId);
		return RetFalse;
	}

	trparam->aFrameId ++;

	return RetTrue;
}


int tp_mp4_close_mp4file(void *params)
{
	//LOGI("close_mp4file *params = %#010x", params);
	mp4TracksParam_t *trparam = (mp4TracksParam_t *)params;
	if(!trparam){
		return  RetFalse;
	}

	MP4Close(trparam->pHfile, 0);
	free(trparam->vBuffer);
	free(trparam->aBuffer);
	free(trparam);

	return RetTrue;
}
