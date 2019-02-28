/*
**	author zhangrs
**	created 2010.06.22
**	for android platform
**/

#include "playlib.h"

#include <android/log.h>
#define null  0
#define LOG_TAG  "playlib.c"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

int g_decodeCnt; 				//abandon 1 frame every 2 frame.
int g_hd_maxWidth = 1280;		//if the video width larger then this value define it's a HD frame
int g_hd_frameRate = 15;		//while video width bigger then this width,
								//abandon some frame to make sure the decoding work without delay
AVCodec *pVCodec = NULL;
my_decoder_t *my_decoder[MAX_ITEM];	//每路视频都需要一个decoder解码h264帧

int init_ffmpeg_lib()
{
	/* Initialize libavcodec, and register all codecs and formats. */
	av_register_all();
	avcodec_register_all();

	int i=0;
	while(i<MAX_ITEM){
		my_decoder[i] = NULL;
		i++;
	}

	pVCodec = avcodec_find_decoder(CODEC_ID_H264);
	if (!pVCodec)
		return -1; 
	else
		return 0;
}

int set_decoder_param(int viindex, int w, int rate)
{
	g_hd_frameRate = rate;
	g_hd_maxWidth	= w;
	return 0;
}

int init_decoder(int viindex)
{
	if(my_decoder[viindex])
		return 0;

	my_decoder[viindex] = malloc(sizeof(my_decoder_t));
	if(!my_decoder[viindex])
		return -1;
	memset(my_decoder[viindex], 0, sizeof(my_decoder_t));

	my_decoder[viindex]->pCodecCtx = avcodec_alloc_context3(pVCodec);
	if(!my_decoder[viindex]->pCodecCtx)
		return -2;

	if (avcodec_open2(my_decoder[viindex]->pCodecCtx, pVCodec, NULL) < 0)
		return -3;

	my_decoder[viindex]->pAVFrame  = av_frame_alloc();
	if (!my_decoder[viindex]->pAVFrame )
		return -4;

	my_decoder[viindex]->pFrameRGB  = av_frame_alloc();
	if(!my_decoder[viindex]->pFrameRGB)
		return -5;
	return 0;
}

//不使用播放库时要release_player
void release_decoder(int viindex)
{
	if(!my_decoder[viindex])
		return ;
	if(my_decoder[viindex]->pFrameRGB)
	{
		av_free(my_decoder[viindex]->pFrameRGB);
		my_decoder[viindex]->pFrameRGB = null;
	}

	if(my_decoder[viindex]->pAVFrame)
	{
		av_free(my_decoder[viindex]->pAVFrame);
		my_decoder[viindex]->pAVFrame = null;
	}
	if(my_decoder[viindex]->pCodecCtx)
	{
		avcodec_close(my_decoder[viindex]->pCodecCtx);
		av_free(my_decoder[viindex]->pCodecCtx);
		my_decoder[viindex]->pCodecCtx = null;
	}

	free(my_decoder[viindex]);
	my_decoder[viindex] = NULL;
	//LOGI("debug release ffmpeg decoder.");
}

//	Data： 2014-4-25  by:LinZh107  增加多 格式支持
//	Data：2014-5-7 更新解码库 和 更换解码方案
int  decode_H264_frame(int frame_rate, int frame_size, char *frame_data,
		int *pParam, char* pOutBuf, int viindex)
{
	//LOGI("In %s function %d",__FUNCTION__, viindex);
	AVPacket avpkt = {0};
	avpkt.size = frame_size;
	avpkt.data = frame_data;
	int got_picture = 0;

	avcodec_decode_video2(my_decoder[viindex]->pCodecCtx, my_decoder[viindex]->pAVFrame, &got_picture, &avpkt);
	if (got_picture)
	{
		//高清时计算要丢弃的帧数  LinZh107
		if(my_decoder[viindex]->pCodecCtx->width > g_hd_maxWidth)
		{
			if( frame_rate > g_hd_frameRate )
			{
				if(g_decodeCnt > g_hd_frameRate /(frame_rate - g_hd_frameRate))
				{
					LOGE("abandon this frame!");
					g_decodeCnt = 1;
					return -1;
				}
			}
		}
		g_decodeCnt++;
		avpicture_fill( (AVPicture *)my_decoder[viindex]->pFrameRGB, pOutBuf, PIX_FMT_RGB565,
				my_decoder[viindex]->pCodecCtx->width,
				my_decoder[viindex]->pCodecCtx->height);
		//avpicture_fill((AVPicture *)pFrameRGB, pOutBuf, PIX_FMT_RGB565, *pWidth, *pHeight);

		/*
		 * *************** 以下为旧的接口，会引起内存泄露 *****************
		 * */
		my_decoder[viindex]->pSwsCtx = sws_getCachedContext(my_decoder[viindex]->pSwsCtx,
				my_decoder[viindex]->pCodecCtx->width,
				my_decoder[viindex]->pCodecCtx->height,
				my_decoder[viindex]->pCodecCtx->pix_fmt,
				pParam[0], pParam[1], PIX_FMT_RGB565,
				SWS_FAST_BILINEAR, NULL, NULL, NULL);

		//pSwsCtx = sws_getContext(pCodecCtx->width, pCodecCtx->height,\
				my_decoder[viindex]->pCodecCtx->pix_fmt, pParam[0], pParam[1], \
				PIX_FMT_RGB565, SWS_FAST_BILINEAR, NULL, NULL, NULL);

		if (my_decoder[viindex]->pSwsCtx == NULL){
			LOGE("could not initialize conversion context\n");
			return -2;
		}

		//将YUV转化为RGB
		sws_scale(my_decoder[viindex]->pSwsCtx,
				(const uint8_t* const *) my_decoder[viindex]->pAVFrame->data,
				my_decoder[viindex]->pAVFrame->linesize,
				0, my_decoder[viindex]->pCodecCtx->height,
				(const uint8_t* const *) my_decoder[viindex]->pFrameRGB->data,
				my_decoder[viindex]->pFrameRGB->linesize);

		pParam[0] = my_decoder[viindex]->pCodecCtx->width ;
		pParam[1] = my_decoder[viindex]->pCodecCtx->height ;

		//LOGI("Out %s function %d",__FUNCTION__, viindex);
		return 0;
	}
	else
		//LOGE("avcodec_decode_video2 failed!\n");
		return -3;
}
