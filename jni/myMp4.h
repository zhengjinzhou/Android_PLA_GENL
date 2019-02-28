#ifndef MYMP4_H
#define MYMP4_H

extern "C"
{
#include "commLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}
#include "mp4v2/mp4v2.h"

typedef struct __audio_pk_t
{
    int	    pkt_len;
    char	hisi[4];
    unsigned char	data[0];
}audio_pk_t;

typedef enum
{
    MP4_READ = 0,
    MP4_CREATE = 1
}MP4FILEFLAG;

typedef struct __MP4PARAM_S_
{
    uint32_t aSampleNum;
    uint32_t vSampleNum;
    bool          isIframe;
    MP4TrackId    aTrack;
    MP4TrackId    vTrack;
    MP4SampleId   aSampleId;
    MP4SampleId   vSampleId;
    MP4FileHandle handle;
}Mp4Param_S;

int tp_mp4_getTotalVideoNum();
int tp_mp4_getCurrentVideoID();
void tp_mp4_setAVCurSampleID(int id);
void tp_mp4_setAVTotalSampleNum(int num);

int tp_mp4_openMp4File(char *fileName, int flag);
int tp_mp4_writeMp4VideoFrame(unsigned char *pData, int nSize);
int tp_mp4_writeMp4AudioFrame(unsigned char *pData, int nSize);

/******************flag: 0->sampleId = 0; 1->sampleId != 0*********/
int tp_mp4_readMp4VideoFrame(unsigned char *pData, int *nSize, int flag);
int tp_mp4_readMp4AudioFrame(unsigned char *pData, int *nSize, int flag);
int tp_mp4_closeMp4File();

#endif // MYMP4_H
