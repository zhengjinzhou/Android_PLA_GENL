/*
 * 版权所有：2007 深圳三立视讯有限公司(santachi)
 * 文 件 名：sip.c
 * 摘    要：SIP协议解码函数
 * 创建日期：2007-09-13
 * 作    者: 张宇 (zhangy@gerd.com)
 * 修    改: 姓名zhangy	日期07.11.30 	
 */


//#include "stdafx.h"
//#include "DecodeSip.h"

//#define		MAX_XML_SIZE		4096
//#define		MAX_SIP_SIZE		1024
//#define		MAX_MSG_SIZE		MAX_XML_SIZE+MAX_SIP_SIZE

//#define __SIP_API_EXP_
#include "SipAPI.h"
#include <string.h>
#include <stdio.h>

#define message_r printf

static	void st_sip_parseSipHeadline(const char *headline, char *cmd, char *value);
static 	void st_sip_parseCmdLine(const char* cmdLine, char *name, char *value);
static	int	 st_sip_setSipHeader(char *sipHeaderline, const char *cmd, char *header);
static	void st_sip_setSipGeneral(char *General, char *to, char *from, char *cseq, char *callId, char *contentType, int contentLength);

 int   st_sip_recvAll(int fd, char* msg, int timet)//SIP_LIB::
{
	//assert((fd > 0) && (msg != NULL));
/*	int	i, j, m = 0, k = 0;
	int	nread;
	int	sipLen = 0;
	int	xmlLen = 0;
	char	name[32];
	char	value[96];
	char	msgbuf[MAX_MSG_SIZE];
	char	xml[MAX_XML_SIZE];
	char line[20][MAX_LINE_SIZE];
	char	contentLength[16];

	memset(line, 0, sizeof(line));
	memset(msgbuf, 0, sizeof(msgbuf));
	for(i = 0; i < 20; i++)
	{
		for(j = 0; j < MAX_LINE_SIZE; j++)	
		{	
			//这里的是读写 READ
			//nread = recv(fd, &line[i][j],sizeof(msgbuf),timet);//
			nread = read(fd, &line[i][j], 1);//yuanban 
			msgbuf[m] = line[i][j];
			m++;
			if(nread <= 0)
			{
				//perror("st_sip_recvAll(), read()");
				return	nread;
			}

			sipLen += nread;

			if((line[i][j] == '\r')||(line[i][j] == '\n'))
				k++;
			else
				k = 0;

			if(line[i][j] == '\n')
				break;
		}

		if(k == 4)
			break;
	}
	msgbuf[m] = '\0';

	//parse sip to get content-Length
	for(j = 1; j < i; j++)
	{
		memset(name, 0, sizeof(name));
		memset(value, 0, sizeof(value));
		
		st_sip_parseCmdLine(line[j], name, value);
		if(0 == strcmp(name, "Content-Length"))
		{
			strncpy(contentLength, value, sizeof(contentLength));
			xmlLen = atoi(contentLength);
			break;
		}
	}
	
	//read xml 
	nread = 0;
	if(xmlLen > 0)
	{
		memset(xml, 0, sizeof(xml));
		
		nread = st_sip_readn(fd, xml, xmlLen, timet);
		if(nread == 0)
		{
			//perror("There has no xml data to read!");
			return	nread;
		}
		else if(nread < 0)
		{
			//perror("st_sip_recvAll(), st_sip_readn()");
			return	nread;
		}

		strcat(msgbuf, xml);
	}

	if((sipLen > MAX_SIP_SIZE) || (xmlLen > MAX_XML_SIZE))
	{
		printf("st_sip_recvAll: message too long(%d) !!!", sipLen+xmlLen);
		return	-1;
	}

	strcpy(msg, msgbuf);
	return	sipLen+nread;
	*/
    return 0;
}
//
void  st_sip_parseSip(char *msg, int msgLen, sip_cmd_t *sip_cmd, char *xmlBody)
{
	int	i, j;
	int	xmlLen = 0;
	//char	buf[MAX_MSG_SIZE];
	char	line[20][MAX_LINE_SIZE];
	//char	xmlbuf[MAX_XML_SIZE];
	char	cmd[48];
	char	name[32];
	char	value[96];
	char	answerNum[16];
	char	*p1, *p2, *pxml;
	char * buf;
	char * xmlbuf;

	memset(line, 0, sizeof(line));
	memset(cmd, 0, sizeof(cmd));
	//memset(buf, 0, sizeof(buf));

	//modified by zhusy
	buf = (char *)malloc(MAX_MSG_SIZE);
	memset(buf, 0, MAX_MSG_SIZE);
	memcpy(buf, msg, msgLen);

	xmlbuf = (char*)malloc(MAX_XML_SIZE);

	p1 = p2 = buf;
	for(i = 0; i < 20; i++)
	{
		p2 = strchr(p1, '\n');
		if(p2 == NULL)
		{
			//perror("st_sip_parseSip(), Can not find the end of the line !");
			if (NULL != buf)
			{
				free(buf);
				buf = NULL;
			}
			
			if (NULL != xmlbuf)
			{
				free(xmlbuf);
				xmlbuf = NULL;
			}
			return;
		}
		memcpy(line[i], p1, p2 - p1 +1);
		
		if(0 == strcmp(line[i], "\r\n"))
			break;

		p1 = p2 + 1;
	}
	j = i;
	pxml = p2 + 1;

	p1 = strchr(line[0], ' ');
	if(p1 == NULL)
	{
		//perror("st_sip_parseSip(), There should be a ' ' !");
		if (NULL != buf)
		{
			free(buf);
			buf = NULL;
		}

		if (NULL != xmlbuf)
		{
			free(xmlbuf);
			xmlbuf = NULL;
		}
		return;
	}
	memcpy(cmd, line[0], p1 - line[0]);
	
	if(strcmp(cmd, "SIP/2.0") == 0)
	{
		st_sip_parseSipHeadline(line[0], answerNum, sip_cmd->cmdName);
		sip_cmd->answerNum = atoi(answerNum);
	}
	else	
		st_sip_parseSipHeadline(line[0], sip_cmd->cmdName, sip_cmd->header);	
	
	//parse sip body
	for(i = 1; i < j; i++)
	{
		memset(name, 0, sizeof(name));
		memset(value, 0, sizeof(value));
		
		st_sip_parseCmdLine(line[i], name, value);
		if(0 == strcmp(name, "To"))
			strcpy(sip_cmd->to, value);
		else if(0 == strcmp(name, "From"))
			strcpy(sip_cmd->from, value);
		else if(0 == strcmp(name, "CSeq"))
			strncpy(sip_cmd->cSeq, value, sizeof(sip_cmd->cSeq));
		else if((0 == strcmp(name, "Call-ID"))||(0 == strcmp(name, "Callid")))
			strncpy(sip_cmd->callId, value, sizeof(sip_cmd->callId));
		else if(0 == strcmp(name, "Content-Type"))
			strncpy(sip_cmd->contentType, value, sizeof(sip_cmd->contentType));
		else if(0 == strcmp(name, "Content-Length"))
			xmlLen = sip_cmd->contentLength = atoi(value);		
		else if(0 == strcmp(name, "Max-Forwards"))
			sip_cmd->maxForwards = atoi(value);
		else if(0 == strcmp(name, "Expires"))
			sip_cmd->expires = atoi(value);
		
	}

	//read xml 
	//memset(xmlbuf, 0, sizeof(xmlbuf));//deleted by zhusy
    memset(xmlbuf, 0, MAX_XML_SIZE);
	memcpy(xmlbuf, pxml, xmlLen);
	if(xmlBody != NULL)//NULL
		strcpy(xmlBody, xmlbuf);

	//added by zhusy
	if (NULL != buf)
	{
		free(buf);
		buf = NULL;
	}

	if (NULL != xmlbuf)
	{
		free(xmlbuf);
		xmlbuf = NULL;
	}
	//added by zhusy end!
}
//
int  st_sip_parseFrom(char *from, char *id, char *ip, int *port)
{	
	//assert(from != NULL);
	int	n;	
	char	frombuf[MAX_LINE_SIZE] = {0};	
	char	idbuf[32] = {0};	
	char	ipbuf[16] = {0};
	char	portbuf[16] = {0};
	char	*p1, *p2;

	strncpy(frombuf, from, MAX_LINE_SIZE);
	n = strlen(frombuf);
	
	p1 = strchr(frombuf, '<');
	if(p1 == 0)
	{
		//only id
		strncpy(idbuf, frombuf, sizeof(idbuf));
		if(id != NULL)
			strcpy(id, idbuf);
		
		return	0;	
	}
	
	memcpy(idbuf, frombuf, p1 - frombuf);
	if(id != NULL)
		strncpy(id, idbuf, 32);
		
	p1 = strchr(frombuf, ':');
	if(p1 == NULL)
	{
		//perror("there should be a ':'");
		return	-1;
	}
	
	p2 = strchr(p1+1, ':');
	if(p2 == NULL)
	{
		//no port
		p2 = strchr(p1+1, '>');
		if(p2 == NULL)
		{
			//perror("there should be a '>'");
			return	-1;
		}

		memcpy(ipbuf, p1+1, p2-p1-1);
		if(ip != NULL)
			strncpy(ip, ipbuf, 16);

		return	0;
	}

	memcpy(ipbuf, p1+1, p2-p1-1);
	if(ip != NULL)
		strncpy(ip, ipbuf, 16);

	p1 = strchr(p2, '>');
	if(p1 == NULL)
	{
		//perror("there should be a '>'");
		return	-1;	
	}
	
	memcpy(portbuf, p2+1, p1-p2-1);
	if(port != NULL)
		*port = atoi(portbuf);	
	
	return	0;
}
//
 void  st_sip_makeMsg(char *msg, char *sipCmd, char *msgBody)
{
//	char	buf[MAX_MSG_SIZE] = {0};
	//modified by zhusy
	char    *buf ;
	buf = (char*)malloc(sizeof(char) * MAX_MSG_SIZE);
	memset(buf , 0,  MAX_MSG_SIZE);

	strcpy(buf, sipCmd);
	
	if(msgBody != NULL)
		strcat(buf, msgBody);

	strcpy(msg, buf);

	//added by zhusy
	if ( NULL != buf)
	{
		free(buf);
		buf = NULL;
	}
	
}
//
/******************************************************************************
 * function: set the whole sip command to sipCmd.
 *
 ******************************************************************************/
void st_sip_setSip(char *sipCmd, char *cmd, char *head, char *to, char *from, char *cseq, char *callId, char *contentType, int contentLength)
{
	//assert(sipCmd != NULL);
	char	buf[MAX_SIP_SIZE] = {0};
	char	sipheader[MAX_LINE_SIZE] = {0};
	char	General[MAX_SIP_SIZE] = {0};
	
	if(0 != st_sip_setSipHeader(sipheader, cmd, head))
	{
		//perror("st_sip_setSipHeader()");
		return;
	}
	strcpy(buf, sipheader);
	
	st_sip_setSipGeneral(General, to, from, cseq, callId, contentType, contentLength);
	strcat(buf, General);
	
	strcpy(sipCmd, buf);	
}
//
 void  st_sip_answerRequest(char *answer, int answerNum, sip_cmd_t *sip_cmd, int xmlLen)
{
	int		len = 0;
	char		General[MAX_SIP_SIZE] = {0};
	char		answerbuf[MAX_MSG_SIZE] = {0};

	if(answerNum < 100)
	{
		//perror("answerNum Wrong !");
		return;
	}
	
	if(sip_cmd == NULL)
	{
		//perror("sip_cmd == NULL");
		return;
	}
	
	if(xmlLen > 0)
		len = xmlLen;

//	memset(answerbuf, 0, sizeof(answerbuf));
	if(answerNum == 200)
		strcpy(answerbuf, "SIP/2.0 200 OK\r\n");
	//Informational
	else if(answerNum == 100)
		strcpy(answerbuf, "SIP/2.0 100 Trying\r\n");
	else if(answerNum == 180)
		strcpy(answerbuf, "SIP/2.0 180 Ringing\r\n");
	else if(answerNum == 181)
		strcpy(answerbuf, "SIP/2.0 181 Call Is Being Forwarded\r\n");
	else if(answerNum == 182)
		strcpy(answerbuf, "SIP/2.0 182 Queued\r\n");
	else if(answerNum == 183)
		strcpy(answerbuf, "SIP/2.0 183 Session Progress\r\n");
	//Redirection
	else if(answerNum == 300)
		strcpy(answerbuf, "SIP/2.0 300 Multiple Choices\r\n");
	else if(answerNum == 301)
		strcpy(answerbuf, "SIP/2.0 301 Moved Permanently\r\n");
	else if(answerNum == 302)
		strcpy(answerbuf, "SIP/2.0 302 Moved Temporarily\r\n");
	else if(answerNum == 305)
		strcpy(answerbuf, "SIP/2.0 305 Use Proxy\r\n");
	else if(answerNum == 380)
		strcpy(answerbuf, "SIP/2.0 380 Alternative Service\r\n");
	//Client-Error
	else if(answerNum == 400)
		strcpy(answerbuf, "SIP/2.0 400 Bad Request\r\n");
	else if(answerNum == 401)
		strcpy(answerbuf, "SIP/2.0 401 Unauthorized\r\n");
	else if(answerNum == 402)
		strcpy(answerbuf, "SIP/2.0 402 Payment Required\r\n");
	else if(answerNum == 403)
		strcpy(answerbuf, "SIP/2.0 403 Forbidden\r\n");
	else if(answerNum == 404)
		strcpy(answerbuf, "SIP/2.0 404 Not Found\r\n");
	else if(answerNum == 405)
		strcpy(answerbuf, "SIP/2.0 405 Method Not Allowed\r\n");
	else if(answerNum == 406)
		strcpy(answerbuf, "SIP/2.0 406 Not Acceptable\r\n");
	else if(answerNum == 407)
		strcpy(answerbuf, "SIP/2.0 407 Proxy Authentication Required\r\n");
	else if(answerNum == 408)
		strcpy(answerbuf, "SIP/2.0 408 Request Timeout\r\n");
	else if(answerNum == 409)
		strcpy(answerbuf, "SIP/2.0 409 Conflict\r\n");
	else if(answerNum == 410)
		strcpy(answerbuf, "SIP/2.0 410 Gone\r\n");
	else if(answerNum == 411)
		strcpy(answerbuf, "SIP/2.0 411 Length Required\r\n");
	else if(answerNum == 413)
		strcpy(answerbuf, "SIP/2.0 413 Request Entity Too Large\r\n");
	else if(answerNum == 414)
		strcpy(answerbuf, "SIP/2.0 414 Request-URI Too Large\r\n");
	else if(answerNum == 415)
		strcpy(answerbuf, "SIP/2.0 415 Unsupported Media Type\r\n");
	else if(answerNum == 420)
		strcpy(answerbuf, "SIP/2.0 420 Bad Extension\r\n");
	else if(answerNum == 480)
		strcpy(answerbuf, "SIP/2.0 480 Temporarily not available\r\n");
	else if(answerNum == 481)
		strcpy(answerbuf, "SIP/2.0 481 Call Leg/Transaction Does Not Exist\r\n");
	else if(answerNum == 482)
		strcpy(answerbuf, "SIP/2.0 482 Loop Detected\r\n");
	else if(answerNum == 483)
		strcpy(answerbuf, "SIP/2.0 483 Too Many Hops\r\n");
	else if(answerNum == 484)
		strcpy(answerbuf, "SIP/2.0 484 Address Incomplete\r\n");
	else if(answerNum == 485)
		strcpy(answerbuf, "SIP/2.0 485 Ambiguous\r\n");
	else if(answerNum == 486)
		strcpy(answerbuf, "SIP/2.0 486 Busy Here\r\n");
	else if(answerNum == 487)
		strcpy(answerbuf, "SIP/2.0 487 Request Cancelled\r\n");
	else if(answerNum == 488)
		strcpy(answerbuf, "SIP/2.0 488 Not Acceptable Here\r\n");
	//Sever-Error
	else if(answerNum == 500)
		strcpy(answerbuf, "SIP/2.0 500 Internal Serve Error\r\n");
	else if(answerNum == 501)
		strcpy(answerbuf, "SIP/2.0 501 Not Implemented\r\n");
	else if(answerNum == 502)
		strcpy(answerbuf, "SIP/2.0 502 Bad Gateway\r\n");
	else if(answerNum == 503)
		strcpy(answerbuf, "SIP/2.0 503 Service Unavailable\r\n");
	else if(answerNum == 504)
		strcpy(answerbuf, "SIP/2.0 504 Gateway Time-out\r\n");
	else if(answerNum == 505)
		strcpy(answerbuf, "SIP/2.0 505 SIP Version not supported\r\n");
	//Global-Failure
	else if(answerNum == 600)
		strcpy(answerbuf, "SIP/2.0 600 Busy Everywhere\r\n");
	else if(answerNum == 603)
		strcpy(answerbuf, "SIP/2.0 603 Decline\r\n");
	else if(answerNum == 604)
		strcpy(answerbuf, "SIP/2.0 604 Does not exist any where\r\n");
	else if(answerNum == 606)
		strcpy(answerbuf, "SIP/2.0 606 Not Acceptable\r\n");

// 	else
// 		perror("Wrong number !");
	
//	memset(General, 0, sizeof(General));
	st_sip_setSipGeneral(General, sip_cmd->to, sip_cmd->from,\
	 	sip_cmd->cSeq, sip_cmd->callId, sip_cmd->contentType, len);

	strcat(answerbuf, General);
	
	if(answer != NULL)
		strcpy(answer, answerbuf);
	
}
//
 int st_sip_readn(int fd, void *buf, int count, int timet)
{
	/*
	int	nleft;
	int	nread;
	char	*ptr;
	
	ptr = (char*)buf;
	nleft = count;
	
	while(nleft > 0)
	{
		if((nread = read(fd, ptr, nleft)) < 0)//yuanban
		//if((nread = recv(fd, ptr, nleft,timet)) < 0)
		{
			//perror("read()");
// 			if(errno == EINTR)
// 				nread = 0;
// 			else
// 				return	nread;
		}
		else if(nread == 0)
		{
			//perror("read()");
			break;
			
		}
		nleft -= nread;
		ptr += nread;
	}
	
	return	(count - nleft);
	*/
	return 0;
}
//
int	 st_sip_writen(int fd, void *buf, int count, int timet)
{
	/*int	nleft;
	int	nwriten;
	const char	*ptr;
	
	ptr = (char*)buf;
	nleft = count;
	
	while(nleft > 0)
	{
		if((nwriten = write(fd,ptr,nleft)) <= 0)//yuanban 
		//if ((nwriten = send(fd, ptr, nleft,timet)) <= 0)//if((nwriten = write(fd,ptr,nleft)) <= 0)
		{
		//	perror("write()");
 			//if(nwriten < 0 && errno == EINTR)
			//	nwriten = 0;
 			//else
 			//	return	nwriten;
		}
		
		nleft -= nwriten;
		ptr += nwriten;
	}
	
	
	return	count;	
	*/		
	return 0;
}
//
void   st_sip_printSip(sip_cmd_t *sip_cmd)
{
	
	if(sip_cmd != NULL)
	{
		if(sip_cmd->callId != NULL)
			message_r("sip_cmd->callId = |%s|\n", sip_cmd->callId);
		if(sip_cmd->cmdName != NULL)
			message_r("sip_cmd->cmdName = |%s|\n", sip_cmd->cmdName);
		
		message_r("sip_cmd->answerNum = |%d|\n", sip_cmd->answerNum);
		
		if(sip_cmd->header != NULL)
			message_r("sip_cmd->header = |%s|\n", sip_cmd->header);
		if(sip_cmd->to != NULL)
			message_r("sip_cmd->to = |%s|\n", sip_cmd->to);
		if(sip_cmd->from != NULL)
			message_r("sip_cmd->from = |%s|\n", sip_cmd->from);
		if(sip_cmd->cSeq != NULL)
			message_r("sip_cmd->cSeq = |%s|\n", sip_cmd->cSeq);
			
		message_r("sip_cmd->maxForwards = |%d|\n", sip_cmd->maxForwards);
		message_r("sip_cmd->expires = |%d|\n", sip_cmd->expires);
		message_r("sip_cmd->contentLength = |%d|\n", sip_cmd->contentLength);
	}
	else
		message_r("sip_cmd == NULL");

}
//
// 以下是静态函数
static void st_sip_parseSipHeadline(const char *headline, char *cmd, char *value)
{	
	//assert(headline != NULL);
	char	cmdbuf[48] = {0};
	char	head[MAX_LINE_SIZE] = {0};
	char	addrbuf[96] = {0};
	char	rebuf[96] = {0};
	char	*p1, *p2;

	strcpy(head, headline);

	p1 = strchr(head, ' ');
	if(p1 == NULL)
	{
		//perror("p1 == NULL");
		return;
	}
	memcpy(cmdbuf, head, p1 - head);

	p2 = strchr(p1 + 1, ' ');
	if(p2 == NULL)
	{
		//perror("p2 == NULL");
		return;
	}
	memcpy(addrbuf, p1 + 1, p2 - p1 - 1);

	p1 = strchr(p2 + 1, '\r');
	if(p1 == NULL)
	{
		//perror("p1 == NULL");
		return;
	}
	memcpy(rebuf, p2 + 1, p1 - p2 - 1);

	if(0 == strcmp(cmdbuf, "SIP/2.0"))
	{
		if(cmd != NULL)
			strncpy(cmd, addrbuf, 16);
		if(value != NULL)
			strncpy(value, rebuf, 48);
	}
	else
	{
		if(cmd != NULL)
			strcpy(cmd, cmdbuf);
		if(value != NULL)
			strcpy(value, addrbuf);
	}
	
}
//
static void st_sip_parseCmdLine(const char* cmdLine, char *name, char *value)
{	
	//assert(cmdLine != NULL);
	char	namebuf[32] = {0};
	char	line[MAX_LINE_SIZE] = {0};
	char	valuebuf[96] = {0};
	char	*p1, *p2;

	strcpy(line, cmdLine);

	p1 = strchr(line, ':');
	if(p1 == NULL)
	{
		//perror("There should be a ':'");
		return;
	}
	memcpy(namebuf, line, p1 - line);
	if(name != NULL)
		strcpy(name, namebuf);

	p2 = strchr(line, '\r');
	if(p2 == NULL)
	{
		//perror("Can not find the end of the line !");
		return;
	}
      	memcpy(valuebuf, p1 + 1, p2 - p1 - 1);
	if(value != NULL)
		strcpy(value, valuebuf);

}


//
static	void st_sip_setSipGeneral(char *General, char *to, char *from, char *cseq, char *callId, char *contentType, int contentLength)
{
	//assert(General != NULL);
	char	buf[MAX_SIP_SIZE] = {0};
	char	contlen[16] = {0};

	strcpy(buf, "To:");
	if(to != NULL)
		strcat(buf, to);
	strcat(buf, "\r\n");
	
	strcat(buf, "From:");
	if(from != NULL)
		strcat(buf, from);
	strcat(buf, "\r\n");

	strcat(buf, "CSeq:");	
	if(cseq != NULL)
		strcat(buf, cseq);
	strcat(buf, "\r\n");

	strcat(buf, "Call-ID:");
	if(callId != NULL)
		strcat(buf, callId);
	strcat(buf, "\r\n");
	
	strcat(buf, "Max-Forwards:70\r\n");
	
	strcat(buf, "Content-Type:");
	if(contentType != NULL)
		strcat(buf, contentType);
	strcat(buf, "\r\n");

	sprintf(contlen, "%d", contentLength);
	strcat(buf, "Content-Length:");
	strcat(buf, contlen);
	strcat(buf, "\r\n");
	strcat(buf, "\r\n");
	
	strcpy(General, buf);
}
//
/******************************************************************************
 * function: set the first line of sip command. if cmd is "OK", the parameters behind do nothing.
 *
 ******************************************************************************/
static	int st_sip_setSipHeader(char *sipHeaderline, const char *cmd, char *header)
{
	//assert((sipHeaderline != NULL) && (cmd != NULL));
	char	buf[MAX_LINE_SIZE] = {0};

	if(0 == strcmp(cmd, "OK"))
	{
		strcpy(sipHeaderline, "SIP/2.0 200 OK\r\n");
		return	0;
	}
	else
	{
		strcpy(buf, cmd);
		strcat(buf, " ");
	}
	
	if(header != NULL)
		strcat(buf, header);
	
	strcat(buf, " SIP/2.0\r\n");

	strncpy(sipHeaderline, buf, MAX_LINE_SIZE);

	return	0;
}
//

