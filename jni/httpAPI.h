/*
 * httpAPI.h
 */
#ifndef __HTTPAPI_H
#define __HTTPAPI_H

#ifdef __cplusplus
extern "C"
{
# endif /* __cplusplus */

#define MAX_PARAM_NUM    	16
#define REQ_TYPE_GET		0
#define REQ_TYPE_POST 		1
#define RESP_TYPE			2
#define HTTP_XML_BUF_SIZE	128*1024 //<==256*1024   LinZh107

typedef struct http_object_t
{
	int type;		// type
	int seq;
	char cmd[32];	// command
	char xml[HTTP_XML_BUF_SIZE];
	struct param
	{
		char paramName[256];
		char paramValue[256]; //<==1024   LinZh107
	} cmdparm[MAX_PARAM_NUM];
} HTTP_OBJECT;

//recv //extern "C" __declspec(dllexport)
int st_http_recvHttpPackage(int sock, char *recv_buf); //HTTP_APT

//pack interface
char *st_http_addParam(char *buffer, char *fmt, ...);

char * st_http_makeHttp(const char * type,	//输入参数，请求或响应
						unsigned int seq,	//输入参数, 命令序列号
						char *buffer,		//输出参数, 最后的返回值
						int *len,			//输出参数, 明文长度
					    char *cmd ,			//输入参数, 命令字串
						char *param_buf ,	//输入参数, http参数字串,回应时默认为空//char *param_buf=NULL,	
						char *xml_buf);	    //输入参数, 请求时默认为空//char *xml_buf=NULL

//parse interface
unsigned char st_http_parseHttp(const char *recv_buf, HTTP_OBJECT *ho_ptr);
int st_http_parseHttpParam(HTTP_OBJECT *ho_ptr, const char *szParam, char *parseBuf);
int recvPackage(int fd, char* buf, unsigned int nMax, long long msec);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
