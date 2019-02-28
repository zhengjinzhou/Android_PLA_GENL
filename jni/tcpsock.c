/*
 * tcpsock.c
 *
 *  Created on: 2009-10-10
 *      Author: pc
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>   		// basic system data types
#include <sys/socket.h>  		// basic socket definitions
#include <sys/time.h>    		// timeval{} for select()
#include <netinet/in.h>  		// sockaddr_in{} and other Internet defns
//#include <netinet/tcp.h>
#include <arpa/inet.h>   		// inet(3) functions
#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <android/log.h>

#include "tcpsock.h"
#include "httpAPI.h"
#include "netglobal.h"
#define  LOG_TAG "tcpsock.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static ssize_t writen(int fd, const void *vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		////my_log("write %d fd %d %s\n", nleft, fd, vptr);
		if ((nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (errno == EINTR)
				nwritten = 0; /* and call write() again */
			else
			{
				return (-1); /* error */
				////my_log("=================>writen error!\n");
			}
		}
		if (errno == 5)
			return (-1);
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n);
}
static int readn(int fd, char*buf, int nLen, struct timeval* timeout)
{
	fd_set fd_read;
	int ret = 0;
	int nRecv = 0;
	int nLeft = nLen;
	unsigned int pos = 0;
#if 0

#endif
	//int nReturn =0;
	while (nLeft > 0)
	{
		// 这里超时时间应该要递减
		//	ret = select( fd+1, &fd_read, NULL, NULL, timeout);
#if 0
		FD_ZERO(&fd_read);
		FD_SET(fd, &fd_read);
		//errno = 0;
		ret = select(fd + 1, &fd_read, NULL, NULL, timeout);//ret = select( fd, &fd_read, NULL, NULL, timeout);
		if (ret <= 0)
		{
			//			TRACE("=== readn: select return %d ===\n", ret);
			//nReturn = ret;  // SOCKET_ERROR或0(超时)
			//my_log("my_readn err %s\n", strerror(errno));
			//errno = 0;
			return -1;
		}
#endif
		nRecv = recv(fd, buf + pos, nLeft, 0);
		if (nRecv < 0)
		{
			if (errno == EINTR)
				nRecv = 0;
			else
			{
				//my_log("my_readn err %s\n", strerror(errno));
				return -1;
			}
		}
		else if (nRecv == 0)
		{
			return 0;
		}

		nLeft -= nRecv;
		pos += nRecv;
	}

	return pos;
}

static int close_socket_tcp(tcpsocket_t * handle)
{
	if (handle == NULL)
		return -1;

	if (handle->m_hSocket != -1)
	{
		close(handle->m_hSocket);
		handle->m_hSocket = -1;
	}

	pthread_mutex_destroy(&handle->m_lock);
	pthread_mutex_destroy(&handle->m_write_lock);

	free(handle);
	handle = NULL;

	return 0;
}

int close_p_socket_tcp(tcpsocket_t * handle)
{
	if (handle == NULL)
		return -1;

	if (handle->m_hSocket != -1)
	{
		close(handle->m_hSocket);
		handle->m_hSocket = -1;
	}

	pthread_mutex_destroy(&handle->m_lock);
	pthread_mutex_destroy(&handle->m_write_lock);

	return 0;
}

int set_tcp_link_tcp(tcpsocket_t *handle, const char* szTcpIp, int nPort)
{
	char *strIP;	//modified by zhusy
	struct hostent * hp;
	char szIP[128] = { 0 };
	int ret = 0;

	// fix the issue about no real video Date: 2013-9-25 Author: yms
	char verifyIP[128] = { 0 };
	strcpy(verifyIP, szTcpIp);
	static char tempIP[128] = { 0 };
	if (0 != strlen(verifyIP))
	{
		strcpy(tempIP, verifyIP);
	}

	if (handle == NULL)
	{
		//my_log("set_tcp_link_tcp handle is NULL\n");
		return -1;
	}
	pthread_mutex_lock(&handle->m_lock);
	if (inet_addr(tempIP/*szTcpIp*/) == INADDR_NONE)
	{
		if ((hp = gethostbyname(tempIP/*szTcpIp*/)) != NULL)
		{
			struct in_addr inAddr;
			char* lpAddr = hp->h_addr_list[0];

			if (lpAddr){
				memcpy(&inAddr, lpAddr, 4);
				strIP = inet_ntoa(inAddr);
				sprintf(szIP, "%s", strIP);
			}
		}
		else{
			ret = -1;
			goto a1;
		}
	}
	else
	{
		strcpy(szIP, tempIP/*szTcpIp*/);
	}
	//this->m_strServerIP.Format("%s", szIP);
	sprintf(handle->m_strServerIP, "%s", szIP);
	handle->m_nServerPort = nPort;

	a1: pthread_mutex_unlock(&handle->m_lock);

	return ret;
}

int get_tcp_link_tcp(tcpsocket_t *handle, char* lpszTcpIp, int *nPort)
{
	if (handle == NULL)
		return -1;

	pthread_mutex_lock(&handle->m_lock);
	strcpy(lpszTcpIp, handle->m_strServerIP);
	*nPort = handle->m_nServerPort;
	pthread_mutex_unlock(&handle->m_lock);
	return 0;
}

int m_connect_tcp(tcpsocket_t *handle, long long msec)
{
	//LOGI("In m_connect_tcp.");
	unsigned int error, len;
	char szTcpIp[128];
	int nPort;
	struct sockaddr_in serv;
	int res;
	fd_set fdWrite, fdread;
	struct timeval tv;
	int rett, ret = 0;
	int flags;

	if (handle == NULL)
	{
		return -1;
	}

	nPort = handle->m_nServerPort;

	memset(&serv, 0, sizeof(struct sockaddr_in));
	strcpy(szTcpIp, handle->m_strServerIP);
	serv.sin_family = PF_INET;
	serv.sin_addr.s_addr = inet_addr(szTcpIp);
	serv.sin_port = htons(nPort);

	flags = fcntl(handle->m_hSocket,F_GETFL,0);
	fcntl(handle->m_hSocket, F_SETFL, flags|O_NONBLOCK);
	
	rett = connect(handle->m_hSocket, (struct sockaddr*)&serv, sizeof(struct sockaddr_in));
	if(rett < 0)
	{
		if (errno != EINPROGRESS)
		{
			LOGE("connet failed 1.");
			return (-1);
		}
	}

	FD_ZERO(&fdWrite);
	FD_SET(handle->m_hSocket, &fdWrite);
	fdread = fdWrite;

	tv.tv_sec = msec/1000;
	tv.tv_usec = (msec * 1000) % 1000000;
	res = select(handle->m_hSocket + 1, &fdread, &fdWrite, NULL, &tv);
	if (res == 0)
	{
		LOGE("connect suc but select timeout");
		ret = -1;
		goto error1;
	}
	if (FD_ISSET(handle->m_hSocket, &fdread) || FD_ISSET(handle->m_hSocket, &fdWrite))
	{
		len = sizeof(error);
		if (getsockopt(handle->m_hSocket, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
		{
			LOGE("m_connect_tcp get sockopt error!");
			ret = -1;
		}
	}
error1:
	fcntl(handle->m_hSocket, F_SETFL, flags); // 转换为阻塞状态
	return ret;
}

int m_senddata_tcp(tcpsocket_t *handle, void* lpBuf, int nBufLen)
{
	if (!handle || !lpBuf || handle->m_hSocket == -1) {
		return -1;
	}

	int ret;
	char* pBuf = (char*) lpBuf;
	pthread_mutex_lock(&handle->m_write_lock);
	ret = writen(handle->m_hSocket, pBuf, nBufLen);
	pthread_mutex_unlock(&handle->m_write_lock);
	return ret;
}

// 检查缓冲区在指定时间内是否有1字节以上数据可读
//2014-6-7  LinZHh107  modify
inline int TestRecvBuf_tcp(tcpsocket_t * handle, long long msec)
{
	struct timeval tv;
	fd_set fdRead; 
	tv.tv_sec = msec/1000;
	tv.tv_usec = (msec*1000) % 1000000;

	FD_ZERO(&fdRead);
	FD_SET(handle->m_hSocket, &fdRead);
	int ret = 0;
	ret = select(handle->m_hSocket + 1, &fdRead, 0, 0, &tv);
	if( ret <= 0)
	{
		//LOGE("select/3seconds ret = %d",ret);
		return ret; // 0/-1
	}
	// 	尝试读一个字节
	char buf[2] = {0};
	ret = recv(handle->m_hSocket, buf, 1, MSG_PEEK);//MSG_PEEK
	return ret;
}


// 在指定时间内读取一个完整的HTTP数据包
int recv_httppackage_tcp(tcpsocket_t*handle, void* lpBuf, int nMax, long long msec)
{
	int nRecv = recvPackage(handle->m_hSocket, (char*) lpBuf, nMax, msec);
	if (nRecv > 0)
		*((char*) lpBuf + nRecv) = '\0';

	return nRecv;
}

//int  (*recv_sippackage)(struct _tcpsocket_t*, void* lpBuf, int nMax, int sec);
int recv_sippackage_tcp(tcpsocket_t* handle, void* lpBuf, int nMax, long long msec)
{
	int nRecv = recvPackage(handle->m_hSocket, (char*) lpBuf, nMax, msec);
	if (nRecv > 0)
		*((char*) lpBuf + nRecv) = '\0';

	return nRecv;
}

int m_receive_tcp(tcpsocket_t* handle, void* lpBuf, int nMax, int msec)
{
    if (!handle || !lpBuf || handle->m_hSocket == -1) {
        return -1;
    }
	struct timeval tv;
	fd_set fdRead;
	int ret = 0;

	tv.tv_sec = msec/1000;
	tv.tv_usec = (msec*1000) % 1000000;

	FD_ZERO(&fdRead);
	FD_SET(handle->m_hSocket, &fdRead);
	ret = select(handle->m_hSocket + 1, &fdRead, 0, 0, &tv);
	if (ret <= 0){
		//LOGE("VideoInstance Select timeout .");
		return ret;
	}
	else
		return readn(handle->m_hSocket, (char*) lpBuf, nMax, 0);
}

int domain_nameparse_tcp(tcpsocket_t *handle, const char* szAddress, char *szTcpIp)
{
	char *strIP;		//modified by zhusy
	struct hostent * hp;
	char szIP[128] = { 0 };

	if (handle == NULL)
		return -1;

	pthread_mutex_lock(&handle->m_lock);
	if (strlen(szAddress) == 0 || strcmp(szAddress, "") == 0 || strcmp(szAddress, "NULL") == 0)
	{
		pthread_mutex_unlock(&handle->m_lock);
		return 0;
	}
	if (inet_addr(szAddress) == INADDR_NONE)    //判断字符串ip是否是合法的IP地址形式
	{
		if ((hp = gethostbyname(szAddress)) != NULL)    //根据网络域名ip获取对应的IP地址
		{
			struct in_addr inAddr;
			char *lpAddr = hp->h_addr_list[0];
			if (lpAddr)
			{
				memmove(&inAddr, lpAddr, 4);
				strIP = inet_ntoa(inAddr);
				sprintf(szIP, "%s", strIP);
			}
		}
		else
		{
			pthread_mutex_unlock(&handle->m_lock);
			return -1;
		}

	}
	else
	{
		strcpy(szTcpIp, szAddress);
	}
	//sprintf(szTcpIp, "%s", szIP);
	pthread_mutex_unlock(&handle->m_lock);
	return 0;
}

int init_p_socket_tcp(tcpsocket_t * handle)
{
	int szOptval;

	if (handle == NULL)
		return -1;

	szOptval = 1;

	memset(handle->m_strServerIP, 0, sizeof(handle->m_strServerIP));
	//handle->init_socket = init_socket_tcp;
	handle->close_socket = close_socket_tcp;
	handle->close_p_socket = close_p_socket_tcp;
	handle->domain_nameparse = domain_nameparse_tcp;
	handle->get_tcp_link = get_tcp_link_tcp;
	handle->set_tcp_link = set_tcp_link_tcp;
	handle->test_recvbuf = TestRecvBuf_tcp;
	handle->m_connect = m_connect_tcp;
	handle->m_receive = m_receive_tcp;
	handle->m_senddata = m_senddata_tcp;
	handle->recv_httppackage = recv_httppackage_tcp;
	handle->recv_sippackage = recv_sippackage_tcp;
	handle->m_nServerPort = 0;
	pthread_mutex_init(&handle->m_lock, NULL);
	pthread_mutex_init(&handle->m_write_lock, NULL);
	handle->m_hSocket = socket(AF_INET, SOCK_STREAM, 0);		//0//参数3 必须为0
	if (handle->m_hSocket < 0)
	{
		//my_log("create socket fail!!!!!!!!!!!!!!!!\n");
		return -1;
	}
	if (setsockopt(handle->m_hSocket, SOL_SOCKET, SO_REUSEADDR, (void*) &szOptval, sizeof(szOptval)) == -1)
	{
		//my_log("st net  SO_REUSEADDR setsockopt fail!!!!!!!!!!!!!!!\n");
		goto error;
	}
	return 0;
error: 
	close(handle->m_hSocket);
	handle->m_hSocket = -1;
	return -1;
}

tcpsocket_t *init_socket_tcp()
{
	tcpsocket_t *handle = malloc(sizeof(*handle));
	int szOptval;

	if (handle == NULL)
		return handle;

	szOptval = 1;

	memset(handle->m_strServerIP, 0, sizeof(handle->m_strServerIP));
	//handle->init_socket = init_socket_tcp;
	handle->close_socket = close_socket_tcp;
	handle->close_p_socket = close_p_socket_tcp;
	handle->domain_nameparse = domain_nameparse_tcp;
	handle->get_tcp_link = get_tcp_link_tcp;
	handle->set_tcp_link = set_tcp_link_tcp;
	handle->test_recvbuf = TestRecvBuf_tcp;
	handle->m_connect = m_connect_tcp;
	handle->m_receive = m_receive_tcp;
	handle->m_senddata = m_senddata_tcp;
	handle->recv_httppackage = recv_httppackage_tcp;
	handle->recv_sippackage = recv_sippackage_tcp;
	handle->m_nServerPort = 0;
	handle->m_hSocket = -1;
	pthread_mutex_init(&handle->m_lock, NULL);
	pthread_mutex_init(&handle->m_write_lock, NULL);
	//PF_INET,SOCK_STREAM, IPPROTO_TCP
	handle->m_hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);		//0//参数3 必须为0

	if (handle->m_hSocket < 0)
	{
		free(handle);
		return NULL;
	}
	if (setsockopt(handle->m_hSocket, SOL_SOCKET, SO_REUSEADDR, (void*) &szOptval, sizeof(szOptval)) == -1)
	{
		printf("st net  userLogin setsockopt\n");
		goto error;
	}
	return handle;
error:
	close(handle->m_hSocket);
	handle->m_hSocket = -1;
	free(handle);
	handle = NULL;

	return NULL;
}

