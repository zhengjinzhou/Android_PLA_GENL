#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <android/log.h>

#include "playList.h"


#define null  0
#define LOG_TAG  "playlist.c"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void freelist(Hi3510_frame_t *List) //add LinZh107
{
	Hi3510_frame_t *tmp = List;
	Hi3510_frame_t *tmp1 = tmp;
	while (tmp1) {
		tmp = tmp1;
		if (tmp->data) {
			free(tmp->data);
			tmp->data = NULL;
		}
		tmp1 = tmp->next;
		if (tmp != NULL) {
			free(tmp);
			tmp = NULL;
		}
	}
}

static void *tp_new_node(frame_head_t *tmp_head, tp_playlistinst_t *playinst)
{
	//LOGI("new video frameList!");
    Hi3510_frame_t *new = (Hi3510_frame_t *) malloc(sizeof(Hi3510_frame_t));
    if (new == NULL) {
        //LOGE("creat new AudioBuffer error!");
        return NULL;
    }
    memcpy(new, tmp_head, sizeof(frame_head_t));

    new->data = (unsigned char *) malloc((sizeof(char)) * tmp_head->frame_size);
    if (new->data == NULL) {
        //LOGE("new->data == null");
        goto error;
    }
    memcpy(new->data, playinst->pframe + sizeof(frame_head_t),
    		(sizeof(char)) * tmp_head->frame_size);

    new->next = NULL;
    return new;

error:
    if (new != NULL) {
        free(new);
        new = NULL;
    }

    return NULL;
}

static void *tp_add_node(frame_head_t *tmp_head, char* tmp_frame, Hi3510_frame_t *Hframe)
{
    int i = 0;
    while (Hframe->next != NULL) {
    	Hframe = Hframe->next;
        i++;
    }
    if (i >= MAX_FRAME_LIST_NUM) {
        //LOGE("Audio_FrameCount over %d!!!", i);
        return NULL;
    }
    Hi3510_frame_t *new = (Hi3510_frame_t *) malloc(sizeof(Hi3510_frame_t));
    if (new == NULL) {
        //LOGE("creat new AudioBuffer error!");
        return NULL;
    }
    memcpy(new, (char*)tmp_head, sizeof(frame_head_t));

    new->data = (unsigned char *) malloc((sizeof(char)) * tmp_head->frame_size);
    if (new->data == NULL) {
        //LOGE("new->data == null");
        goto error;
    }
    memcpy(new->data, (char*)(tmp_frame + sizeof(frame_head_t)),
    		(sizeof(char)) * tmp_head->frame_size);

    Hframe->next = new;
    new->next = NULL;
    return NULL;

error:
    if (new != NULL) {
        free(new);
        new = NULL;
    }

    return NULL;
}

int st_initFrameList(tp_playlistinst_t *playinst)
{
	if (playinst == NULL)
		return -1;

	playinst->VframeList = NULL;
	playinst->AframeList = NULL;
	pthread_mutex_init(&playinst->Video_mutex, NULL); //modify by qiaob
	pthread_mutex_init(&playinst->Audio_mutex, NULL); //add LinZh107
	playinst->pframe = malloc(FRAME_MAX_SIZE_T);
	if (!playinst->pframe)
		return -1;
	memset(playinst->pframe, 0, FRAME_MAX_SIZE_T);
	playinst->prev_Vframe_no = 2147483647;

	return 0;
}

int st_destroyFrameList(tp_playlistinst_t *playinst)
{
	//LOGI("%s", __FUNCTION__);
	if (!playinst)
		return -1;

	pthread_mutex_lock(&playinst->Video_mutex);
	pthread_mutex_unlock(&playinst->Video_mutex);
	pthread_mutex_destroy(&playinst->Video_mutex);
	pthread_mutex_destroy(&playinst->Audio_mutex); //add LinZh107

	freelist(playinst->VframeList);
	freelist(playinst->AframeList);

	if (playinst->pframe) {
		free(playinst->pframe);
		playinst->pframe = NULL;
	}
	return 0;
}

//get key frame
Hi3510_frame_t *st_getKeyFramefromList(tp_playlistinst_t *playinst)
{
	if (!playinst){
		LOGE("This playinst is empty.");
		return NULL;
	}

	pthread_mutex_lock(&playinst->Video_mutex);
	while (NULL != playinst->VframeList)
	{
		//frame_type: (1)i_frame, (2)P_frame,(3)audio_frame
		if(VIDEO_FRAME == playinst->VframeList->frame_type){
			pthread_mutex_unlock(&playinst->Video_mutex);
			return playinst->VframeList;
		}
		playinst->VframeList = playinst->VframeList->next;
	}

	pthread_mutex_unlock(&playinst->Video_mutex);
	return NULL;
}

//get frame list head
Hi3510_frame_t *st_getFrameListHead(tp_playlistinst_t *playinst, int flags)
{
	if (!playinst){
		LOGE("This playinst is empty.");
		return NULL;
	}

	switch (flags)
	{
	case VIDEO_FRAME:{
		pthread_mutex_lock(&playinst->Video_mutex);
		if (playinst->VframeList == NULL || playinst->VframeList->data == NULL) {
			pthread_mutex_unlock(&playinst->Video_mutex);
			return NULL;
		}
		pthread_mutex_unlock(&playinst->Video_mutex);

		if (playinst->VframeList->frame_no > playinst->prev_Vframe_no + LOST_FRAME_NUM) {
			while (NULL != playinst->VframeList
					&& (VIDEO_FRAME != playinst->VframeList->frame_type)){
				//frame_type: (1)i_frame, (2)P_frame,(3)audio_frame
				st_removeFrameListHead(playinst, VIDEO_FRAME);
			}
		}
		if (playinst->VframeList != NULL)
			playinst->prev_Vframe_no = playinst->VframeList->frame_no;
		return playinst->VframeList;
	}
	case AUDIO_FRAME:{
		pthread_mutex_lock(&playinst->Audio_mutex);
		if (playinst->AframeList == NULL || playinst->AframeList->data == NULL) {
			pthread_mutex_unlock(&playinst->Audio_mutex);
			return NULL;
		}
		pthread_mutex_unlock(&playinst->Audio_mutex);
		return playinst->AframeList;
	}
	default:
		break;
	}

	return NULL;
}

//LinZh107 flags frame_type  0:Video  1:Audio
void st_removeFrameListHead(tp_playlistinst_t *playinst, int flags)
{
	if (!playinst){
		LOGE("This playinst is empty.");
		return;
	}
	Hi3510_frame_t *tmp = NULL;
	switch (flags)
	{
	case VIDEO_FRAME:
		pthread_mutex_lock(&playinst->Video_mutex);
		tmp = playinst->VframeList;
		if (tmp != NULL){
			playinst->VframeList = playinst->VframeList->next;
			if (tmp->data != NULL) {
				free(tmp->data);
				tmp->data = NULL;
			}
			free(tmp);
			tmp = NULL;
		}
		pthread_mutex_unlock(&playinst->Video_mutex);
		break;
	case AUDIO_FRAME:
		pthread_mutex_lock(&playinst->Audio_mutex);
		tmp = playinst->AframeList;
		if (tmp != NULL)  {
			playinst->AframeList = playinst->AframeList->next;
			if (tmp->data){
				free(tmp->data);
				tmp->data = NULL;
			}
			free(tmp);
			tmp = NULL;
		}
		pthread_mutex_unlock(&playinst->Audio_mutex);
		break;
	}
	return;
}

//input one frame to list tail
int st_inputFrameListTail(tp_playlistinst_t *playinst, char *inputFrame, int dataSize)
{
	if (NULL == playinst || NULL == inputFrame || dataSize <= 32) {
		//LOGE("st_inputFrameListTail failed datasize=%d", dataSize);
		return -1;
	}

	memset(playinst->pframe, 0, FRAME_MAX_SIZE_T);
	memcpy(playinst->pframe, inputFrame, dataSize);

	frame_head_t *tmp_head = (frame_head_t *) playinst->pframe;
	//LOGI("tmp_frame->frame_size = %d", tmp_frame->frame_size);
	if (tmp_head->frame_type == AUDIO_FRAME) //audio frame
	{
		pthread_mutex_lock(&playinst->Audio_mutex);
		if (playinst->AframeList == NULL){		//new list
			//LOGI("new a audio frameList!");
			playinst->AframeList = tp_new_node(tmp_head, playinst);
		}
		else {									//add to list
			tp_add_node(tmp_head, playinst->pframe, playinst->AframeList);
			//LOGI("input audio frameListTail frame_size = %d", tmp_frame->frame_size);
		}
		pthread_mutex_unlock(&playinst->Audio_mutex);
	}
	else //video frame
	{
		pthread_mutex_lock(&playinst->Video_mutex);
		if (playinst->VframeList == NULL) {
			playinst->VframeList = tp_new_node(tmp_head, playinst);
			//LOGI("new a video frameList! frame_size = %d", tmp_head->frame_size);
		}
		else {
			tp_add_node(tmp_head, playinst->pframe, playinst->VframeList);
			//LOGI("input video frameListTail frame_size = %d", tmp_head->frame_size);
		}
		pthread_mutex_unlock(&playinst->Video_mutex);
	}

	return 0;
}

void st_setFrameListBroadcast(tp_playlistinst_t *playinst)
{
	if (!playinst)
		return;

	pthread_mutex_lock(&playinst->Video_mutex);
	pthread_mutex_unlock(&playinst->Video_mutex);
}

int getFrameNum(tp_playlistinst_t *playinst, int flags) //2014-5-28 modify LinZh107
{
	if (!playinst)
		return -1;

	int num = 0;
	Hi3510_frame_t *tmp = NULL;
	switch (flags)
	{
	case VIDEO_FRAME:
		tmp = playinst->VframeList;
		break;
	case AUDIO_FRAME:
		tmp = playinst->AframeList;
		break;
	}

	while (tmp) {
		tmp = tmp->next;
		num++;
	}
	return num;
}
