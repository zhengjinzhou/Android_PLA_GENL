/*
**	author zhangrs
**	created 2010.06.22
**	for android platform
**/
#ifndef __PLAYLIB_H__
#define __PLAYLIB_H__

#include <stdio.h>
#include <include/libavcodec/avcodec.h>
#include <include/libavutil/frame.h>
#include <include/libavformat/avformat.h>
#include <include/libswscale/swscale.h>

/*void cvt_420p_to_rgb565(int width,	int height,
						const unsigned char *yuv_y,
						const unsigned char *yuv_u,
						const unsigned char *yuv_v,
						unsigned short *dst);*/

typedef struct my_decoderParams{
	int isInit;
	struct SwsContext *pSwsCtx;		// sws Context
	AVCodecContext *pCodecCtx;	// Codec Context
	AVFrame *pAVFrame;		  		// Raw Frame
	AVFrame	*pFrameRGB;				// RGB frame
}my_decoder_t;

int init_ffmpeg_lib(void);

#define MAX_ITEM 16					//最多只能支持16路视频.

//使用时先init_player
int init_decoder(int viindex);

int set_decoder_param(int viindex, int w, int rate);

//不使用播放库时要release_player
void release_decoder(int viindex);

//解析h264格式的视频
//int decodeH264video(char *rawbuf,int insize, char* output_buf, int *outsize); // 2013-11-22
//int decodeH264video(char *rawbuf,int insize, char* output_buf, int *pwidth,int *pheight);
//AVFrame *decodeH264video2(char *frame_data, int frame_size, int frame_rate, int *pwidth, int *pheight);

int decode_H264_frame(int frame_rate, int frame_size, char *frame_data,
		int *pParam, char* pOutBuf, int viindex);
#endif /* __PLAYLIB_H__ */
