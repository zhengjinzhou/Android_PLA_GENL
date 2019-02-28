/*
 * netglobal.h
 *
 *  Created on: 2009-10-13
 *      Author: pc
 */

#ifndef NETGLOBAL_H_
#define NETGLOBAL_H_

#include "netdatatype.h"
#include "list.h"
#include <pthread.h>
#include "st_cuapi.h"
#include "clientlink.h"
#include "VideoInstance.h"
#include "AudioInstance.h"

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

#define MDU_KEEP_CIRCUIT  15000
#define CMU_KEEP_CIRCUIT  15000

typedef struct tag_bib_instance_para
{	
	clientlink          gClientLink;
	int					hMsgWnd;
	unsigned int        nNetLibMsg;
	long long           CmdTimeOut;

	int 				msg_list_num;
	PACK_ELEMENT_list	ResponseList;
	pthread_mutex_t		lockResponseList;
	int					Responselistcount;

	P_AUDIOINSTANCE_T	pAudioInstance;
	int  				isExit;
}LIB_INSTANCE_PARA, *P_LIB_INSTANCE_PARA;

//----------------------------------------------------------------------
// 全局功能函数定义
int init_my_log(char *filename);
//void my_log(const char *format, ...);
int utf8_to_unicode(uint8_t *in, uint16_t *out, int *outsize);
int  CreateSeqByTime(void);
char* CreateCallID(char* callID);
int  writelog(const char* log);
int WaitForResponse(int pdwServerPara, int nSeqNum, const char* cmd,
		PACK_ELEMENT* element, long long dwWaitTime);//modified by zhusy

int  CreatePack_CMU_Heart(void* lpBuf, int nSeqNum);
int  CreatePack_LoginReq(void* lpBuf, int nSeqNum);
int  CreatePack_CMU_Logout(void* lpBuf, int nSeqNum);
int  CreatePack_DevceReq(void* lpBuf, int nSeqNum, const char* szDomainID, const char* szAreaID);
int  CreatePack_CMU_RealStartReq(void* lpBuf, int nSeqNum,  const char* szDomanId,
		const char* szPuId, const char* szGuId);
int  CreatePack_CMU_RealStopReq(void* lpBuf, int  nSeqNum, const char* szPuId);
int  CreatePack_CMU_DomeCtrlQuery(void* lpBuf, int nSeqNum, const char* szGuID,
		const char* szDomainid);
int  CreatePack_CMU_MsuRecord(void* lpBuf, int nSeqNum, const char* szPuid, const char* szGuid,int nTime,
		const char* szMsuName, const char* szMsuIp, const char* szMsuID, const char* szDomainId);
int  CreatePack_CMU_VodServerQuery(void* lpBuf, int nSeqNum, const char* szGuid, const char* szDomainid);

// 中心录像策略
int  CreatePack_CMU_MsuRecordInfo(void* lpBuf, int nSeqNum, const char* szPuid, const char* szGuid, const char* szMsuid, const char* szDomainId);

int CreatePack_MDU_RealStartReq(void* lpBuf, int nSeqNum, char* callId, GUINFO*guInfo, const int nProtocolType, MDU_ROUTE_INFO mduInfo);
int CreatePack_MDU_Heart(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo );
int CreatePack_MDU_RealStartACK(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo);
int CreatePack_MDU_Bye(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo);
int CreatePack_Voice_Bye(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo);
int CreatePack_MDU_VoiceChart(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo, const int nProtocolType, MDU_ROUTE_INFO mduInfo);
int IsElementTimeOut(const PACK_ELEMENT* Element);

// 手动解析HTTP包中的XML文本, 针对CMU的响应
char* GetMiddleString(char* strSrc, const char* strStart, const char* strEnd, char* strOut, int nMaxLen);
int GetRealMDURoute(char* szXML, MDU_ROUTE_INFO* mduRoute);
int GetMDURoute(char* szXML, char * lpszMduIp, int* nPort);//LPTSTR->char*
int GetHeadLine(char* szBuf, char* szHead, int nMax);
void  GetVideoIdFromGuid(char* lpszGuid, int* nVideoId, int* nGuType);//LPCTSTR->char *
//----------------------------------------------------------------------
//POSITION  GetVideoInsPos(int pdwServerPara, void* lHandle);
// 时间段解析
//int  GetTimeSegFromString(const char* szTimeSeg, RECORD_TIMESEG* pTimeSeg);
//int  FormatTimeSegString(RECORD_TIMESEG timeSeg, char* szTimeSeg, int nMax);

void ConvertUtf8ToGBK(char *strUtf8);//
void ConvertGBKToUtf8(char *strGBK);//LPCSTR
int  UTF8ToGBK(char * lpUTF8Str,char * lpGBKStr,int nGBKStrLen);
//////////////////////////////////////////////////////////////////////////
int Recv(int fd, char *vptr, int n,int flag);
int Send(int fd, char *vptr, int n,int flag);


#  ifdef __cplusplus
}
#  endif /* __cplusplus */
//void LoadStringFromID(HINSTANCE hHandle, char* pszID, char* pszContent, int len);
#endif /* NETGLOBAL_H_ */
