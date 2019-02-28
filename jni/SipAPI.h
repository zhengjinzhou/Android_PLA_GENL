
/*
 * 版权所有：2007 深圳三立视讯有限公司(santachi)
 * 文 件 名：sip.h
 * 摘    要：SIP解码函数头文件
 * 创建日期：2007-09-13
 * 作    者: 张宇 (zhangy@gerd.com)
 * 修    改: 姓名zhangy	日期07.12.01	
 *                  修改内容描述: 
 *	
 * answerNum (sip_cmd_t) :
 * 		
 *	Informational	=	"100"	; 	Trying
 *				   	"180"	;	Ringing
 *					"181"	;	Call Is Being Forwarded
 *					"182"	;	Queued
 *					"183"	;	Session Progress
 *
 *	Success		=	"200"	;	OK
 *
 *	Redirection	=	"300"	;	Multiple Choices	;重定向
 *					"301"	;	Moved Permanently
 *					"302"	;	Moved Temporarily
 *					"305"	;	Use Proxy
 *					"380"	;	Alternative Service
 *
 *	Client-Error	=	"400"	;	Bad Request
 *					"401"	;	Unauthorized
 *					"402"	;	Payment Required
 *					"403"	;	Forbidden
 *					"404"	;	Not Found
 *					"405"	;	Method Not Allowed
 *					"406"	;	Not Acceptable
 *					"407"	;	Proxy Authentication Required
 *					"408"	;	Request Timeout
 *					"409"	;	Conflict
 *					"410"	;	Gone
 *					"411"	;	Length Required
 *					"413"	;	Request Entity Too Large
 *					"414"	;	Request-URI Too Large
 *					"415"	;	Unsupported Media Type
 *					"420"	;	Bad Extension
 *					"480"	;	Temporarily not available
 *					"481"	;	Call Leg/Transaction Does Not Exist
 *					"482"	;	Loop Detected
 *					"483"	;	Too Many Hops
 *					"484"	;	Address Incomplete
 *					"485"	;	Ambiguous
 *					"486"	;	Busy Here
 *					"487"	;	Request Cancelled
 *					"488"	;	Not Acceptable Here
 *
 *	Sever-Error	=	"500"	;	Internal Serve Error
 *					"501"	;	Not Implemented
 *					"502"	;	Bad Gateway
 *					"503"	;	Service Unavailable
 *					"504"	;	Gateway Time-out
 *					"505"	;	SIP Version not supported
 *
 *	Global-Failure	=	"600"	;	Busy Everywhere
 *					"603"	;	Decline
 *					"604"	;	Does not exist any where
 *					"606"	;	Not Acceptable
 *
 */


#ifndef SIPAPI_H
#define SIPAPI_H
//#define __SIP_API_EXP_ 

 #ifdef __cplusplus
 extern "C"{
 #endif


#define		MAX_XML_SIZE		4096
#define		MAX_SIP_SIZE		1024
#define		MAX_MSG_SIZE		MAX_XML_SIZE+MAX_SIP_SIZE
#define		MAX_LINE_SIZE		128

typedef	struct	__sip_cmd_t
{
	char		cmdName[48];
	int		answerNum;		/*100, 180, 200 ...*/
	char		header[MAX_LINE_SIZE];
	char		to[MAX_LINE_SIZE];
	char 	from[MAX_LINE_SIZE];
	char		callId[64];
	char		cSeq[32];
	char		contentType[16];
	int		maxForwards;
	int		expires;
	int		contentLength;
}sip_cmd_t;

 /////////////////SIP_APT 
int    st_sip_recvAll(int fd, char* msg, int timet);
void   st_sip_parseSip(char *msg, int msgLen, sip_cmd_t *sip_cmd, char *xmlBody);
void   st_sip_setSip(char *sipCmd, char *cmd, char *head, char *to, char *from, char *cseq, char *callId, char *contentType, int contentLength);
void   st_sip_answerRequest(char *answer, int answerNum, sip_cmd_t *sip_cmd, int xmlLen);
void   st_sip_makeMsg(char *msg, char *sipCmd, char *msgBody);
int	 st_sip_parseFrom(char *from, char *id, char *ip, int *port);
int    st_sip_readn(int fd, void *buf, int count, int timet);
int	 st_sip_writen(int fd, void *buf, int count, int timet);
void   st_sip_printSip(sip_cmd_t *sip_cmd);

#ifdef __cplusplus
 }
#endif /* __cplusplus */
 
 
 
 #endif



