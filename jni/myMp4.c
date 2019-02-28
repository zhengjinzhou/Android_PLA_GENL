#include "myMp4.h"

#define  FRAMERATE      28
#define  SAMPLEDUR      320
#define  MP4WIDTH       1280
#define  MP4HEIGHT      720
#define  SAMPLEPERSEC   8000
#define  TIMESCALE      90000

unsigned char sps[10] = {0x67,0x4d,0x00,0x1f,0x95,0xa8,0x14,0x01,0x6e,0x40};
unsigned char pps[4]= {0x68,0xee,0x3c,0x80};

static bool isKeyFrame;
static Mp4Param_S g_mp4Param;
static unsigned int g_sampleId[2] = {0,0};   ///[0]->aSampleNum; [1]->vSampleNum;

void tp_mp4_setAVCurSampleID(int id)
{
    g_mp4Param.aSampleId = id;
    g_mp4Param.vSampleId = id;
}

void tp_mp4_setAVTotalSampleNum(int num)
{
    g_sampleId[0] = num;
    g_sampleId[1] = num;
}

int tp_mp4_getCurrentVideoID()
{
    return g_mp4Param.vSampleId-1;
}

int tp_mp4_getTotalVideoNum()
{
    return g_mp4Param.vSampleNum;
}

void tp_mp4_initMp4Param()
{
    g_mp4Param.aSampleNum = 0;
    g_mp4Param.vSampleNum = 0;
    g_mp4Param.isIframe = false;
    g_mp4Param.handle = NULL;
    g_mp4Param.aSampleId = 1;
    g_mp4Param.vSampleId = 1;
    g_mp4Param.aTrack = MP4_INVALID_TRACK_ID;
    g_mp4Param.vTrack = MP4_INVALID_TRACK_ID;
    isKeyFrame = false;
}

int tp_mp4_openMp4File(char *fileName, int flag)
{    
    tp_mp4_initMp4Param();

    switch(flag)
    {
        case MP4_CREATE:
            {
                g_mp4Param.handle = MP4Create(fileName);
                if (g_mp4Param.handle == MP4_INVALID_FILE_HANDLE)
                {
                    error_Printf("MP4Create fail ...");
                    return -1;
                }
                MP4SetTimeScale(g_mp4Param.handle, TIMESCALE);
            }
            break;
        case MP4_READ:
            {
                g_mp4Param.handle = MP4Read(fileName);
                if (g_mp4Param.handle == MP4_INVALID_FILE_HANDLE)
                {
                    error_Printf("MP4Read fail ...");
                    return -1;
                }
            }
            break;
        default:
            g_mp4Param.handle = MP4_INVALID_FILE_HANDLE;
            break;
    }

    return 0;
}

//------------------------------------------------------------------------------------------------- Mp4Encode说明
// 【h264编码出的NALU规律】
// 第一帧 SPS【0 0 0 1 0x67】 PPS【0 0 0 1 0x68】 SEI【0 0 1 0x6】 IDR【0 0 1 0x65】
// p帧      P【0 0 0 1 0x61】
// I帧    SPS【0 0 0 1 0x67】 PPS【0 0 0 1 0x68】 IDR【0 0 1 0x65】
// 【mp4v2封装函数MP4WriteSample】
//-------------------------------------------------------------------------------------------------

int tp_mp4_writeMp4VideoFrame(unsigned char *pData, int nSize)
{
    if (g_mp4Param.handle == MP4_INVALID_FILE_HANDLE)
    {
        error_Printf("g_mp4Param.handle is MP4_INVALID_FILE_HANDLE ...");
        return -1;
    }

    if(pData[0] == 0 && pData[1] == 0 && pData[2] == 0 && pData[3] == 1 && pData[4] == 0x67)
    {
        isKeyFrame = true;
        g_mp4Param.isIframe = true;
    }
    else
    {
        g_mp4Param.isIframe = false;
    }
    if(!isKeyFrame)
    {
        error_Printf("the first frame is not I frame ...");
        return -1;
    }

    if(g_mp4Param.vTrack == MP4_INVALID_TRACK_ID)
    {
        g_mp4Param.vTrack = MP4AddH264VideoTrack(g_mp4Param.handle,
                                                        90000,
                                                        90000 / 28,
                                                        MP4WIDTH,     // width
                                                        MP4HEIGHT,    // height
                                                        sps[1], // sps[1] AVCProfileIndication
                                                        sps[2], // sps[2] profile_compat
                                                        sps[3], // sps[3] AVCLevelIndication
                                                        3);
        if (g_mp4Param.vTrack == MP4_INVALID_TRACK_ID)
        {
            error_Printf("MP4AddH264VideoTrack fail ...");
            return -1;
        }

        MP4SetVideoProfileLevel(g_mp4Param.handle, 0x7f);
        MP4AddH264SequenceParameterSet(g_mp4Param.handle,g_mp4Param.vTrack,sps,strlen((const char*)sps));
        MP4AddH264PictureParameterSet(g_mp4Param.handle,g_mp4Param.vTrack,pps,strlen((const char*)pps));
    }

    if(!MP4WriteSample(g_mp4Param.handle, g_mp4Param.vTrack, pData, nSize, MP4_INVALID_DURATION, 0, g_mp4Param.isIframe))
    {
        error_Printf("MP4WriteVideoSample fail ...\n\n");
        return -1;
    }

    return 0;
}

int tp_mp4_writeMp4AudioFrame(unsigned char *pData,int nSize)
{
    if (g_mp4Param.handle == MP4_INVALID_FILE_HANDLE)
    {
        error_Printf("g_mp4Param.handle is MP4_INVALID_FILE_HANDLE ...");
        return -1;
    }
    if(g_mp4Param.aTrack == MP4_INVALID_TRACK_ID)
    {
        g_mp4Param.aTrack = MP4AddULawAudioTrack(g_mp4Param.handle, SAMPLEPERSEC, SAMPLEDUR);
        if (g_mp4Param.aTrack == MP4_INVALID_TRACK_ID)
        {
            error_Printf("MP4AddAudioTrack fail ...\n\n");
            return -1;
        }

        MP4SetTrackIntegerProperty(g_mp4Param.handle,
                                g_mp4Param.aTrack,
                                "mdia.minf.stbl.stsd.ulaw.channels",
                                 1);
        MP4SetAudioProfileLevel(g_mp4Param.handle, 0x2);
    }

    int	pkt_cnt;
    audio_pk_t *a_pkt = NULL;

    memcpy(&pkt_cnt, pData, sizeof(int));
    a_pkt = (audio_pk_t *)((char*)pData + sizeof(int));

    while(pkt_cnt-- > 0)
    {
        if(!MP4WriteSample(g_mp4Param.handle, g_mp4Param.aTrack, a_pkt->data, a_pkt->pkt_len-4, MP4_INVALID_DURATION, 0, 1))
        {
            error_Printf("MP4WriteAudioSample fail ...\n\n");
            return -1;
        }
        a_pkt += a_pkt->pkt_len + sizeof(int);
    }

    return 0;
}

int tp_mp4_readMp4VideoFrame(unsigned char *pData, int *nSize , int flag)
{    
    if (g_mp4Param.handle == MP4_INVALID_FILE_HANDLE)
    {
        error_Printf("g_mp4Param.handle is MP4_INVALID_FILE_HANDLE ...");
        return -1;
    }
    if(g_mp4Param.vTrack == MP4_INVALID_TRACK_ID)
    {
        g_mp4Param.vTrack = MP4FindTrackId(g_mp4Param.handle, 0, MP4_VIDEO_TRACK_TYPE, 0);
        if (g_mp4Param.vTrack == MP4_INVALID_TRACK_ID)
        {
            error_Printf("MP4AddVideoTrack fail ...");
            return -1;
        }

        g_mp4Param.vSampleNum = MP4GetTrackNumberOfSamples(g_mp4Param.handle, g_mp4Param.vTrack);
        if(1 == flag)
        {
            if(g_sampleId[1] <= g_mp4Param.vSampleNum)
            {
                g_mp4Param.vSampleNum = g_sampleId[1];
            }
        }
    }

    if(g_mp4Param.vSampleId > g_mp4Param.vSampleNum)
    {
        infor_Printf("MP4ReadSample video complete ...");
        return 2;
    }
    if(!MP4ReadSample(g_mp4Param.handle, g_mp4Param.vTrack, g_mp4Param.vSampleId, &pData, (uint32_t*)nSize))
    {
        error_Printf("MP4ReadSample fail ...\n\n");
        return -1;
    }
    g_mp4Param.vSampleId++;

    return 0;
}

int tp_mp4_readMp4AudioFrame(unsigned char *pData, int *nSize, int flag)
{
    if (g_mp4Param.handle == MP4_INVALID_FILE_HANDLE)
    {
        error_Printf("g_mp4Param.handle is MP4_INVALID_FILE_HANDLE ...");
        return -1;
    }
    if(g_mp4Param.aTrack == MP4_INVALID_TRACK_ID)
    {
        g_mp4Param.aTrack = MP4FindTrackId(g_mp4Param.handle, 0, MP4_AUDIO_TRACK_TYPE, 0);
        if (g_mp4Param.aTrack == MP4_INVALID_TRACK_ID)
        {
            error_Printf("MP4FindTrackId fail ...\n\n");
            return -1;
        }
        g_mp4Param.aSampleNum = MP4GetTrackNumberOfSamples(g_mp4Param.handle, g_mp4Param.aTrack);
        if(1 == flag)
        {
            if(g_sampleId[1] <= g_mp4Param.aSampleNum)
            {
                g_mp4Param.aSampleNum = g_sampleId[1];
            }
        }
    }

    if(g_mp4Param.aSampleId > g_mp4Param.aSampleNum)
    {
        infor_Printf("MP4ReadSample audio complete ...");
        return 2;
    }

    if(!MP4ReadSample(g_mp4Param.handle, g_mp4Param.aTrack, g_mp4Param.aSampleId, &pData, (uint32_t*)nSize))
    {
        error_Printf("MP4ReadSample fail ...\n\n");
        return -1;
    }

    g_mp4Param.aSampleId++;

    return 0;
}

int tp_mp4_closeMp4File()
{
    if(g_mp4Param.handle)
    {
        MP4Close(g_mp4Param.handle);
        g_mp4Param.handle = NULL;
    }

    return 0;
}

