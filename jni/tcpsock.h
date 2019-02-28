/*
 * tcpsock.h
 *
 *  Created on: 2009-10-10
 *      Author: pc
 */

#ifndef TCPSOCK_H_
#define TCPSOCK_H_

#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

#include		<pthread.h>

typedef  struct _tcpsocket_t
{	
	int    m_hSocket;//SOCKET
	char      m_strServerIP[128];
//	CString   m_strServerIP;
	int      m_nServerPort;
	pthread_mutex_t m_lock;
	pthread_mutex_t m_write_lock;
	int tcp_socket_error;
	
	int (*close_socket)(struct _tcpsocket_t *);
	int (*close_p_socket)(struct _tcpsocket_t *);

	int  (*set_tcp_link)(struct _tcpsocket_t *, const char* szTcpIp, int nPort);
	int (*get_tcp_link)(struct _tcpsocket_t *, char* lpszTcpIp,  int *nPort);
	int (*m_connect)(struct _tcpsocket_t*, long long msec);
	int (*m_senddata)(struct _tcpsocket_t *, void* lpBuf, int nBufLen);

	int  (*test_recvbuf)(struct _tcpsocket_t*, long long msec);
	int  (*recv_httppackage)(struct _tcpsocket_t*, void* lpBuf, int nMax, long long msec);
	int  (*recv_sippackage)(struct _tcpsocket_t*, void* lpBuf, int nMax, long long msec);
	int  (*m_receive)(struct _tcpsocket_t*,void* lpBuf, int nMax, int sec);
	int  (*domain_nameparse)(struct _tcpsocket_t*,const char* szAddress, char *szTcpIp);
	
}tcpsocket_t;


tcpsocket_t *init_socket_tcp();
int  init_p_socket_tcp(tcpsocket_t * handle);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
 
 

#endif /* TCPSOCK_H_ */
