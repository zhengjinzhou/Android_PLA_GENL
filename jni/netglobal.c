/*
 * netglobal.c
 *
 *  Created on: 2009-10-13
 *      Author: pc
 */

#include "netglobal.h"
#include <sys/time.h>
#include <stdlib.h>
//#include <iconv.h>
#include "st_cuapi.h"
#include "list.h"
#include <stdarg.h>
#include "SipAPI.h"
#include <android/log.h>

#define null 0
#define  LOG_TAG    "netglobal.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

CST_CULibApp theApp;

int unicode_to_utf8(uint16_t *in, int insize, uint8_t *out)
{
	int i = 0;
	int outsize = 0;
	int charscount = 0;
	// uint8_t *result = NULL;
	uint8_t *tmp = NULL;

	charscount = insize / sizeof(uint16_t);
	// result = (uint8_t *)malloc(charscount * 3 + 1);
	// memset(result, 0, charscount * 3 + 1);
	tmp = out;

	for (i = 0; i < charscount; i++)
	{
		uint16_t unicode = in[i];

		if (unicode >= 0x0000 && unicode <= 0x007f)
		{
			*tmp = (uint8_t) unicode;
			tmp += 2;
			outsize += 1;
		}
		else if (unicode >= 0x0080 && unicode <= 0x07ff)
		{
			*tmp = 0xc0 | (unicode >> 6);
			tmp += 1;
			*tmp = 0x80 | (unicode & (0xff >> 2));
			tmp += 1;
			outsize += 2;
		}
		else if (unicode >= 0x0800 && unicode <= 0xffff)
		{
			*tmp = 0xe0 | (unicode >> 12);
			tmp += 1;
			*tmp = 0x80 | (unicode >> 6 & 0x00ff);
			tmp += 1;
			*tmp = 0x80 | (unicode & (0xff >> 2));
			tmp += 1;
			outsize += 3;
		}

	}

	*tmp = '\0';

	return 0;
}

int utf8_to_unicode(uint8_t *in, uint16_t *out, int *outsize)
{
	uint8_t *p = in;
	int resultsize = 0;
	uint8_t *tmp = NULL;

	tmp = (uint8_t *) out;
	while (*p)
	{
		if (*p >= 0x00 && *p <= 0x7f)
		{
			*tmp = *p;
			tmp++;
			*tmp = '\0';
			tmp++;
			resultsize += 2;
		}
		else if ((*p & (0xff << 5)) == 0xc0)
		{
			uint16_t t = 0;
			uint8_t t1 = 0;
			uint8_t t2 = 0;

			t1 = *p & (0xff >> 3);
			p++;
			t2 = *p & (0xff >> 2);

			*tmp = t2 | ((t1 & (0xff >> 6)) << 6);   //t1 >> 2;
			tmp++;

			*tmp = t1 >> 2;   //t2 | ((t1 & (0xff >> 6)) << 6);
			tmp++;

			resultsize += 2;
		}
		else if ((*p & (0xff << 4)) == 0xe0)
		{
			uint16_t t = 0;
			uint8_t t1 = 0;
			uint8_t t2 = 0;
			uint8_t t3 = 0;

			t1 = *p & (0xff >> 3);
			p++;
			t2 = *p & (0xff >> 2);
			p++;
			t3 = *p & (0xff >> 2);

			//Little Endian
			*tmp = ((t2 & (0xff >> 6)) << 6) | t3;   //(t1 << 4) | (t2 >> 2);
			tmp++;

			*tmp = (t1 << 4) | (t2 >> 2);   //((t2 & (0xff >> 6)) << 6) | t3;
			tmp++;
			resultsize += 2;
		}

		p++;
	}

	*tmp = '\0';
	tmp++;
	*tmp = '\0';
	resultsize += 2;

	//*out = result;
	*outsize = resultsize;
	return 0;
}
#if 0
void dump_utf8(uint8_t *utf8)
{
	uint8_t *p = utf8;

	while(*p)
	{
		printf("%02X", *p);
		p++;
	}
	putchar('\n');
}

void dump_unicode(uint16_t *utf16, int size)
{
	uint8_t *p = (uint8_t *)utf16;
	int i = 0;

	for (i = 0; i < size; i++)
	{
		printf("%02X", *p);
		p++;
	}
	putchar('\n');
}

#endif

int CreateSeqByTime(void)
{
	int nSeqNum;
	char szTime[32];
	struct timeval tv;
	//time_t t1;
	struct tm *curtm;

	//t1 = time(NULL);
	gettimeofday(&tv, NULL);
	curtm = localtime((const time_t*) &tv.tv_sec);

	sprintf(szTime, "1%02d%02d%02d%03d", curtm->tm_hour, curtm->tm_min, curtm->tm_sec, tv.tv_usec / 1000);
	nSeqNum = atoi(szTime);

	return nSeqNum;
}

// 外部提供缓冲区,大小不小于32字节
char* CreateCallID(char* callID)
{
	char m_UserId[32];
	struct timeval tv;
	char szTime[32];
	struct tm *curtm;
	static int flag = 1, count;

	if (flag)
	{
		srand(time(NULL));
		count = rand();

		flag = 0;
	}
	memset(m_UserId, 0, 32);
	strcpy(m_UserId, theApp.m_UserID);
	//ConvertGBKToUtf8(m_UserId); //add LinZh107 2014-9-19
	
	gettimeofday(&tv, NULL);
	curtm = localtime((const time_t*) &tv.tv_sec);

	count++;
	sprintf(szTime, "%02d%02d%02d%03d", curtm->tm_hour, curtm->tm_min, curtm->tm_sec, tv.tv_usec / 1000);
	sprintf(callID, "CU%08d%s%s", count, m_UserId/*theApp.m_UserID*/, szTime);

	//	sprintf(callID, "CU%08d%s%s",  count, theApp.m_UserID.Left(12), szTime);

	return callID;
}

//----------------------------------------------------------------------
// 函数功能:检查消息队列缓冲区,在规定时间里取得消息,否则超时退出,返回错误代码
//----------------------------------------------------------------------
int inline IsElementTimeOut(const PACK_ELEMENT *Element)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	if ((((long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000) - Element->dwTime) < 3000) // 15000 2013-10-30
		return 0;
	else
		return 1;
}

//----------------------------------------------------------------------
// create by zhusy, modified by LinZh107
// 函数功能:检查消息队列缓冲区,在规定时间里取得消息;
int WaitForResponse(int pdwServerPara, int nSeqNum, const char* cmd, PACK_ELEMENT *element,
		long long dwWaitTime)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pdwServerPara;
	PACK_ELEMENT_list *cur_node = NULL, *curn_node = NULL;
	struct timeval tv;
	long long dwStart;

	pthread_mutex_lock(&pPara->lockResponseList);

	/************** 删除超时的节点元素  *****************/
	//list_for_each_entry_safe(cur_node, curn_node, &pPara->pack_list.list, list, PACK_ELEMENT_list)
	cur_node = list_entry((&pPara->ResponseList.list)->next, PACK_ELEMENT_list, list);
	curn_node = list_entry(cur_node->list.next, PACK_ELEMENT_list, list);
	while (1)
	{
		PACK_ELEMENT* pElement = NULL;
		if (&cur_node->list == (&pPara->ResponseList.list))
			break;
		pElement = &(cur_node->pack_element);
		if (IsElementTimeOut(pElement))
		{
			list_del(&cur_node->list);
			free(cur_node);
			cur_node = NULL;
			pPara->msg_list_num--;
		}
		cur_node = curn_node;		//记录此次循环的POSITION
		curn_node = list_entry(cur_node->list.next, PACK_ELEMENT_list, list);
	}

	pthread_mutex_unlock(&pPara->lockResponseList);

	gettimeofday(&tv, NULL);
	dwStart = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;

	while (1)
	{
		usleep(5*1000);// 释放一些CPU时间
		struct timeval tv1;
		gettimeofday(&tv1, NULL);
		if ((((long long) tv1.tv_sec * 1000 + (long long) tv1.tv_usec / 1000) - dwStart) > dwWaitTime)  // 判断超时
		{
			LOGE("WaitForResponse timeOut. dwWaitTime:%ld", dwWaitTime);
			return -1;
		}
        pthread_mutex_lock(&pPara->lockResponseList);
		if (list_empty(&pPara->ResponseList.list))
		{
            pthread_mutex_unlock(&pPara->lockResponseList);
			continue;
		}

		//list_for_each_entry_safe(cur_node, curn_node, &pPara->pack_list.list, list, PACK_ELEMENT_list)
		cur_node = list_entry((&pPara->ResponseList.list)->next, PACK_ELEMENT_list, list);
		curn_node = list_entry(cur_node->list.next, PACK_ELEMENT_list, list);
		while (1)
		{
			PACK_ELEMENT* pElement = NULL; //&cur_node->pack_element;
			// 检查回应与请求是否匹配,将匹配的项拷贝出来,并删除列表中的项

			if (NULL != cur_node) // 2013-10-30
				pElement = &cur_node->pack_element;
			if (0 == strcmp(cmd, pElement->httpObject.cmd))
			{
				if (pElement->httpObject.type == RESP_TYPE)
				{
					if (nSeqNum == pElement->httpObject.seq)
					{
						//element = *pElement;    // 结构体赋值，不是指针赋值
						memcpy(element, pElement, sizeof(PACK_ELEMENT));
						list_del(&cur_node->list);
						free(cur_node);           // 释放缓冲区
						cur_node = NULL;          // 释放缓冲区
						pthread_mutex_unlock(&pPara->lockResponseList);
						return 0;
					}
				}
				else{
					//element = *pElement;    // 结构体赋值，不是指针赋值
					memcpy(element, pElement, sizeof(PACK_ELEMENT));
					list_del(&cur_node->list);
					free(cur_node);             // 释放缓冲区
					cur_node = NULL;
					pthread_mutex_unlock(&pPara->lockResponseList);
					return 0;
				}
			}
			// 2013-10-30
			if (&cur_node->list == (&pPara->ResponseList.list))
			{
				//LOGE("no response! cur_node->list = glistResponseCount:%d", glistResponseCount);
				break;
			}
			cur_node = curn_node;
			curn_node = list_entry(curn_node->list.next, PACK_ELEMENT_list, list);
		}//end while(1)

		pthread_mutex_unlock(&pPara->lockResponseList);
	}
	return -2;
}

//----------------------------------------------------------
// CMU相关数据包的创建
//----------------------------------------------------------
int CreatePack_CMU_Heart(void* lpBuf, int nSeqNum)
{
	//CString strUserID = theApp.m_UserID;
	char bufParam[128];
	int nLength = 0;
	
	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19
	
	sprintf(bufParam, "User=%s", pchar);

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, CMU_KEEPALIVE, bufParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24
	return nLength;

}

int CreatePack_CMU_Logout(void* lpBuf, int nSeqNum)
{
	char bufParam[128];
	int nLength = 0;
	
	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19
	
	sprintf(bufParam, "User=%s", pchar);

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, CMU_USERLOGOUT, bufParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24

	return nLength;

}

int CreatePack_LoginReq(void* lpBuf, int nSeqNum)
{
	//deleted by zhusy
	
	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19
	
	char bufParam[256];
	int nLength = 0;
	if (strcmp(theApp.m_strCheckCode, "") != 0)
		sprintf(bufParam, "User=%s&Password=%s&CheckCode=%s", pchar, theApp.m_UserPwd, theApp.m_strCheckCode);
	else
		sprintf(bufParam, "User=%s&Password=%s", pchar, theApp.m_UserPwd);

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, USER_LOGIN_REQ, bufParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24
	return nLength;
}

int CreatePack_DevceReq(void* lpBuf, int nSeqNum, const char* szDomainID, const char* szAreaID)
{
	//CString strUserID = theApp.m_UserID;
	char bufParam[256];
	int nLength = 0;
	char strUserID[32];

	memset(bufParam, 0, sizeof(bufParam));
	//CString strAreaID ;
	//strAreaID.Format("%s", szAreaID);

	//char strAreaID[64];
	//sprintf(strAreaID,"%s",szAreaID);

	sprintf(strUserID, "%s", theApp.m_UserID);
	//ConvertGBKToUtf8(strUserID); //add LinZh107 2014-9-19
	
	//if (strcmp(szAreaID,"0") == 0 ||strcmp(szAreaID," ") == 0)
	//if (strcmp(strAreaID," ") == 0 )
	//if( strAreaID== "0" || strAreaID == "")
	if (strlen(szAreaID) == 0 || strcmp(szAreaID, "0") == 0)
	{
		sprintf(bufParam, "User=%s&DomainID=%s", strUserID, szDomainID);
	}
	else
	{
		sprintf(bufParam, "User=%s&AreaID=%s", strUserID, szAreaID);
	}

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, DEVICELIST_REQ, bufParam, NULL);// HTTP_LIB:: zhusy modified 2009-06-24
	//ASSERT(nLength > 0);
	return nLength;

}

// 云台控制查询(请求)权限
int CreatePack_CMU_DomeCtrlQuery(void* lpBuf, int nSeqNum, const char* szGuID, const char* szDomainid)
{
	char bufParam[256];
	int nLength = 0;

	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19

	memset(bufParam, 0, sizeof(bufParam));
	sprintf(bufParam, "User=%s&GUID=%s&DomainID=%s", pchar, szGuID, szDomainid);

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, CMU_DOMECTRLQUERY, bufParam, NULL);// HTTP_LIB:: zhusy modified 2009-06-24
	//ASSERT(nLength > 0);
	return nLength;
}

// 向CMU请求视频连接,得到MDU信息
int CreatePack_CMU_RealStartReq(void* lpBuf, int nSeqNum, const char* szDomanId, const char* szPuId, const char* szGuId)
{
	char bufParam[256];
	int nLength = 0;
	
	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19

	memset(bufParam, 0, sizeof(bufParam));
	sprintf(bufParam, "User=%s&PUID=%s&GUID=%s&CMUIP=%s&DomainID=%s", pchar, szPuId, szGuId,
			theApp.m_ServerIP, szDomanId);

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, CMU_REAL_START_REQ, bufParam, 0);// HTTP_LIB:: zhusy modified 2009-06-24
//	ASSERT(nLength > 0);
	return nLength;
}

int CreatePack_CMU_MsuRecord(void* lpBuf, int nSeqNum, const char* szPuid, const char* szGuid, int nTime,
		const char* szMsuName, const char* szMsuIp, const char* szMsuID, const char* szDomainId)
{
	char bufParam[256];
	int nLength = 0;
	memset(bufParam, 0, sizeof(bufParam));
	sprintf(bufParam, "User=%s&PUID=%s&GUID=%s&TimeLength=%d&MsuName=%s&MsuIp=%s&MSUID=%s&DomainID=%s", theApp.m_UserID,
			szPuid, szGuid, nTime, szMsuName, szMsuIp, szMsuID, szDomainId);

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, "StartManualRecordReq", bufParam, NULL);// HTTP_LIB:: zhusy modified 2009-06-24
	//ASSERT(nLength > 0);
	return nLength;
}

int CreatePack_CMU_VodServerQuery(void* lpBuf, int nSeqNum, const char* szGuid, const char* szDomainid)
{
	char bufParam[256];
	int nLength = 0;
	
	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19

	memset(bufParam, 0, sizeof(bufParam));
	sprintf(bufParam, "User=%s&GUID=%s&CMUIP=%s&DomainID=%s", pchar, szGuid, theApp.m_ServerIP, szDomainid);

	st_http_makeHttp("GET", nSeqNum, (char*) lpBuf, &nLength, "VodServerQueryReq", bufParam, NULL);	// HTTP_LIB:: zhusy modified 2009-06-24
	//ASSERT(nLength > 0);
	return nLength;
}

//-----------------------------------------------------------------------------------------
// MDU相关数据包的创建
//-------------------------------------------------		----------------------------------------
// 创建MDU实时视频请求数据包
int CreatePack_MDU_RealStartReq(void* lpBuf, int nSeqNum, char* callId, GUINFO *guInfo,
		const int nProtocolType, MDU_ROUTE_INFO mduInfo)
{
	char cuid[32] = { 0 };
	char MduIp[5][32] = { 0 };
	char sipCmd[512] = { 0 };
	char sipMsg[1024] = { 0 };
	char xmlBuf[768] = { 0 };
	int nReturn;

	char cmd[128], head[128], to[128], from[128], cseq[128], callid[128];
	sprintf(cuid, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(cuid);
	
	strncpy(MduIp[0], "127.0.0.1", sizeof(MduIp[0]) - 1);

//------------------------------------------------------------------------------
	if (guInfo->DomainType == 0)
	{
		sprintf(xmlBuf,
				"<?xml version=\"1.0\"?>\
<message command=\"%s\">\
<domainID>%s</domainID>\
<puId>%s</puId>\
<guId>%s</guId>\
<cuId>%s</cuId>\
<mduId>%s</mduId>\
<mduIp>%s</mduIp>\
<mduPort>%d</mduPort>\
<domainUsername></domainUsername>\
<domainPassword></domainPassword>\
<videoId>1</videoId>\
<mediaType>3</mediaType>\
<streamType>0</streamType>\
<IPprotoType>%d</IPprotoType>\
</message>\r\n",
				MDU_REAL_START_REQ, guInfo->DomainID, guInfo->PUID, guInfo->GUID, cuid, mduInfo.szLocalMduId,
				mduInfo.szLocalMduIp, mduInfo.uLocalPort, nProtocolType);
	}
	else
	{
		sprintf(xmlBuf,
				"<?xml version=\"1.0\"?>\
<message command=\"%s\">\
<domainID>%s</domainID>\
<puId>%s</puId>\
<guId>%s</guId>\
<cuId>%s</cuId>\
<mduId>%s</mduId>\
<mduIp>%s</mduIp>\
<mduPort>%d</mduPort>\
<domainUsername></domainUsername>\
<domainPassword></domainPassword>\
<videoId>1</videoId>\
<mediaType>3</mediaType>\
<streamType>0</streamType>\
<IPprotoType>%d</IPprotoType>\
</message>\r\n",
				MDU_REAL_START_REQ, guInfo->DomainID, guInfo->PUID, guInfo->GUID, cuid,
				mduInfo.szRemoteMduId, mduInfo.szRemoteMduIp, mduInfo.uRemoteMduPort, nProtocolType);
	}
//------------------------------------------------------------------------------

	sprintf(cmd, "INVITE");
	sprintf(head, "sip:127.0.0.1");
	sprintf(to, "<sip:%s:%d>", MduIp, 0);
	sprintf(from, "%s<sip:cuip:cu_port>", cuid);
	sprintf(cseq, "%d %s", nSeqNum, cmd);
	sprintf(callid, "%s", callId);

#if 0
	// 对字符串的编码进行转换, 转成UTF-8 (CP_ACP;GB2312-20936;GB18030-54936)
	int temp;
	WCHAR wideBuf[1024];
	//modified by zhusy (ENCODING_IN->CP_ACP,ENCODING_IN not supported!)
	temp = ::MultiByteToWideChar( CP_ACP/*ENCODING_IN*/, 0, xmlBuf, strlen(xmlBuf)+1, wideBuf, sizeof(wideBuf)/sizeof(WCHAR));
	temp = ::WideCharToMultiByte( ENCODING_OUT, 0, wideBuf, wcslen(wideBuf)+1, xmlBuf, sizeof(xmlBuf)/sizeof(char), NULL, NULL);
#endif

	st_sip_setSip(sipCmd, cmd, head, to, from, cseq, callid, "text/xml", strlen(xmlBuf));
	st_sip_makeMsg(sipMsg, sipCmd, xmlBuf);	//tmpbuf
	////my_log("st_sip_makeMsg  %s\n", sipMsg);
	// 为了避免不必要的出错,加上结束符号,返回长度并不包括
	memcpy((char*) lpBuf, sipMsg, strlen(sipMsg) + 1);
	nReturn = strlen(sipMsg);

	return nReturn;
}

// 创建与MDU实时视频请求后的心跳数据包
int CreatePack_MDU_Heart(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo)
{
	char cuid[32] = { 0 };
	char MduIp[5][32] = { 0 };
	char sipCmd[1024] = { 0 };
//	char sipMsg[2048] = {0};
	char cmd[128], head[128], to[128], from[128], cseq[128], callid[128];
	//int nReturn;

	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19

	sprintf(cuid, "%s", pchar);
	strncpy(MduIp[0], "127.0.0.1", sizeof(MduIp[0]) - 1);
	sprintf(cmd, "OPTIONS");
	sprintf(head, "sip:127.0.0.1");
	sprintf(to, "<sip:%s:%d>", MduIp, 0);
	sprintf(from, "%s <sip:cuip:cu_port>", cuid);
	sprintf(cseq, "%d %s", nSeqNum, cmd);
	sprintf(callid, "%s", callId);

	st_sip_setSip(sipCmd, cmd, head, to, from, cseq, callid, "text/xml", 0);
	//st_sip_makeMsg(sipMsg,sipCmd, NULL);

	st_sip_makeMsg((char*) lpBuf, sipCmd, NULL);

	// 为了避免不必要的出错,加上结束符号,返回长度并不包括
	//memcpy((char*)lpBuf, sipMsg, strlen(sipMsg)+1);
	//nReturn = strlen(sipMsg);

	return strlen((char*) lpBuf);
}

// 创建MDU实时视频请求后的ACK确认数据包
int CreatePack_MDU_RealStartACK(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo)
{
	char cuid[32] = { 0 };
	char MduIp[5][32] = { 0 };

	char sipCmd[1024] = { 0 };
	char sipMsg[1024] = { 0 };

	char cmd[128] = { 0 };
	char head[128] = { 0 };
	char to[128] = { 0 };
	char from[128] = { 0 };
	char cseq[128] = { 0 };
	char callid[128] = { 0 };
	int nReturn;
	
	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19

	sprintf(cuid, "%s",pchar);
	strncpy(MduIp[0], "127.0.0.1", sizeof(MduIp[0]) - 1);
	sprintf(cmd, "ACK");
	sprintf(head, "sip:127.0.0.1");
	sprintf(to, "<sip:%s:%d>", MduIp, 0);
	sprintf(from, "%s <sip:cuip:cu_port>", cuid);
	sprintf(cseq, "%d %s", nSeqNum, cmd);
	sprintf(callid, "%s", callId);

	st_sip_setSip(sipCmd, cmd, head, to, from, cseq, callid, "text/xml", 0);
	st_sip_makeMsg(sipMsg, sipCmd, NULL);

	// 为了避免不必要的出错,加上结束符号,返回长度并不包括
	memcpy((char*) lpBuf, sipMsg, strlen(sipMsg) + 1);
	nReturn = strlen(sipMsg);

	return nReturn;
}
// 发送bye指令结束本次会话
int CreatePack_MDU_Bye(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo)
{
	char cuid[32] = { 0 };
	char MduIp[5][32] = { 0 };

	char sipCmd[512];
	char sipMsg[1024];
	char xmlBuf[512];
	char cmd[128], head[128], to[128], from[128], cseq[128], callid[128];
	int nReturn;
	int outlen = 0;
	char tmpbuf[1024];

	char pchar[32] = {0};
	strcpy(pchar, theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19
	

	sprintf(cuid, "%s", pchar);
	strncpy(MduIp[0], "127.0.0.1", sizeof(MduIp[0]) - 1);
//--------------------------------------------------------------------------------
	sprintf(xmlBuf,
			"<?xml version=\"1.0\"?>\
<message command=\"%s\">\
<domainID>%s</domainID>\
<puId>%s</puId>\
<guId>%s</guId>\
<cuId>%s</cuId>\
<videoId>1</videoId>\
</message>\r\n",
			MDU_REAL_START_REQ, guInfo->DomainID, guInfo->PUID, guInfo->GUID, pchar);
//---------------------------------------------------------------------------------

	sprintf(cmd, "BYE");
	sprintf(head, "sip:127.0.0.1");
	sprintf(to, "<sip:%s:%d>", MduIp, 0);
	sprintf(from, "%s<sip:cuip:cu_port>", cuid);
	sprintf(cseq, "%d %s", nSeqNum, cmd);
	sprintf(callid, "%s", callId);

#if 0
	// 对字符串的编码进行转换, 转成UTF-8 (CP_ACP;GB2312-20936;GB18030-54936)
	int temp;
	WCHAR wideBuf[1024];
	temp = ::MultiByteToWideChar( ENCODING_IN, 0, xmlBuf, strlen(xmlBuf)+1, wideBuf, sizeof(wideBuf)/sizeof(WCHAR));
	temp = ::WideCharToMultiByte( ENCODING_OUT, 0, wideBuf, wcslen(wideBuf)+1, xmlBuf, sizeof(xmlBuf)/sizeof(char), NULL, NULL);
#endif

	st_sip_setSip(sipCmd, cmd, head, to, from, cseq, callid, "text/xml", strlen(tmpbuf));	//strlen(xmlBuf));
	st_sip_makeMsg(sipMsg, sipCmd, xmlBuf);

	// 为了避免不必要的出错,加上结束符号,返回长度并不包括
	memcpy((char*) lpBuf, sipMsg, strlen(sipMsg) + 1);
	nReturn = strlen(sipMsg);

	return nReturn;
}

int CreatePack_MDU_VoiceChart(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo, const int nProtocolType, MDU_ROUTE_INFO mduInfo)
{
    char cuid[32] = {0};
    char MduIp[5][32] = {0};
    sprintf(cuid, "%s", theApp.m_UserID );
    strncpy(MduIp[0], "127.0.0.1",sizeof(MduIp[0])-1);

    char sipCmd[512] = {0};
    char sipMsg[1024] = {0};
    char xmlBuf[1024] = {0};
    if(guInfo->DomainType == 0)
    {
        sprintf(xmlBuf, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
                <message command=\"%s\">\
                <domainID>%s</domainID>\
                <puId>%s</puId>\
                <guId>%s</guId>\
                <cuId>%s</cuId>\
                <mduId>%s</mduId>\
                <mduIp>%s</mduIp>\
                <mduPort>%d</mduPort>\
                <domainUsername></domainUsername>\
                <domainPassword></domainPassword>\
                <audioId>1</audioId>\
                <mediaType>1</mediaType>\
                <IPprotoType>%d</IPprotoType>\
                </message>\n", MDU_REAL_START_REQ, guInfo->DomainID, guInfo->PUID , guInfo->GUID, theApp.m_UserID,
                mduInfo.szLocalMduId, mduInfo.szLocalMduIp, mduInfo.uLocalPort+ST_MDU_VOICE_OFFSET, nProtocolType);
    }
    else
    {
        sprintf(xmlBuf, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
                <message command=\"%s\">\
                <domainID>%s</domainID>\
                <puId>%s</puId>\
                <guId>%s</guId>\
                <cuId>%s</cuId>\
                <mduId>%s</mduId>\
                <mduIp>%s</mduIp>\
                <mduPort>%d</mduPort>\
                <domainUsername></domainUsername>\
                <domainPassword></domainPassword>\
                <audioId>1</audioId>\
                <mediaType>1</mediaType>\
                <IPprotoType>%d</IPprotoType>\
                </message>\n", MDU_REAL_START_REQ, guInfo->DomainID, guInfo->PUID , guInfo->GUID, theApp.m_UserID,
                mduInfo.szRemoteMduId, mduInfo.szRemoteMduIp, mduInfo.uRemoteMduPort+ST_MDU_VOICE_OFFSET, nProtocolType);
    }
    char cmd[128], head[128], to[128], from[128], cseq[128], callid[128];
    sprintf(cmd, "INVITE");
    sprintf(head, "sip:127.0.0.1");
    sprintf(to, "<sip:%s:%d>", MduIp, 0);
    sprintf(from, "%s <sip:cuip:cu_port>", cuid);
    sprintf(cseq, "%d %s", nSeqNum, cmd);
    sprintf(callid, "%s", callId);

#if 0
    // 杞爜UTF-8 (CP_ACP;GB2312-20936;GB18030-54936)
    int  temp;
    WCHAR  wideBuf[1024];
    temp  = MultiByteToWideChar( ENCODING_IN, 0, xmlBuf,  strlen(xmlBuf)+1, wideBuf, sizeof(wideBuf)/sizeof(WCHAR));
    temp  = WideCharToMultiByte( ENCODING_OUT, 0, wideBuf, wcslen(wideBuf)+1, xmlBuf, sizeof(xmlBuf)/sizeof(char), NULL, NULL);
#endif

    st_sip_setSip(sipCmd, cmd, head, to, from, cseq, callid, "text/xml", strlen(xmlBuf));
    st_sip_makeMsg(sipMsg,sipCmd, xmlBuf);

    memcpy((char*)lpBuf, sipMsg, strlen(sipMsg)+1);
    int nReturn = strlen(sipMsg);

    return nReturn;
}

// 发送bye指令结束本次会话
int CreatePack_Voice_Bye(void* lpBuf, int nSeqNum, char* callId, const GUINFO* guInfo)
{
	char cuid[32] = { 0 };
	char MduIp[5][32] = { 0 };
	int nReturn;

	char cmd[128], head[128], to[128], from[128], cseq[128], callid[128];
	char sipCmd[512];
	char sipMsg[1024];
	char xmlBuf[512];
	int outlen = 0;
	char tmpbuf[1024];

	sprintf(cuid, "%s", theApp.m_UserID);
	//ConvertGBKToUtf8(pchar); //add LinZh107 2014-9-19
	
	strncpy(MduIp[0], "127.0.0.1", sizeof(MduIp[0]) - 1);
//--------------------------------------------------------------------------------
	sprintf(xmlBuf,
"<?xml version=\"1.0\"?>\
<message command=\"%s\">\
<domainID>%s</domainID>\
<puId>%s</puId>\
<guId>%s</guId>\
<cuId>%s</cuId>\
<audioId>1</audioId>\
</message>\r\n",
			MDU_REAL_START_REQ, guInfo->DomainID, guInfo->PUID, guInfo->GUID, cuid);
//---------------------------------------------------------------------------------

	sprintf(cmd, "BYE");
	sprintf(head, "sip:127.0.0.1");
	sprintf(to, "<sip:%s:%d>", MduIp, 0);
	sprintf(from, "%s<sip:cuip:cu_port>", cuid);
	sprintf(cseq, "%d %s", nSeqNum, cmd);
	sprintf(callid, "%s", callId);

#if 0
	// 对字符串的编码进行转换, 转成UTF-8 (CP_ACP;GB2312-20936;GB18030-54936)
	int temp;
	WCHAR wideBuf[1024];
	temp = ::MultiByteToWideChar( ENCODING_IN, 0, xmlBuf, strlen(xmlBuf)+1, wideBuf, sizeof(wideBuf)/sizeof(WCHAR));
	temp = ::WideCharToMultiByte( ENCODING_OUT, 0, wideBuf, wcslen(wideBuf)+1, xmlBuf, sizeof(xmlBuf)/sizeof(char), NULL, NULL);
#endif

	st_sip_setSip(sipCmd, cmd, head, to, from, cseq, callid, "text/xml", strlen(tmpbuf));//strlen(xmlBuf));
	st_sip_makeMsg(sipMsg, sipCmd, xmlBuf);

	// 为了避免不必要的出错,加上结束符号,返回长度并不包括
	memcpy((char*) lpBuf, sipMsg, strlen(sipMsg) + 1);
	nReturn = strlen(sipMsg);

	return nReturn;
}

//--------------------------------------------------------------------------------
// 函数说明: 在一字符串里查找中间字符串, 并把字符串拷贝到strOut里
// 返回: NULL表示没有满足要求的字符串, 如果有,则返回串首地址
//--------------------------------------------------------------------------------
char* GetMiddleString(char* strSrc, const char* strStart, const char* strEnd, char* strOut, int nMaxLen)
{
	char *pStr1 = NULL;
	char *pStr2 = NULL;
//ASSERT(strSrc!=NULL);
	if (strcmp(strSrc, "") == 0)
		return NULL;

	pStr1 = (char*) strstr(strSrc, strStart);
	if (NULL == pStr1)
		return NULL;
	else
	{
		pStr2 = (char*) strstr(pStr1 + strlen(strStart), strEnd);
		if (NULL == pStr2)
			return NULL;
		else
		{
			int nMidLen = pStr2 - pStr1 - strlen(strStart); // 计算间隔
			if (nMidLen < 0)
				return NULL;
			if (nMaxLen <= nMidLen)
			{
				//TRACE("GetMiddleString(), 提供缓冲区大小不够!\n");
				return NULL;
			}
			memcpy(strOut, pStr1 + strlen(strStart), nMidLen);  // 执行数据的拷贝
			*(strOut + nMidLen) = '\0';                         // 加上字符串结束符
			return pStr1 + strlen(strStart);                   // 返回中间字符串的首地址
		}
	}
}
int GetRealMDURoute(char* szXML, MDU_ROUTE_INFO* mduRoute)
{
	char * pXML = szXML;                   //const char * const
	char str1[32], str2[32], str3[32], str4[32], str5[32], str6[32], str7[32], str8[32], str9[32], str10[32], str11[32],
			str12[32], str13[32], str14[32], str15[32], str16[32], str17[32], str18[32];

	char *pStr = NULL;
	char *pTemp = NULL;
	char szMduInf[1024]; // 存储MDU信息字符串
	char szMduId[32] = {0};	//mdu id
	char szMduIP[128] = { 0 };  // mdu ip
	char szMduPort[32] = {0};  // mdu port
	// 获取<MDU></MDU>中间字符串

	strncpy(str1, "<MDU>", 16);
	strncpy(str2, "</MDU>", 16);
	//LocalMDUID
	strncpy(str3, "<MDUID0>", 16);
	strncpy(str4, "</MDUID0>", 16);
	strncpy(str5, "<MDUIP0>", 16);
	strncpy(str6, "</MDUIP0>", 16);
	strncpy(str7, "<MDUPORT0>", 16);
	strncpy(str8, "</MDUPORT0>", 16);
	//remoteMDUID
	strncpy(str9, "<MDUID1>", 16);
	strncpy(str10, "</MDUID1>", 16);
	strncpy(str11, "<MDUIP1>", 16);
	strncpy(str12, "</MDUIP1>", 16);
	strncpy(str13, "<MDUPORT1>", 16);
	strncpy(str14, "</MDUPORT1>", 16);
	strncpy(str15, "<SubCMUIP>", 16);
	strncpy(str16, "</SubCMUIP>", 16);
	strncpy(str17, "<CMUPORT>", 16);
	strncpy(str18, "</CMUPORT>", 16);
	pStr = GetMiddleString(pXML, str1, str2, szMduInf, sizeof(szMduInf));
	if (NULL == pStr)
		return -1;
	//retrive mduid
	pTemp = GetMiddleString(pStr, str3, str4, szMduId, sizeof(szMduId));
	sprintf(mduRoute->szLocalMduId, "%s", szMduId);
	// 获取<MDUIP></MDUIP>之见的字符串
	pTemp = GetMiddleString(pStr, str5, str6, szMduIP, sizeof(szMduIP));
	if (NULL == pTemp)
		return -1;
	sprintf(mduRoute->szLocalMduIp, "%s", szMduIP);
	// 获取port
	pTemp = GetMiddleString(pStr, str7, str8, szMduPort, sizeof(szMduPort));
	if (NULL == pTemp)
		return -1;
	mduRoute->uLocalPort = (unsigned int) atoi(szMduPort);
	//Remote mdu info	//mdu id
	memset(szMduId, 0, sizeof(szMduId));
	pTemp = GetMiddleString(pStr, str9, str10, szMduId, sizeof(szMduId));
	sprintf(mduRoute->szRemoteMduId,"%s", szMduId); 
	// remote mdu ip
	memset(szMduIP, 0, sizeof(szMduIP));
	pTemp = GetMiddleString(pStr, str11, str12, szMduIP, sizeof(szMduIP));
	sprintf(mduRoute->szRemoteMduIp, "%s", szMduIP);
	// 获取port
	memset(szMduPort, 0, sizeof(szMduPort));
	pTemp = GetMiddleString(pStr, str13, str14, szMduPort, sizeof(szMduPort));
	mduRoute->uRemoteMduPort = (unsigned int) atoi(szMduPort);
	//Remote cmu info
	memset(szMduIP, 0, sizeof(szMduIP));
	pTemp = GetMiddleString(pStr, str15, str16, szMduIP, sizeof(szMduIP));
	sprintf(mduRoute->szRemoteCsgIp, "%s", szMduIP);
	// 获取port
	memset(szMduPort, 0, sizeof(szMduPort));
	pTemp = GetMiddleString(pStr, str17, str18, szMduPort, sizeof(szMduPort));
	mduRoute->uRemoteCsgPort = (unsigned int) atoi(szMduPort);
	return 0;
}

int GetHeadLine(char* szBuf, char* szHead, int nMax)
{
	char *pStr = NULL;
	pStr = (char*) strstr(szBuf, "\n");
	if (pStr == NULL)
	{
		int nLen = strlen(szBuf) + 1;
		if (nMax < nLen)
			return -1;
		else
		{
			strncpy(szHead, szBuf, nLen);
			szHead[nLen] = '\0';
		}
	}
	else
	{
		int nLen = strlen(szBuf) - strlen(pStr) + 1;
		if (nMax < nLen)
			return -1;
		else
		{
			strncpy(szHead, szBuf, nLen);
			szHead[nLen] = '\0';
		}
	}
	return 0;
}

void GetVideoIdFromGuid(char* lpszGuid, int* nVideoId, int* nGuType)	//LPCTSTR->char*
{
	char szVideoId[3] = { 0 };
	char szGuType[1] = { 0 };
	int nlen = strlen(lpszGuid);
	if (nlen < 2)
		return;

	szVideoId[0] = lpszGuid[nlen - 2];
	szVideoId[1] = lpszGuid[nlen - 1];

	(*nVideoId) = atoi(szVideoId);

	memcpy(&szGuType, &lpszGuid[nlen - 3], 1);
	(*nGuType) = atoi(szGuType);

}

void ConvertGBKToUtf8(char* strGBK) //CString &LPCSTR
{
	int inlen;
	int outlen = 0;
	char tmpbuf[1024];

	if (!strGBK)
	{
		return;
	}
#if 0
	int len=MultiByteToWideChar(CP_ACP, 0, (char*)strGBK, -1, NULL,0); //LPCTSTR-->LPCSTR
	//unsigned char* wszUtf8 = new unsigned char[len+1];
	WCHAR * wszUtf8 = new WCHAR[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	len = MultiByteToWideChar(CP_ACP, 0, (char*)strGBK, -1, wszUtf8, len);//
	if (!len)
	{
		return;
	}

	len = WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, NULL, 0, NULL, NULL);
	char *szUtf8=new char[len + 1];
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte (CP_UTF8, 0, wszUtf8, -1, szUtf8, len, NULL,NULL);
#endif
	inlen = strlen(strGBK) + 1;
	//g2u(strGBK, &inlen, tmpbuf, &outlen);
	GB2312ToUTF_8(tmpbuf, strGBK, inlen);
	//	strGBK = szUtf8;
	strcpy(strGBK, tmpbuf); //modified by zhusy
	//delete[] szUtf8;
	//delete[] wszUtf8;
}

///////////////////////////////////////////////////////
// UTF8编码转换到GBK编码
int UTF8ToGBK(char * lpUTF8Str, char * lpGBKStr, int nGBKStrLen)
{
//	int inlen;
	int outlen = 0;

#if 0
	wchar_t * lpUnicodeStr = NULL;
	int nRetLen = 0;

	if(!lpUTF8Str)  //如果UTF8字符串为NULL则出错退出
	return 0;

	nRetLen = ::MultiByteToWideChar(CP_UTF8,0,(char *)lpUTF8Str,-1,NULL,NULL);//获取转换到Unicode编码后所需要的字符空间长度
	lpUnicodeStr = new WCHAR[nRetLen + 1];//为Unicode字符串空间
	nRetLen = ::MultiByteToWideChar(CP_UTF8,0,(char *)lpUTF8Str,-1,lpUnicodeStr,nRetLen);//转换到Unicode编码
	if(!nRetLen)//转换失败则出错退出
	return 0;

	nRetLen = ::WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,NULL,NULL,NULL,NULL);//获取转换到GBK编码后所需要的字符空间长度

	if(!lpGBKStr)//输出缓冲区为空则返回转换后需要的空间大小
	{
		if(lpUnicodeStr)
			delete []lpUnicodeStr;
		return nRetLen;
	}

	if(nGBKStrLen < nRetLen)  //如果输出缓冲区长度不够则退出
	{
		if(lpUnicodeStr)
		delete []lpUnicodeStr;
		return 0;
	}

	nRetLen = ::WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,(char *)lpGBKStr,nRetLen,NULL,NULL);  //转换到GBK编码

	if(lpUnicodeStr)
	delete []lpUnicodeStr;
#endif
	//u2g(lpUTF8Str, &nGBKStrLen, tmpbuf, &outlen);
	//UTF_8ToGB2312(lpGBKStr, lpUTF8Str, int pLen);
	return outlen;
}
///////////////////////////////////////////////////////

void ConvertUtf8ToGBK(char* strUtf8)
{
	int inlen;
	int outlen = 0;
	char tmpbuf[1024];

	if (!strUtf8)
	{
		return;
	}
#if 0
	int len=MultiByteToWideChar(CP_ACP, 0, (char*)strGBK, -1, NULL,0); //LPCTSTR-->LPCSTR
	//unsigned char* wszUtf8 = new unsigned char[len+1];
	WCHAR * wszUtf8 = new WCHAR[len + 1];
	memset(wszUtf8, 0, len * 2 + 2);
	len = MultiByteToWideChar(CP_ACP, 0, (char*)strGBK, -1, wszUtf8, len);//
	if (!len)
	{
		return;
	}

	len = WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, NULL, 0, NULL, NULL);
	char *szUtf8=new char[len + 1];
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte (CP_UTF8, 0, wszUtf8, -1, szUtf8, len, NULL,NULL);
#endif
	inlen = strlen(strUtf8);
	//u2g(strUtf8, &inlen, tmpbuf, &outlen);
	UTF_8ToGB2312(tmpbuf, strUtf8, inlen);
	//	strGBK = szUtf8;
	strcpy(strUtf8, tmpbuf);

}

//////////////////////////////////////////////////////////////////////////
int Recv(int fd, char *vptr, int n, int flag)
{
	int nleft, nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nread = recv(fd, ptr, nleft, flag)) < 0)
		{
			//if (errno == WSAEINTR)
			//	nread = 0;
			//else
			//	return(-1);
		}
		else if (nread == 0)
		{
			break;  //EOF
		}
		nleft -= nread;
		ptr += nread;
	}
	return (n - nleft);
}

int Send(int fd, char *vptr, int n, int flag)
{
	int nleft;
	int nwritten;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nwritten = send(fd, ptr, nleft, flag)) <= 0)
		{
// 			if (errno == WSAEINTR)
// 				nwritten = 0;
// 			else
// 				return(-1);
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n - nleft);
}

void LoadStringFromID(void* hHandle, char* pszID, char* pszContent, int len)
{
	if (len < (int) strlen(pszID))
		return;
	//ASSERT(0);
	strncpy(pszContent, pszID, len);
	return;
}
