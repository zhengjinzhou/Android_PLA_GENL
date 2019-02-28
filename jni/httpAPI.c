/******************************************************
 * Copyright (c) 2007 Santachi
 * All rights reserved
 *
 * filename: http.c
 * content: implementation file, pack http protocol and parse http protocol
 *
 * version: v1.0.0
 * author: weiyu
 * m:zoujl
 * date: 2007-09-17
 *******************************************************/
//#include "stdafx.h"	//VC下打开使用
//#define __HTTP_API_EXP_    // 此宏用来决定macro HTTP_APT
#include "httpAPI.h"       // 包含需要导出的函数\结构\宏
#include "netglobal.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>   		// inet(3) functions
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>  		// sockaddr_in{} and other Internet defns
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>   		// basic system data types
#include <sys/socket.h>  		// basic socket definitions
#include <sys/time.h>    		// timeval{} for select()
#include <signal.h>

#include <android/log.h>
#define  LOG_TAG "httpAPI.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//using namespace HTTP_LIB;
//private
int st_http_getSeq(const char *http_head);
int st_http_getHead(const char *recv_buf, char *http_head, char *http_body);
char *st_http_getLine(const char *http_head, char *line);
char *st_http_getMethod(const char *line, char *method);
char *st_http_getURL(const char *line, char *pURL);
char *st_http_getVersion(const char *line, char *version);
int st_http_recv(int sock, char *recv_data, int len, int timeout);
int st_http_recvLine(int sock, char *recv_line);

int st_http_scanHttpMsg(const char *recv_buf);

static int my_readn(int fd, char*buf, int nLen, struct timeval* timeout);

static int readHeadFlag(int fd, char* buf, int nMaxLen, struct timeval* timeout);

//--------------------------------------------------------------------------
// 返回值:
//     -1: socket错误
//     -2: 其他非socket错误
//      0: 超时
//     >0: 为有效数据长度
//-------- ------------------------------------------------------------------
int  recvPackage(int fd, char* buf, unsigned int nMax,  long long msec)
{
	int  nHeadLen = 0;
	int  nBodyLen = 0;
	char *pStr = NULL;
	char szLengthFlag[32];
	sprintf(szLengthFlag, "Content-Length:");

	struct timeval  tv;
	tv.tv_sec = msec/1000;
	tv.tv_usec = (msec*1000)%1000000;

	nHeadLen = readHeadFlag( fd, buf, nMax, &tv);  // 读取到头结束标志
//	LOGI("!!!!!\nsocket recv =%s\n!!!!!",	buf);
    if( nHeadLen <=0)
	{
		return nHeadLen;  
	}
	buf[nHeadLen] = '\0';   // nHeadlen为包首部的长度
	// 已经到到包的首部, 需要检查有没有BODY的长度标志
	pStr = strstr(buf, szLengthFlag);  // 寻找BODY长度标志"Content Length:"
	if( NULL == pStr)                  // 如果没有找到长度标志,那么认为没有BODY,应返回头长度
		return nHeadLen;
	nBodyLen = atoi(pStr + strlen(szLengthFlag));
	if( nBodyLen <= 0 )    // 长度字段<=0,则认为没有BODY,应返回头长度
		return nHeadLen;

	// 需要判断缓冲区是否足够
	if( (int)nMax <= (nHeadLen+nBodyLen))
	{
//		LOGI("Notice! Head len=%d, Body len=%d", nHeadLen, nBodyLen);
		return -2;
	}

	// 长度标志有效,应根据长度标志读取BODY的长度, 添加到外部缓冲区
	int nRecv =  my_readn( fd, buf+nHeadLen, nBodyLen, &tv);  	
	if(nRecv == nBodyLen)  
	{		
		return  nHeadLen + nBodyLen;	// 读BODY成功则返回总长度
	}
	else
		return nRecv; 
}

//-------------------------------------------------------------------------
// 尝试读取socket缓冲区以寻找头结束标志
// socket错误返回-1, 只有当select超时才返回0;
// 其他错误返回-2 ;
// 成功返回>0, 为buf的有效长度
// 要求: 1- 严格保证函数在给定时间内返回
//       2- 在第一条允许的范围内保证可以进行断点调试
#define MAX_SIZE_HTTPHEAD 1024

int my_readn(int fd, char*buf, int nLen, struct timeval* timeout)
{
	fd_set fd_read;
	int ret = 0;
	int nRecv = 0;
	int nLeft = nLen;
	unsigned int pos = 0;

	int nReturn =0;
	while (nLeft > 0)
	{
		// 这里超时时间应该要递减
		//	ret = select( fd+1, &fd_read, NULL, NULL, timeout);
		FD_ZERO(&fd_read);
		FD_SET(fd, &fd_read);
		//errno = 0;
		ret = select(fd+1, &fd_read, NULL, NULL, timeout);	//ret = select( fd, &fd_read, NULL, NULL, timeout);
		if (ret <= 0)
		{
//			LOGI("my_readn err %s", strerror(errno));
			return ret;
		}
		nRecv = recv(fd, buf + pos, nLeft, 0);
		if (nRecv < 0)
		{
			if (errno == EINTR)
			{
				errno = 0;
				nRecv = 0;
			}
			else
			{
				//my_log("my_readn err %s\n", strerror(errno));
				return -1;
			}
		}
		else if (nRecv == 0)
		{
			return 0;
			break;
		}
		nLeft -= nRecv;
		pos += nRecv;
	}
	//2014-6-9 LinZh107 modify
	if( nLeft > 0)  // 如果已经读到一些数据, 则返回数据的长度
	{
		nReturn = nLen - nLeft;		
	}
	else
		nReturn = nLen;
	return nReturn;
}

#if 0
int  readHeadFlag(int fd, char* buf, int nMaxLen, struct timeval* timeout)
{
	fd_set fd_read;
	int ret = 0;
	int nRecv = 0 ;
	int nRecvAll = 0;
	int nHeadLen = 0;
	char szHeadFlag[32];
	strcpy(szHeadFlag, "\r\n\r\n");

	FD_ZERO(&fd_read);
	FD_SET(fd, &fd_read);

	ret = select(fd + 1, &fd_read, NULL, NULL, timeout);//fd

	if (ret <= 0)
	{
		return ret;
	}

	while (1)
	{
		nRecv = read(fd, buf+nRecvAll, 1); // 一次读取字节数
//		LOGI("!!!!!\nsocket recv =%s\n!!!!!",	buf);
		if( nRecv < 0)
		{
			if ((errno == EINTR) || (errno == EINPROGRESS))
			{
				errno = 0;
				nRecv = 0;
				continue;
			}
			else
			{
				return -1;
			}
		}

		if(nRecv == 0)
		{
			return -1;
		}

		nRecvAll += nRecv;
		if (nRecvAll >= strlen(szHeadFlag))
		{
			char* pStr = NULL;
			pStr = strstr(buf, szHeadFlag);    // 寻找头结束标志"\r\n\r\n"
			if( NULL == pStr)                  // not found,continue finding
			{
				continue;
			}
			nHeadLen = pStr - buf + strlen(szHeadFlag);
			if( nHeadLen >= MAX_SIZE_HTTPHEAD)
			{
				return 0;
			}

			return nHeadLen;
		}
	}
}
#else    
//-------------------------------------------------------------------------
// 尝试读取socket缓冲区以寻找头结束标志
// socket错误返回-1, 只有当select超时才返回0;
// 其他错误返回-2 ;
// 成功返回>0, 为buf的有效长度
// 要求: 1- 严格保证函数在给定时间内返回
//       2- 在第一条允许的范围内保证可以进行断点调试
static unsigned int GetTickCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#define MAX_SIZE_HTTPHEAD 1024 
int readHeadFlag(int fd, char* buf, int nMaxLen, struct timeval* timeout)
{
	//PC_NET_LIBRARY
	fd_set fd_read;
	
	int nReturn 	  = 0;
	int  nRecv        = 0;
	int  nHeadLen     = 0;
	int  nRecvAll     = 0;
	int  nRecvLastAll = 0;
	char temp[MAX_SIZE_HTTPHEAD];

	char szHeadFlag[32];
	sprintf(szHeadFlag, "\r\n\r\n"); 

	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	DWORD   dwStart, dwUsed;  // 记录开始时间和select时间开销
	dwUsed =0;
	dwStart = GetTickCount();
	dwUsed  = GetTickCount();
	
	//这种判断延时的方式会有0 ~ +3秒以内的误差。
	while((dwUsed-dwStart) <= (DWORD)(timeout->tv_sec*1000 + timeout->tv_usec/1000))
	{		
		FD_ZERO(&fd_read);
		FD_SET(fd, &fd_read);

		int ret = select(fd+1, &fd_read, NULL, NULL, &tv); 
		if (ret <= 0)
		{
			if(ret < 0)
			{
//				LOGI("call select() function error");
				return ret;
			}
			dwUsed = GetTickCount();
			continue;
		}
		
		// 从系统缓冲区里COPY数据包到到应用程序接收缓冲区
		int nRecvOnce = 256;
		if( nMaxLen < 256)
			nRecvOnce = nMaxLen;
		
		nRecv= recv( fd, buf+nRecvAll, nRecvOnce, MSG_PEEK); // 一次读取字节数
		if( nRecv <= 0)
		{
//			LOGI("readHeadFlag: select return >0, but recv=%d", nRecv);
			nReturn = 1053;//SOCKET_ERROR;
			dwUsed = GetTickCount();
			if(nRecv < 0)
			{
//				LOGE("readHeadFlag: select return >0, but recv=%d", nRecv);
				return nRecv;
			}
			continue;
		}
		nRecvLastAll = nRecvAll;    // 保存上一次读取后的总长度
		nRecvAll += nRecv;          // 更新目前已经读取的总长度
		if( nMaxLen < nRecvAll)   // 检查外部接收缓冲区长度,如果不够长则返回
		{
			recv( fd, temp, nRecv, 0); // remove from socket buffer, buf里的数据不效
			nReturn =  -2;
			break;
		}
		buf[nRecvAll] = '\0';

		// 分析接收到的数据中是否有头部结束标志"\r\n\r\n"
		char* pStr = NULL;
		pStr = strstr(buf, szHeadFlag);    // 寻找头结束标志"\r\n\r\n"
		if( NULL == pStr)                  // not found,continue finding
		{
			recv( fd, temp, nRecv, 0);     // 本次读出的数据需要全部remove
			continue;
		}
		// 如果buf里已经有了头结束标志
		nHeadLen = pStr - buf + strlen(szHeadFlag);
		// remove socket buf
		recv(fd, temp, nHeadLen- nRecvLastAll, 0);  
		// 可能的异常情况:经过几次循环读取后,这个长度过大,应将包丢弃并返回错误(这样的包不允许进一步解析)
		if( nHeadLen >= MAX_SIZE_HTTPHEAD)  	
			nReturn =  -2;
		// 这个包首部符合常规要求, 然后返回这个包首部的长度
		nReturn =  nHeadLen;
		break;
	}
	return nReturn;
}
#endif

// 在指定时间内读取指定的长度
// 有数据返回读取的数据长度; 只有当select超时才返回0, 其他(包括对端正常关闭)都返回socket error
int st_http_recv(int sock, char *recv_data, int len, int timeout)
{
	int recv_len = 0;
	int recv_left = 0;
	int recv_read = 0;
	int n_ret = 0;
	fd_set readfds, oldfds;
	struct timeval tv;

	recv_left = len;

	if (timeout <= 0)
		return 0;

	FD_ZERO(&oldfds);
	FD_SET(sock, &oldfds);

	while (recv_left > 0)
	{
		--timeout;

		readfds = oldfds;
		tv.tv_sec = timeout/1000;
		tv.tv_usec = (timeout*1000)%1000000;

		errno = 0;
		n_ret = select(sock + 1, &readfds, NULL, NULL, &tv);
		//printf("select value: %d, timeout: %d, %d\n", n_ret, timeout, recv_left);
		if (n_ret <= 0)
		{
			//			printf("select error %s, %d\n", strerror(errno), GetLastError());
			return -1;
		}
		else if (n_ret == 0)
			return recv_len;

		recv_read = recv(sock, recv_data + recv_len, recv_left, 0);
		if ((recv_read) < 0)
		{
			//			printf("receive error %s\n", strerror(errno));
			if (errno == EINTR)
				recv_read = 0;
			else
				return -1;
		}
		else if (recv_read == 0)
			break;

		recv_left -= recv_read;
		recv_len += recv_read;

	}
	return recv_len;
}
int st_http_recvLine(int sock, char *recv_line)
{
	///////////////////////////////////

	int recv_len = 0;
	int recv_ret = 0;
	char *ptr = NULL;                  //NULL
	int dwStart = time(NULL);

	ptr = recv_line;
	while (1)
	{
		recv_ret = st_http_recv(sock, ptr, 1, 5); //st_http_recv(sock, ptr, 1, 10)
		//printf("%c", *ptr);
		if (recv_ret < 0)
			return -1;
		else if (recv_ret == 0)
			return recv_len;
		if (*ptr == '\n')
		{
			recv_len++;
			break;
		}
		ptr += recv_ret;
		recv_len += recv_ret;

// 		time_end = time((time_t*)0);
// 		//如果5秒中还没有收到\n，认为该数据有问题
// 		if (difftime(time_bg, time_end) > 5)
// 			return -2;

		if (time(NULL) - dwStart >= 5)
			return -2;
	}
	*(++ptr) = '\0';

	return recv_len;

#if 0
	//PC_NET_LIBRARY
	ssize_t n, rc;
	char c, *ptr;
	int n_ret = 0;
	struct timeval tv;
	fd_set readfds;
	int dwStart;

	ptr = recv_line;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	FD_ZERO (&readfds);
	FD_SET (sock, &readfds);

	n_ret = select(sock + 1, &readfds, NULL, NULL, &tv);
	//printf("select value: %d, timeout: %d, %d\n", n_ret, timeout, recv_left);
	if (n_ret < 0)
	{
//			printf("select error %s, %d\n", strerror(errno), GetLastError());
		return -1;
	}
	else if(n_ret == 0)
	return 0;

	dwStart = time(NULL);

	while(1)
	{
		again:
		if ((rc = read(sock, &c, 1)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
			break; /* newline is stored, like fgets() */
		}
		else if (rc == 0)
		{
			*ptr = 0;
			return (n - 1); /* EOF, n-1 bytes were read */
		}
		else
		{
			if (errno== EINTR)
			goto again;
			return (-1);
		}
	}
	*ptr = 0; /* null terminate like fgets() */
	return (n);
#endif
}
//

// 导出函数必须是属于namespace HTTP_LIB的
//recv http package
int st_http_recvHttpPackage(int sock, char *recv_buf)	//HTTP_LIB::
{
	int len = 0;
	int head_len = 0;
	int body_len = 0;
	char *pTmp = NULL;
	char *token = NULL;
	char *pRecv = NULL;
	char line[1024];

	memset(recv_buf, 0, strlen(recv_buf));                  //sizeof(recv_buf) //modified by zhusy
	memset(line, 0, sizeof(line));

	pRecv = recv_buf;
	while (1)
	{
		len = st_http_recvLine(sock, line);
		if (len < 0)
			return len;
		else if (len == 0)
			break;

		memcpy(pRecv, line, len);
		pRecv += len;
		head_len += len;
		if (strstr(line, "Content-Length:") != NULL)
		{
			token = strtok(line, ":");
			token = strtok(NULL, ":");
			if (token != NULL)
				body_len = atoi(token);
		}
		if (strcmp(line, "\r\n") == 0)
			break;
	}

	if (body_len != 0)
	{
		len = st_http_recv(sock, pRecv, body_len, 20);
		if (len <= 0)
			return len;
		else if (len != body_len)
			return -1;
	}
	//printf("%s, rv:%s\n", pRecv, recv_buf - head_len - body_len);
	return head_len + body_len;
}
//

//pack

char* st_http_addParam(char *buffer, char *fmt, ...)                  // HTTP_LIB::
{
	char *s;
	int d;
	char *p = NULL;
	char buf[32];
	va_list args;

	//assert(buffer != NULL);
	va_start(args, fmt);

	while (*fmt != '\0')
	{
		if (*fmt == '%')
		{
			fmt++;
			if (*fmt == 's')
			{
				s = va_arg(args, char *);
				strcat(buffer, s);
			}
			else if (*fmt == 'd')
			{
				memset(buf, 0, 32);
				d = va_arg(args, int);
				sprintf(buf, "%d", d);
				strcat(buffer, buf);
			}
		}
		else
		{
			memset(buf, 0, 32);
			sprintf(buf, "%c", *fmt);
			strcat(buffer, buf);
		}
//	  	p = fmt;
//		if ( *++p != '\0')
//			strcat(buffer, "&");
		fmt++;
	}
	va_end(args);

	return buffer;
}

//
//HTTP_LIB::
char* st_http_makeHttp(const char * type, unsigned int seq, char *buffer, int *len, char *cmd, char *param_buf,
		char *xml_buf)		//输入参数, 请求时默认为空
{
	char* buf = NULL;
	int n_len = 0;

	buf = (char*) malloc(512);
	memset(buf, 0, (sizeof(char) * 512));		//modified by zhusy

	if (strcmp(type, "GET") == 0)
	{
		if (param_buf == NULL)
			sprintf(buf, "GET /%s? HTTP/1.0\r\nSeq:%d\r\n\r\n", cmd, seq);
		else
			sprintf(buf, "GET /%s?%s HTTP/1.0\r\nSeq:%d\r\n\r\n", cmd, param_buf, seq);
	}
	else if (strcmp(type, "POST") == 0)
	{
		n_len = strlen(param_buf);
		if (param_buf == NULL)
			sprintf(buf, "POST /%s HTTP/1.0\r\nContent-Type:Charset=UTF-8\r\nContent-Length:0\r\nSeq:%d\r\n\r\n", cmd,
					seq);
		else
			sprintf(buf, "POST /%s HTTP/1.0\r\nContent-Type:Charset=UTF-8\r\nContent-Length:%d\r\nSeq:%d\r\n\r\n%s",
					cmd, n_len, seq, param_buf);
	}
	else if (strcmp(type, "RESPONSE") == 0)
	{
		n_len = strlen(xml_buf);

		if (xml_buf == NULL)
			sprintf(buf,
					"HTTP/1.1 200 OK\r\nCommand:%s\r\nContent-Type:text/xml;Charset=UTF-8\r\nContent-Length:0\r\nSeq:%d\r\n\r\n",
					cmd, seq);
		else
			sprintf(buf,
					"HTTP/1.1 200 OK\r\nCommand:%s\r\nContent-Type:text/xml;Charset=UTF-8\r\nContent-Length:%d\r\nSeq:%d\r\n\r\n\%s",
					cmd, n_len, seq, xml_buf);
	}

	*len = strlen(buf);

	memcpy(buffer, buf, strlen(buf));

	if (NULL != buf)
	{
		free(buf);
		buf = NULL;
	}

	return buffer;
}
//
#if 1
unsigned char st_http_parseHttp(const char *recv_buf, HTTP_OBJECT *ho_ptr)
{	//PC_NET_LIBRARY
	char http_head[1024] =	{0};
	char http_body[HTTP_XML_BUF_SIZE] =	{0};
	char line[1024] =	{0};
	char method[32] =	{0};
	char pURL[1024] =	{0};
	char *token = NULL;
	char *_token = NULL;
	int i = 0;
	int head_len = 0;
	char *pTmp1 = NULL;
	char *pTmp2 = NULL;
	char tmp_buf[64] =
	{	0};

	if (recv_buf == NULL)
	return -1;
	memset(ho_ptr, 0, sizeof(HTTP_OBJECT));

	//recv_buf 可能已被破坏
	int j = sizeof(recv_buf);

	if (st_http_scanHttpMsg(recv_buf) != 0)
	{
//		LOGE("st_http_scanHttpMsg(recv_buf) != 0");
		return 0;
	}

	head_len = st_http_getHead(recv_buf, http_head, http_body);
	//LOGI("st_http_getHead(head_len)=%d", head_len);

	ho_ptr->seq = st_http_getSeq(http_head);
	st_http_getLine(http_head, line);
	st_http_getMethod(line, method);
	st_http_getURL(line, pURL);

	if (strcmp(method, "GET") == 0)
	{
		//get command
		ho_ptr->type = REQ_TYPE_GET;
		//LOGI("set REQ_TYPE_GET");
		pTmp1 = strstr(pURL, "/");
		pTmp2 = strstr(pURL, "?");
		if (pTmp2 == NULL)
		strcpy(ho_ptr->cmd, pTmp1 + 1);
		else
		strncpy(ho_ptr->cmd, pTmp1 + 1, (pTmp2 - pTmp1 - 1));
		//get parameters
		pTmp2 += 1;
		if (pTmp2 == NULL)
		return 0;
		else
		{
			token = strtok(pTmp2, "&");
			while (token != NULL)
			{
				_token = strstr(token, "=");
				strncpy(ho_ptr->cmdparm[i].paramName, token, _token - token);

				strcpy(ho_ptr->cmdparm[i].paramValue, _token + 1);

				token = strtok(NULL, "&");
				i++;
			}
		}
	}
	else if (strcmp(method, "POST") == 0)
	{
		//get command
		ho_ptr->type = REQ_TYPE_POST;
		//LOGI("set  EQ_TYPE_POST");
		pTmp1 = pURL;
		while ((pTmp1 = strstr(pTmp1, "/")) != NULL)
		{
			pTmp1 = pTmp1 + 1;
			strcpy(ho_ptr->cmd, pTmp1);
		}
		//get parameters
		//st_http_getURL (line, pURL);
		if (http_body == NULL)
		return 0;
		else
		{
			memcpy(ho_ptr->xml, http_body, strlen(http_body));
//			pTmp1 = http_body;
//			token = strtok (pTmp1, "&");
//			while (token != NULL)
//			{
//				_token = strstr(token, "=");
//				strncpy (ho_ptr->cmdparm[i].paramName, token, _token - token);
//
//				strcpy (ho_ptr->cmdparm[i].paramValue, _token + 1);
//
//				token = strtok (NULL, "&");
//				i++;
//			}
		}
	}
	else if (strcmp(method, "HTTP/1.1") == 0)
	{
		//get command
		ho_ptr->type = RESP_TYPE;
		//LOGI("set RESP_TYPE");
		if (http_body == NULL)
		{
			printf("Parse Error! \n");
			return 0;
		}
		else
		{
			pTmp1 = http_head;
			pTmp1 = strstr(pTmp1, "Command");
			pTmp2 = strstr(pTmp1, "\r\n");
			pTmp1 = strstr(pTmp1, ":");
			pTmp1 += 1;
			memcpy(tmp_buf, pTmp1, pTmp2 - pTmp1 + 1);
			memcpy(ho_ptr->cmd, tmp_buf, pTmp2 - pTmp1);
			memcpy(ho_ptr->xml, http_body, strlen(http_body));
		}
	}
	return 1;
}
//

#else
//parse   modify 2014-6-4
unsigned char st_http_parseHttp(const char *recv_buf, HTTP_OBJECT *ho_ptr)		//HTTP_LIB::
{
	char method[32] = { 0 };
	char *token = NULL;
	char *_token = NULL;
	int i = 0;
	int head_len = 0;
	char *pTmp1 = NULL;
	char *pTmp2 = NULL;
	char tmp_buf[64] = { 0 };

	//add by zhusy
	char* http_body = NULL;
	char* http_head = NULL;
	char* pURL = NULL;
	char* line = NULL;
	int ret = 0;

	memset(ho_ptr, 0, sizeof(HTTP_OBJECT));
	////////////////////////////////add by zhusy start

	http_body = (char*) malloc(HTTP_XML_BUF_SIZE);
	memset(http_body, 0, HTTP_XML_BUF_SIZE);		//

	http_head = (char*) malloc(1024);		//
	memset(http_body, 0, 1024);

	pURL = (char*) malloc(1024);
	memset(pURL, 0, 1024);

	line = (char*) malloc(1024);
	memset(line, 0, 1024);

	/////////////////////////////////add by zhusy end!

	if (st_http_scanHttpMsg(recv_buf) != 0)
	{

		ret = -1;
		//my_log("st_http_scanHttpMsg error/n ");
		goto error;
	}

	head_len = st_http_getHead(recv_buf, http_head, http_body);

	ho_ptr->seq = st_http_getSeq(http_head);

	char *ptmp = st_http_getLine(http_head, line);
		LOGI("parseHttp line = %s",ptmp);
		
	 ptmp = st_http_getMethod(line, method);
	 	LOGI("parseHttp method = %s",ptmp);

	 ptmp = st_http_getURL(line, pURL);
		LOGI("parseHttp pURL = %s",ptmp);

	if (strcmp(method, "GET") == 0)
	{
		//get command
		ho_ptr->type = REQ_TYPE_GET;
		pTmp1 = strstr(pURL, "/");
		pTmp2 = strstr(pURL, "?");
		if (pTmp2 == NULL)
			strcpy(ho_ptr->cmd, pTmp1 + 1);
		else
			strncpy(ho_ptr->cmd, pTmp1 + 1, (pTmp2 - pTmp1 - 1));
		//get parameters
		pTmp2 += 1;
		if (pTmp2 == NULL)
		{
#if 0
			if(NULL != http_body)
			{
				free(http_body);
				http_body = NULL;
			}

			if(NULL != http_head)
			{
				free(http_head);
				http_head = NULL;
			}

			if(NULL != pURL)
			{
				free(pURL);
				pURL = NULL;
			}

			if(NULL != line)
			{
				free(line);
				line = NULL;
			}
#endif
			ret = -1;
			goto error;
			return -1;
		}
		else
		{
			token = strtok(pTmp2, "&");
			while (token != NULL)
			{
				_token = strstr(token, "=");
				strncpy(ho_ptr->cmdparm[i].paramName, token, _token - token);

				strcpy(ho_ptr->cmdparm[i].paramValue, _token + 1);

				token = strtok(NULL, "&");
				i++;
			}
		}
	}
	else if (strcmp(method, "POST") == 0)
	{
		//get command
		ho_ptr->type = REQ_TYPE_POST;
		pTmp1 = pURL;
		while ((pTmp1 = strstr(pTmp1, "/")) != NULL)
		{
			pTmp1 = pTmp1 + 1;
			strcpy(ho_ptr->cmd, pTmp1);
		}
		//get parameters
		//st_http_getURL (line, pURL);
		if (http_body == NULL)
		{
#if 0
			if(NULL != http_body)
			{
				free(http_body);
				http_body = NULL;
			}

			if(NULL != http_head)
			{
				free(http_head);
				http_head = NULL;
			}

			if(NULL != pURL)
			{
				free(pURL);
				pURL = NULL;
			}

			if(NULL != line)
			{
				free(line);
				line = NULL;
			}
#endif
			ret = -1;
			goto error;

			return -1;
		}
		else
		{
			pTmp1 = http_body;

			token = strtok(pTmp1, "&");
			while (token != NULL)
			{
				_token = strstr(token, "=");
				strncpy(ho_ptr->cmdparm[i].paramName, token, _token - token);

				strcpy(ho_ptr->cmdparm[i].paramValue, _token + 1);

				token = strtok(NULL, "&");
				i++;
			}
		}
	}
	else if (strcmp(method, "HTTP/1.1") == 0)
	{
		//get command
		ho_ptr->type = RESP_TYPE;
		if (http_body == NULL)
		{
			//printf ("Parse Error! \n");
			//////////////////////
#if 0
			if(NULL != http_body)
			{
				free(http_body);
				http_body = NULL;
			}

			if(NULL != http_head)
			{
				free(http_head);
				http_head = NULL;
			}

			if(NULL != pURL)
			{
				free(pURL);
				pURL = NULL;
			}

			if(NULL != line)
			{
				free(line);
				line = NULL;
			}
#endif
			////////////////////
			ret = -1;
			goto error;

			return -1;
		}
		else
		{
			pTmp1 = http_head;
			pTmp1 = strstr(pTmp1, "Command");
			pTmp2 = strstr(pTmp1, "\r\n");
			pTmp1 = strstr(pTmp1, ":");
			pTmp1 += 1;
			memcpy(tmp_buf, pTmp1, pTmp2 - pTmp1 + 1);
			memcpy(ho_ptr->cmd, tmp_buf, pTmp2 - pTmp1);
			memcpy(ho_ptr->xml, http_body, strlen(http_body));
		}
	}

	///////////////////////added by zhusy start
error: 
	if (NULL != http_body)
	{
		free(http_body);
		http_body = NULL;
	}

	if (NULL != http_head)
	{
		free(http_head);
		http_head = NULL;
	}

	if (NULL != pURL)
	{
		free(pURL);
		pURL = NULL;
	}

	if (NULL != line)
	{
		free(line);
		line = NULL;
	}
	///////////////////////added by zhusy end!

	return ret;
}
//
#endif 

int st_http_parseHttpParam(HTTP_OBJECT *ho_ptr, const char *szParam, char *parseBuf)                //HTTP_LIB::
{
	int i = 0;
	for (i = 0; i < MAX_PARAM_NUM; i++)
	{
		if (0 == strcmp(ho_ptr->cmdparm[i].paramName, szParam))
		{
			sprintf(parseBuf, "%s", ho_ptr->cmdparm[i].paramValue);
			return 0;
		}
	}
	return -1;
}

//get seq
int st_http_getSeq(const char *http_head)
{
	char seq[128];
	char *pest = NULL;                //((void *)0);
	int i = 0;

	memset(seq, 0, sizeof(seq));

	pest = strstr(http_head, "Seq:");
	if (pest == NULL)
		return 0;

	pest = strstr(pest, ":");
	pest += 1;
	while (*pest != '\r')
	{
		if (*pest != ' ' && *pest != '	')
			seq[i] = *pest;
		pest++;
		i++;
	}

	return atoi(seq);
}

int st_http_getHead(const char *recv_buf, char *http_head, char *http_body)
{
	char *pest = NULL;
	char tmp_ptr[HTTP_XML_BUF_SIZE];
	memset(tmp_ptr, 0, sizeof(tmp_ptr));

	int len = strlen(recv_buf);
	if (len < sizeof(tmp_ptr))
		strcpy(tmp_ptr, recv_buf);
	else
		memcpy(tmp_ptr, recv_buf, sizeof(recv_buf));

	pest = strstr(tmp_ptr, "\r\n\r\n");
	pest += 4;
	strncpy(http_head, tmp_ptr, pest - tmp_ptr);
	if (*pest != '\0')
		strcpy(http_body, pest);

	return strlen(http_head);
}

char *st_http_getLine(const char *http_head, char *line)
{
	char tmp_buf[1024];
	unsigned char b_find = 0;
	int i = 0;

//	assert (http_head != NULL);
	memset(tmp_buf, 0, 1024);
	while (b_find == 0 && (*http_head) != '\0')
	{
		switch (*http_head)
		{
		case '\r':
			break;
		case '\n':
			b_find = 1;		//find one line
			break;
		default:
			sprintf(tmp_buf, "%c", *http_head);
			strcat(line, tmp_buf);
			i++;
			break;
		}
		http_head++;
	}
	line[i + 1] = '\0';
	return line;
}

char *st_http_getMethod(const char *line, char *method)
{
//	assert (line != NULL);
	char line_tmp[1024]; 
	char *token = NULL;

	memset (line_tmp, 0, sizeof(line_tmp));
	memcpy (line_tmp, line, sizeof(line_tmp));
	token = strtok ((char *)line_tmp, " ");
	strcpy (method, token);

	return method;
}


char *st_http_getURL(const char *line, char *pURL)
{
	//assert (line != NULL);
	char line_tmp[1024];
	char *token = NULL;
	char *pest = NULL;

	memset(line_tmp, 0, sizeof(line_tmp));
	memcpy(line_tmp, line, sizeof(line_tmp));
	pest = strstr(line_tmp, "/");

	token = strtok(pest, " ");
	strcpy(pURL, token);

	return pURL;
}

char *st_http_getVersion(const char *line, char *version)
{
	//assert (line != NULL);
	char line_tmp[1024];
	char *token = NULL;
	char *pest = NULL;

	memset(line_tmp, 0, sizeof(line_tmp));
	memcpy(line_tmp, line, sizeof(line_tmp));
	pest = strstr(line_tmp, "/");
	pest += line_tmp - pest + 1;

	token = strtok(pest, " ");
	while (token != NULL)
	{
		token = strtok(NULL, " ");
	}
	strcpy(version, token);

	return version;
}

int st_http_getBodyLength(const char *http_head)
{
	char len[128];
	char *pest = NULL;
	int i = 0;

	memset(len, 0, sizeof(len));

	pest = strstr(http_head, "Content-Length:");
	if (pest == NULL)
		return 0;

	pest = strstr(pest, ":");
	pest += 1;
	while (*pest != '\r')
	{
		if (*pest != ' ' && *pest != '	')
			len[i] = *pest;
		pest++;
		i++;
	}

	return atoi(len);
}

// 下面是新增加的扫描函数

#define MAX_HTTP_PACKAGE_LEN  128*1024//256*1024
int st_http_strcmp(const char*first, const char* last, const char* str)
{
//	assert(first != NULL && last != NULL && str != NULL);
	if (last - first != strlen(str))
	return -1;
	while (first != last)
	{
		if (*str != '?' && *first != *str)
		return -1;
		++first;
		++str;
	}
	return 0;
}

#if 0
int st_http_scanHttpMsg(const char *recv_buf)
{
	//char http_msg[MAX_HTTP_PACKAGE_LEN];
	char* http_msg;
	int paired;
	char *to_find;
	char *cur = NULL;		//added by zhusy
	//	char *cur = http_msg;
	const char*pos = recv_buf;

	http_msg = (char*) malloc(sizeof(char) * MAX_HTTP_PACKAGE_LEN);
	if (NULL == http_msg)
	{
		return -1;
	}
	memset(http_msg, 0, sizeof(char) * MAX_HTTP_PACKAGE_LEN);
	cur = http_msg;	//

	//memset(http_msg, 0, MAX_HTTP_PACKAGE_LEN);
	/////////////////////////////////////
	while (*cur++ = *pos++)
	{
		if (cur == http_msg)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}
	}
	cur = http_msg;
	while (*cur == ' ')
		++cur;
	switch (*cur)
	{
	case 'G':
	{
		if (st_http_strcmp(cur, cur + 5, "GET /") != 0)	//"GET_/"
			return -1;
		cur += 5;
		to_find = strstr(cur, "\r\n");	//char *to_find;
		if (to_find == NULL || st_http_strcmp(to_find - 8, to_find, "HTTP/?.?") != 0)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}

		to_find = strstr(cur, "?");
		if (to_find == NULL || to_find == cur)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}
		cur = to_find + 1;
		to_find = strstr(cur, "=");
		if (to_find == NULL || to_find == cur)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}

		cur = to_find + 1;
		to_find = strstr(cur, "&");
		if (to_find != NULL)
		{
			paired = 1;	//int paired = 1;
			while (*to_find != '\n')
			{
				if (*to_find == '&')
				{
					if (paired == 0)
					{
						if (NULL != http_msg)
						{
							free(http_msg);
							http_msg = NULL;
						}
						return -1;
					}

					paired = 0;
				}
				else if (*to_find == '=')
				{
					if (paired == 1 || *(to_find - 1) == '&')
					{
						if (NULL != http_msg)
						{
							free(http_msg);
							http_msg = NULL;
						}
						return -1;
					}
					paired = 1;
				}
				else if (*to_find == '\0')
				{
					if (NULL != http_msg)
					{
						free(http_msg);
						http_msg = NULL;
					}
					return -1;
				}
				++to_find;
			}
			if (paired == 0)
			{
				if (NULL != http_msg)
				{
					free(http_msg);
					http_msg = NULL;
				}
				return -1;
			}

		}
		to_find = strstr(cur, "\r\n");
		while (to_find != NULL)
		{
			cur = to_find + 2;
			to_find = strstr(cur, "\r\n");
			if (to_find != NULL && to_find != cur)
			{
				while (*cur != ':' && cur != to_find)
					++cur;
				if (cur == to_find)
				{
					if (NULL != http_msg)
					{
						free(http_msg);
						http_msg = NULL;
					}
					return -1;
				}

			}
			else
				break;
		}
		break; //Case 'G'
	}

	case 'P':
	{
		if (st_http_strcmp(cur, cur + 6, "POST /") != 0) //"POST_/"
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}

		cur += 6;
		to_find = strstr(cur, "\r\n"); //char *to_find = strstr(cur,"\r\n");
		if (to_find == NULL || st_http_strcmp(to_find - 8, to_find, "HTTP/?.?") != 0)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}

		if (*cur == ' ')
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}
		to_find = strstr(cur, "?");
		if (to_find != NULL)
		{
			cur = to_find + 1;
			to_find = strstr(cur, "=");
			if (to_find == NULL || to_find == cur)
			{
				if (NULL != http_msg)
				{
					free(http_msg);
					http_msg = NULL;
				}
				return -1;
			}
			cur = to_find + 1;
			to_find = strstr(cur, "&");
			if (to_find != NULL)
			{
				int paired = 1;
				while (*to_find != '\n')
				{
					if (*to_find == '&')
					{
						if (paired == 0)
						{
							if (NULL != http_msg)
							{
								free(http_msg);
								http_msg = NULL;
							}
							return -1;
						}
						paired = 0;
					}
					else if (*to_find == '=')
					{
						if (paired == 1 || *(to_find - 1) == '&')
						{
							if (NULL != http_msg)
							{
								free(http_msg);
								http_msg = NULL;
							}
							return -1;
						}
						paired = 1;
					}
					else if (*to_find == '\0')
					{
						if (NULL != http_msg)
						{
							free(http_msg);
							http_msg = NULL;
						}
						return -1;
					}
					++to_find;
				}
				if (paired == 0)
				{
					if (NULL != http_msg)
					{
						free(http_msg);
						http_msg = NULL;
					}
					return -1;
				}
			}
		}
		to_find = strstr(cur, "\r\n");
		while (to_find != NULL)
		{
			cur = to_find + 2;
			to_find = strstr(cur, "\r\n");
			if (to_find != NULL && to_find != cur)
			{
				while (*cur != ':' && cur != to_find)
					++cur;
				if (cur == to_find)
				{
					if (NULL != http_msg)
					{
						free(http_msg);
						http_msg = NULL;
					}
					return -1;
				}
			}
			else
				break;
		}
		to_find = strstr(http_msg, "\r\n\r\n");
		if (to_find == NULL)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}

		cur = to_find + 4;
		to_find = strstr(cur, "=");
		if (to_find == NULL || to_find == cur)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}
		to_find = strstr(cur, "&");
		if (to_find != NULL)
		{
			paired = 1;
			cur = to_find + 1;
			while (*to_find != '\n')
			{
				if (*to_find == '&')
				{
					if (paired == 0)
					{
						if (NULL != http_msg)
						{
							free(http_msg);
							http_msg = NULL;
						}
						return -1;
					}
					paired = 0;
				}
				else if (*to_find == '=')
				{
					if (paired == 1)
						return -1;
					paired = 1;
				}
				else if (*to_find == '\0')
					break;
				++to_find;
			}
			if (paired == 0)
			{
				if (NULL != http_msg)
				{
					free(http_msg);
					http_msg = NULL;
				}
				return -1;
			}
		}
		break; //Case 'P'
	}

	case 'H':
	{
		if (st_http_strcmp(cur, cur + 9, "HTTP/?.? ") != 0) //"HTTP/1.1_"
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}

		to_find = strstr(cur, "\r\n"); //	char *to_find = strstr(cur, "\r\n");
		if (to_find == NULL)
		{
			if (NULL != http_msg)
			{
				free(http_msg);
				http_msg = NULL;
			}
			return -1;
		}
		while (to_find != NULL)
		{
			cur = to_find + 2;
			to_find = strstr(cur, "\r\n");
			if (to_find != NULL && to_find != cur)
			{
				while (*cur != ':' && cur != to_find)
					++cur;
				if (cur == to_find)
				{
					if (NULL != http_msg)
					{
						free(http_msg);
						http_msg = NULL;
					}
					return -1;
				}
			}
			else
				break;
		}
		break; //Case 'H'
	}

	default:
	{
		if (NULL != http_msg)
		{
			free(http_msg);
			http_msg = NULL;
		}
		return -1;
	}
	}//switch
	if (NULL != http_msg)
	{
		free(http_msg);
		http_msg = NULL;
	}	
	return 0;
}
#else
//PC_NET_LIBRARY
//============================================================//
int st_http_scanHttpMsg(const char *recv_buf)
{
	char http_msg[MAX_HTTP_PACKAGE_LEN];
	char *cur = http_msg;
	const char*pos = recv_buf;
	char *to_find = NULL;
	memset(http_msg, 0, MAX_HTTP_PACKAGE_LEN);
	while ((*cur++ = *pos++))
	{
		if (cur == &http_msg[MAX_HTTP_PACKAGE_LEN])
		return -1;
	}
	cur = http_msg;
	while (*cur == ' ')
		++cur;
	switch (*cur)
	{
		case 'G':
		{
			if (st_http_strcmp(cur, cur + 5, "GET /") != 0) //"GET_/"
			return -1;
			cur += 5;
			to_find = strstr(cur, "\r\n");
			if (to_find == NULL || st_http_strcmp(to_find - 8, to_find, "HTTP/?.?") != 0)
			return -1;
			to_find = strstr(cur, "?");
			if (to_find == NULL || to_find == cur)
			return -1;
			cur = to_find + 1;
			to_find = strstr(cur, "=");
			if (to_find == NULL || to_find == cur)
			return -1;
			cur = to_find + 1;
			to_find = strstr(cur, "&");
			if (to_find != NULL)
			{
				int paired = 1;
				while (*to_find != '\n')
				{
					if (*to_find == '&')
					{
						if (paired == 0)
						return -1;
						paired = 0;
					}
					else if (*to_find == '=')
					{
						if (paired == 1 || *(to_find - 1) == '&')
						return -1;
						paired = 1;
					}
					else if (*to_find == '\0')
					return -1;
					++to_find;
				}
				if (paired == 0)
				return -1;
			}
			to_find = strstr(cur, "\r\n");
			while (to_find != NULL)
			{
				cur = to_find + 2;
				to_find = strstr(cur, "\r\n");
				if (to_find != NULL && to_find != cur)
				{
					while (*cur != ':' && cur != to_find)
					++cur;
					if (cur == to_find)
					return -1;
				}
				else
				break;
			}
			break; //Case 'G'
		}

		case 'P':
		{
			if (st_http_strcmp(cur, cur + 6, "POST /") != 0) //"POST_/"
			return -1;
			cur += 6;
			to_find = strstr(cur, "\r\n");
			if (to_find == NULL || st_http_strcmp(to_find - 8, to_find, "HTTP/?.?") != 0)
			return -1;
			if (*cur == ' ')
			return -1;
			to_find = strstr(cur, "?");
			if (to_find != NULL)
			{
				cur = to_find + 1;
				to_find = strstr(cur, "=");
				if (to_find == NULL || to_find == cur)
				return -1;
				cur = to_find + 1;
				to_find = strstr(cur, "&");
				if (to_find != NULL)
				{
					int paired = 1;
					while (*to_find != '\n')
					{
						if (*to_find == '&')
						{
							if (paired == 0)
							return -1;
							paired = 0;
						}
						else if (*to_find == '=')
						{
							if (paired == 1 || *(to_find - 1) == '&')
							return -1;
							paired = 1;
						}
						else if (*to_find == '\0')
						return -1;
						++to_find;
					}
					if (paired == 0)
					return -1;
				}
			}
			to_find = strstr(cur, "\r\n");
			while (to_find != NULL)
			{
				cur = to_find + 2;
				to_find = strstr(cur, "\r\n");
				if (to_find != NULL && to_find != cur)
				{
					while (*cur != ':' && cur != to_find)
					++cur;
					if (cur == to_find)
					return -1;
				}
				else
				break;
			}
			to_find = strstr(http_msg, "\r\n\r\n");
			if (to_find == NULL)
			return -1;
			cur = to_find + 4;
			to_find = strstr(cur, "=");
			if (to_find == NULL || to_find == cur)
			return -1;
			to_find = strstr(cur, "&");
			if (to_find != NULL)
			{
				int paired = 1;
				cur = to_find + 1;
				while (*to_find != '\n')
				{
					if (*to_find == '&')
					{
						if (paired == 0)
						return -1;
						paired = 0;
					}
					else if (*to_find == '=')
					{
						if (paired == 1)
						return -1;
						paired = 1;
					}
					else if (*to_find == '\0')
					break;
					++to_find;
				}
				if (paired == 0)
				return -1;
			}
			break; //Case 'P'
		}

		case 'H':
		{
			if (st_http_strcmp(cur, cur + 9, "HTTP/?.? ") != 0) //"HTTP/1.1_"
			return -1;
			to_find = strstr(cur, "\r\n");
			if (to_find == NULL)
			return -1;
			while (to_find != NULL)
			{
				cur = to_find + 2;
				to_find = strstr(cur, "\r\n");
				if (to_find != NULL && to_find != cur)
				{
					while (*cur != ':' && cur != to_find)
					++cur;
					if (cur == to_find)
					return -1;
				}
				else
				break;
			}
			break; //Case 'H'
		}

		default:
		return -1;
	}
	return 0;
}
#endif
