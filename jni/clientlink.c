/*
 * clientlink.c
 *
 *  Created on: 2009-10-13
 *      Author: pc
 */

#include "clientlink.h"
//#include "ST_CULib.h"
//#include "ST_CUAPI.h"
//#include "msg_def.h"
#include "st_cuapi.h"
#include "netglobal.h"
#include <stdlib.h>
#include "xml/mxml.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <android/log.h>
//#include "list.h"

#define  LOG_TAG "Clientlink.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

int glistResponseCount = 0;
int viindex = 0;

//2014-6-10 LinZh107 增加报警链表
#define MAX_ALARM_COUNT 32
static int dev_index = 0; 
static int g_alarm_count = 0;
static ck_alarminfo gckInfo[MAX_ALARM_COUNT] = {0};

static void * Thread_Client_Recv(void *param);
static void * Thread_Client_Heart(void *param);

void init_clientlink(clientlink * handle)
{
	handle->m_bLinkFlag = 0;
	handle->m_pThread_Recv = 0;
	handle->m_pThread_Heart = 0;
	handle->m_heartpthreadflag = 0;
	handle->m_recvpthreadflag = 0;
	memset(handle->m_strDomainID, 0, sizeof(handle->m_strDomainID));
	memset(handle->m_strAreaID, 0, sizeof(handle->m_strAreaID));
	memset(handle->m_strRemoteIP, 0, sizeof(handle->m_strRemoteIP));

	memset(&handle->thrdPara, 0, sizeof(THREAD_CLINTLINK_PARA));
	handle->m_nLoginedSum = 0;
    return;
}

int uninit_clientlink(clientlink * handle)
{
	if(handle == NULL)
		return -1;

	handle->m_heartpthreadflag = 0;
	handle->m_recvpthreadflag = 0;
	
	if(handle->m_TcpSocket != NULL)
		handle->m_TcpSocket->close_socket(handle->m_TcpSocket);

	return 0;
}

int clientlinkPrepareToLink(clientlink *handle)
{
	memset(handle->m_strDomainID,0,sizeof(handle->m_strDomainID));
	memset(handle->m_strAreaID,0,sizeof(handle->m_strAreaID));

	handle->m_TcpSocket = init_socket_tcp();
	if(handle->m_TcpSocket == NULL)
		return -1;
	return 0;
}

#if 0
void clientlinkSetLinkFlag(int bConnected)
{
	//m_csFlag.Lock();
	m_bLinkFlag = bConnected;
	//m_csFlag.Unlock();
}
//
int clientlinkGetLinkFlag(void)
{
	m_csFlag.Lock();
	BOOL bFlag = m_bLinkFlag;
	m_csFlag.Unlock();
	return bFlag;
}
#endif

void clientlinkDisonnect(clientlink *handle)
{
	//LOGI("In Clientlink_Disconnect.");
	// 发送退出命令(不一定被响应)
	int nSeqNum = CreateSeqByTime();
	int buf[256];
	int nLen = 0;

	memset(buf, 0, sizeof(buf));
	nLen = CreatePack_CMU_Logout(buf, nSeqNum);
    
	if(handle->m_TcpSocket->m_hSocket != -1)
    {
        handle->m_TcpSocket->m_senddata(handle->m_TcpSocket, buf, nLen);
        //LOGI("after send data to sock.");
    }
	
	handle->m_bLinkFlag = 0;
	   
	if(handle->m_TcpSocket->m_hSocket != -1)
    {
		close(handle->m_TcpSocket->m_hSocket);
		handle->m_TcpSocket->m_hSocket = -1;
	}
	memset(gckInfo, 0, sizeof(gckInfo[0])*MAX_ALARM_COUNT);
	LOGI("Out Clientlink_Disconnect.");
}

int clientlinkReconnect(clientlink *handle, int pdwserverpara)
{
	LIB_INSTANCE_PARA * pPara = (LIB_INSTANCE_PARA *)pdwserverpara;//NULL;
	char szCMUIP[128] = {0};
	int iCMUPort;
	int hRet = 0;

	if ((handle->m_TcpSocket->m_hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)	//0//参数3 必须为0
	{
		return -1;
	}

	if (handle->m_TcpSocket->set_tcp_link(handle->m_TcpSocket, theApp.m_ServerIP, theApp.m_nPort) < 0)
		return -1;

	handle->m_TcpSocket->get_tcp_link(handle->m_TcpSocket, szCMUIP, &iCMUPort);
	//theApp.m_strRealServerIP.Format("%s", szCMUIP);
	sprintf(theApp.m_strRealServerIP, "%s", szCMUIP);

	if (handle->m_TcpSocket->m_connect(handle->m_TcpSocket, pPara->CmdTimeOut) < 0)
	{
		if (handle->m_TcpSocket->m_hSocket != -1)
		{
			close(handle->m_TcpSocket->m_hSocket);
			handle->m_TcpSocket->m_hSocket = -1;
		}
		return -1;
	}

	if ((hRet = clientlinkRegisterServer(handle, pdwserverpara)) >= 0)
		handle->m_bLinkFlag = 1;

	return hRet;
}

// 启动工作者线程,线程对象不自动退出,需要手动delete

//void ClientLink::StartThreads(DWORD pdwServerPara)
int clientlinkStartThreads(clientlink *handle, int pdwserverpara)
{
	LOGI("----> In ClientLinkStartThreads.");
	int ret;
	handle->thrdPara.pdwserverpara = pdwserverpara;
	handle->thrdPara.pclient = handle;

	handle->m_recvpthreadflag = 2;
	ret = pthread_create(&handle->m_pThread_Recv, NULL, Thread_Client_Recv, &handle->thrdPara);
	if (ret < 0)
	{
		LOGE("create pThread recv error!");
		handle->m_recvpthreadflag = 0;
		return -1;
	} 
	usleep(100 * 1000);  //单位微秒	sleep(10); //单位秒
	handle->m_heartpthreadflag = 2;
	ret = pthread_create(&handle->m_pThread_Heart, NULL, Thread_Client_Heart, &handle->thrdPara);
	if (ret < 0)
	{
		LOGE("create pThread heart error!");
		handle->m_heartpthreadflag = 0;
		return -1;
	}
	return 0;
}

// 2013-11-6
void handleSignalQuit(int signo)
{
	if (SIGQUIT == signo)
		pthread_exit(NULL);
}

// 退出实例的线程对象并删除线程对象,主要是为了正常退出线程
//void ClientLink::QuitInstance()
void clientlinkQuitInstance(clientlink *handle)
{
	if (handle->m_heartpthreadflag >= 2)
	{
		handle->m_heartpthreadflag = 10;
		while (1)
		{
			if (handle->m_heartpthreadflag == 1)
				break;
			else
				usleep(1000); // 释放CPU
		}
	}

	while (handle->m_recvpthreadflag >= 2)
	{
		handle->m_recvpthreadflag = 10;
		while (1)
		{
			if (handle->m_recvpthreadflag == 1)
				break;
			else
				usleep(1000); // 释放 CPU
		}
	}
}

//BOOL  ClientLink::SendHeart(DWORD pdwServerPara)
int clientlinkSendHeart(struct _clientlink *handle, int pdwserverpara)
{
	int nSeqNum = CreateSeqByTime();
	int buf[256], ret = 0;
	int nLen = 0;

	PACK_ELEMENT *element = (PACK_ELEMENT *) malloc(sizeof(PACK_ELEMENT));
	if(!element)
	{	return -1; }
	memset(buf, 0, sizeof(buf));

	nLen = CreatePack_CMU_Heart(buf, nSeqNum);
	if (handle->m_TcpSocket->m_senddata(handle->m_TcpSocket, buf, nLen) <= 0)
	{
		ret = -1;
		goto error1;
	}

	// 增加一个检索处理
	ret = WaitForResponse(pdwserverpara, nSeqNum, CMU_KEEPALIVE, element, 3000);

	error1: if (element != NULL)
	{
		free(element);
		element = NULL;
	}

	return ret;
}

// 连接上服务器后, 发送登陆等求并recv(不开启接收线程)
int clientlinkRegisterServer(clientlink *handle, int pdwserverpara)
{
	LOGI("----> ClientlinkRegisterServer.");
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pdwserverpara;
	if (handle == NULL)
	{
		return -5;
	}

	char buf[1024];
	memset(buf, 0, sizeof(buf));

	int nSeqNum = CreateSeqByTime();
	int nLen;
	HTTP_OBJECT *httpObject = (HTTP_OBJECT *) malloc(sizeof(HTTP_OBJECT));
	if(!httpObject)
	{
		return -6;
	}

	long long dwStart;
	struct timeval tv;
	int ret;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	char str1[20] = { 0 }, *str2 = NULL;

	nLen = CreatePack_LoginReq(buf, nSeqNum);

	handle->m_TcpSocket->m_senddata(handle->m_TcpSocket, buf, nLen);

	// 接收线程还未启动, 需要主动读socket缓冲区
	// 循环接受, 排除接收到服务器的主动消息

	gettimeofday(&tv, NULL);
	dwStart = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;

	while (1)  //while(GetTickCount()-dwStart < pPara->CmdTimeOut)
	{
		gettimeofday(&tv, NULL);
		if ((((long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000) - dwStart) >= pPara->CmdTimeOut)
		{
			break;
		}
		memset(buf, 0, sizeof(buf));
		ret = handle->m_TcpSocket->recv_httppackage(handle->m_TcpSocket, buf, sizeof(buf) - 1, pPara->CmdTimeOut/3);
		if (ret < 0)
		{
			if (NULL != httpObject)
			{
				free(httpObject);
				httpObject = NULL;
			}
			return -7;
		}
		memset(httpObject, 0, sizeof(HTTP_OBJECT));  //

		ret = st_http_parseHttp(buf, httpObject);		///modified by zhusy end!
		// 解析出错, 直接返回错误
		if (ret <= 0)
		{
			//AddLoginFlag(0x00000006);
			if (NULL != httpObject)
			{
				free(httpObject);
				httpObject = NULL;
			}
//			LOGE("clientlinkRegisterServer -8");
			return -8;
		}
		// 如果收到的不是回应数据包, 则重新接收
// 		if( httpObject.type != RESP_TYPE)
// 			continue;

		if (httpObject->type != RESP_TYPE)
		{
			continue;
		}

		if (0 != strcmp(USER_LOGIN_REQ, httpObject->cmd))		//modified by zhusy
		{
			if (NULL != httpObject)
			{
				free(httpObject);
				httpObject = NULL;
			}
//			LOGE("clientlinkRegisterServer -9");
			return -9;
		}

		break;
	}

	// 分析xml字符串
	tree = mxmlLoadString(NULL, (const char*) httpObject->xml, MXML_NO_CALLBACK);
	root = tree;

	node = root;
	node = mxmlFindElement(node, root, "result", NULL, NULL, MXML_DESCEND);
	strncpy(str1, mxmlElementGetAttr(node, (const char*) "code"), sizeof(str1) - 1);
	if (strcmp("0", str1))
	{
		int ret = -16;
		if (!strcmp("1", str1))
		{
			//AddLoginFlag(0x80000007);
			//LOGI("验证失败: 用户名或密码错误!");
			ret = -10;
		}
		else if (!strcmp("2", str1))
		{
			//AddLoginFlag(0x80000008);
			//LOGI("验证失败: SMU服务器无连接!");
			ret = -11;
		}
		else if (!strcmp("3", str1))
		{
			//AddLoginFlag(0x80000009);
			//LOGI("失败: 该用户已在其他地方登陆..");
			ret = -12;
		}
		else if (!strcmp("6", str1))
		{
			//AddLoginFlag(0x8000800A);
			//LOGI("失败: 登陆请求被拒绝, 用户名未激活");
			ret = -13;
		}
		else if (!strcmp("7", str1))
		{
			//AddLoginFlag(0x8000800B);
			//LOGI("失败: 登陆请求被拒绝, 用户名已超过使用期限");
			ret = -14;
		}
		else if (!strcmp("4", str1))
		{
			//bstr = pXml->getElementsByTagName(_bstr_t("CUIP"))->Getitem(0)->Gettext();
			node = root;
			node = mxmlFindElement(node, root, "CUIP", NULL, NULL, MXML_DESCEND);
			// node->child->value.text.string;
			//m_strRemoteIP.Format("%s", (char*)bstr);
			sprintf(handle->m_strRemoteIP, "%s", node->child->value.text.string);
			//bstr = pXml->getElementsByTagName(_bstr_t("UserNum"))->Getitem(0)->Gettext();
			node = root;
			node = mxmlFindElement(node, root, "UserNum", NULL, NULL, MXML_DESCEND);
			handle->m_nLoginedSum = atoi(node->child->value.text.string);
			//AddLoginFlag(0x8000800C);
			//LOGI("失败: 共享用户数超出限定值");
			ret = -15;
		}

		if (NULL != httpObject)
		{
			free(httpObject);
			httpObject = NULL;
		}
		mxmlDelete(tree);
		return ret;
	}
	node = root;
	node = mxmlFindElement(node, root, "DomainID", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
		return -11;
	sprintf(handle->m_strDomainID, "%s", node->child->value.text.string);
	//LOGI("clientlinkRegisterServer m_strDomainID %s\n", handle->m_strDomainID);
	node = root;
	node = mxmlFindElement(node, root, "AreaID", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
		return -12;
	sprintf(handle->m_strAreaID, "%s", node->child->value.text.string);

	strcpy(handle->m_strRemoteIP, handle->m_TcpSocket->m_strServerIP);
	strcpy(handle->m_strLoginedRemoteIP, handle->m_strRemoteIP);
	//LOGI("clientlinkRegisterServer m_strAreaID %s m_strRemoteIP %s", handle->m_strAreaID, handle->m_strRemoteIP);

	strcpy(handle->m_strLoginAreaID, handle->m_strAreaID);
	//m_strLoginDomainID = m_strDomainID;
	strcpy(handle->m_strLoginDomainID, handle->m_strDomainID);

	if (httpObject != NULL)
	{
		free(httpObject);
		httpObject = NULL;
	}
	//added by zhusy end!

	mxmlDelete(tree);
	//handle-> m_nLoginedSum++;
	return 0; // 只有在这里才返回成功
}


void *Thread_Client_Heart(void *param)
{
    LOGI("Create----->Thread_Client_Heart Thread!");
	THREAD_CLINTLINK_PARA* pPara = (THREAD_CLINTLINK_PARA*) param;
	clientlink* pClient = pPara->pclient;
	long long dwLastSend;
	int i;
	struct timeval tv;
	unsigned long iDeathCount = 0;
	int count = 0;

	pthread_detach(pthread_self());
    
    gettimeofday(&tv, NULL);
    dwLastSend = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
	
	while(1)
	{
		for(i = 0; i < 50; i++)
		{
			if (pClient->m_heartpthreadflag == 10)
				goto retout;

			if (0 == pClient->m_bLinkFlag) // 2013-11-5
			{
				clientlinkDisonnect(pClient);
				clientlinkProcessNotifyMsg(pClient, pPara->pdwserverpara, 0);
				if (0 == clientlinkReconnect(pClient, pPara->pdwserverpara))
				{
					pClient->m_bLinkFlag = 1;
                    LOGI("+++++++++++++++++++++++++++");
					LOGI("Thread_Client_Heart reconnect succees!");
					clientlinkProcessNotifyMsg(pClient, pPara->pdwserverpara, 1);
					iDeathCount = 0;
				}
				else
					LOGE("reSend Clientlink_HeartPack!");
			}
            usleep(100*1000);//sleep 1 second
		}

		if (1 == pClient->m_bLinkFlag) // 如果连接正常
		{
			gettimeofday(&tv, NULL);
			if ((((long long)tv.tv_sec * 1000 + (long long)tv.tv_usec/1000) - dwLastSend)
                >=  CMU_KEEP_CIRCUIT/3 )
			{
				dwLastSend = (long long)tv.tv_sec*1000 + (long long)tv.tv_usec/1000;
                
				if (clientlinkSendHeart(pClient, pPara->pdwserverpara) < 0) // 定时发送心跳
					count++;
				else
                {
					count = 0;
                }
				if(count > 3)
                {
					//clientlinkDisonnect(pClient);
					count = 0;
					pClient->m_bLinkFlag = 0;
                }
			}
		}
	}
	retout:
		pClient->m_heartpthreadflag = 1;
	LOGI("Exit---->Thread_Client_Heart!");
	return NULL;
}

#define  RECVBUF_MAX_SIZE  (512*1024)//1024*256 modified by zhusy
//UINT ClientLink::Thread_Client_Recv(LPVOID lParam)
void *Thread_Client_Recv(void *param)
{
    LOGI("Create----->Thread_Client_Recv Thread!");

	int ret = 0;
	int nRecv = 0;
	dev_index = 0;
	g_alarm_count = 0;
	memset(gckInfo, 0, sizeof(ck_alarminfo) * MAX_ALARM_COUNT);
	
	THREAD_CLINTLINK_PARA* pPara = (THREAD_CLINTLINK_PARA*) param;
	clientlink* pClient = pPara->pclient;

	pthread_detach(pthread_self());

	// 2013-11-5
	signal(SIGQUIT, handleSignalQuit);
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);     //允许退出线程
	//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置立即取消

	char  *pBufRecv= NULL;
	pBufRecv = malloc(RECVBUF_MAX_SIZE);
	if (!pBufRecv)
		goto error;

	while(1)
	{
		//	LOGI("Thread_Client_Recv Loop!");
		if(pClient->m_recvpthreadflag == 10)
			break;

		if (0 == pClient->m_bLinkFlag){
			usleep(50*1000);
			continue;
		}

		ret = pClient->m_TcpSocket->test_recvbuf(pClient->m_TcpSocket, 500);//CMU_KEEP_CIRCUIT *3); // 2013-11-11
		if( ret <= 0)
		{
			if(pClient->m_bLinkFlag != 1)
				pClient->m_bLinkFlag = 0;
			continue;
		}
		else
		{
			memset(pBufRecv, 0, RECVBUF_MAX_SIZE);
			nRecv = pClient->m_TcpSocket->recv_httppackage(pClient->m_TcpSocket, pBufRecv, RECVBUF_MAX_SIZE-1, 1000);
			if(nRecv <= 0)
			{
				if((errno == ETIMEDOUT) || (errno == 0)){
				    errno = 0;
				   	continue;
				} 
				pClient->m_bLinkFlag = 0;
				//clientlinkDisonnect(pClient);  //remove at 2014-11-08
				errno = 0;
				continue;
			}
			else
			{
				pBufRecv[nRecv] = '\0';
				//LOGI("Received Data = %s",pBufRecv);
				if (nRecv < RECVBUF_MAX_SIZE - 1)
					clientlinkProcessRecvData(pClient, pPara->pdwserverpara, pBufRecv);
			}
		}
	}
	
retout:
	if (pBufRecv != NULL)
		free(pBufRecv);
error:
	pClient->m_recvpthreadflag = 1;
	LOGI("Exit---->Thread_Client_Recv!");
	return NULL;
}

void clientlinkSendResponseMsg(clientlink *handle)
{
	char buf[256] = { 0 };
	strcpy(buf, "HTTP/1.1 200 OK\r\nCommand:AlarmResponse\r\nContent-Type:text/xml;Charset=UTF-8\r\nContent-Length:0\r\nSeq:123456789\r\n\r\n");
	//LOGI("clientlinkSendResponseMsg senddata buf = %s",buf);
	handle->m_TcpSocket->m_senddata(handle->m_TcpSocket, buf, strlen(buf));
}

void clientlinkProcessRecvData(clientlink *handle, int pdwserverpara, char* szBuf)
{
	LIB_INSTANCE_PARA * pPara = (LIB_INSTANCE_PARA *)pdwserverpara;
	HTTP_OBJECT *httpObject = (HTTP_OBJECT *) malloc(sizeof(HTTP_OBJECT)); //added by zhusy
	if(!httpObject){
		return;
	}
	memset(httpObject, 0, sizeof(HTTP_OBJECT));
	
	int ret = 0;
	//AlarmInfo_list *pAlarm = malloc(sizeof(*pAlarm)); //2014-6-10 LinZh107
	
	ret = st_http_parseHttp(szBuf, httpObject); // HTTP_LIB:: zhusy modified 2009-07-10
	if (ret <= 0)
	{
		//LOGE("st_http_parseHttp fail");
		goto retout;
	}

	if (httpObject->type == RESP_TYPE)
	{
		//if(httpObject->cmd[0] == 'H')
		//	LOGI("Received HeartBeat ACK Message!");
		PACK_ELEMENT_list* pElement = malloc(sizeof(PACK_ELEMENT_list));
		if(!pElement)
		{
			goto retout;
		}
		memset(pElement, 0, sizeof(PACK_ELEMENT_list));
		pElement->pack_element.httpObject = *httpObject; //modified by zhusy 2009-0711
		
		struct timeval tv;
		gettimeofday(&tv, NULL);
		pElement->pack_element.dwTime = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000; // 加上时间戳
		pthread_mutex_lock(&pPara->lockResponseList);
		list_add(&pElement->list, &pPara->ResponseList.list);
		pthread_mutex_unlock(&pPara->lockResponseList);
		goto retout;
	}
	else
	{
		//	LOGI("httpObject->type != RESP_TYPE");
		if (0 == strcmp(httpObject->cmd, TW_MODIFY_QUERY) || 0 == strcmp(httpObject->cmd, TOUR_MODIFY_QUERY)
				|| 0 == strcmp(httpObject->cmd, GROUP_MODIFY_QUERY)
				|| 0 == strcmp(httpObject->cmd, "RequestAddUserEncoderInfo")
				|| 0 == strcmp(httpObject->cmd, "RequestAddUserDecoderInfo"))
		{
			PACK_ELEMENT_list* pElement = malloc(sizeof(PACK_ELEMENT_list));
            if (!pElement) {
                goto retout;
            }
			memset(pElement, 0, sizeof(PACK_ELEMENT_list));
			struct timeval tv;
			pElement->pack_element.httpObject = *httpObject; //modified by zhusy 2009-0711
			gettimeofday(&tv, NULL);
			pElement->pack_element.dwTime = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;
			pthread_mutex_lock(&pPara->lockResponseList);
			list_add(&pElement->list, &pPara->ResponseList.list);
			pthread_mutex_unlock(&pPara->lockResponseList);
			goto retout;
		}
		else
		{
			//----------在这里处理告警通知---modified by LinZh107 2014-6-10-----------------
			//	LOGI("handle Alarminfo here!");
			AlarmInfo alarm = { 0 };
			struct timeval tv;			
			gettimeofday(&tv, NULL);
			alarm.dwAlarmTime = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;
			
			int i = 0;
			char szGUName[32] = { 0 }; //LinZh107  fix that long name cause the crash!
			char strNameTmp[128];
			
			//char szContent[TEXT_MAXLEN] = { 0 };

			clientlinkSendResponseMsg(handle); // 对CMU的主动消息, 须发送一个回应
			st_http_parseHttpParam(httpObject, "DomainID", alarm.szDomainid);
			st_http_parseHttpParam(httpObject, "PUID", alarm.szPuid);
			st_http_parseHttpParam(httpObject, "GUID", alarm.szGuid);
			st_http_parseHttpParam(httpObject, "GUName", szGUName);
			strncpy(strNameTmp, szGUName, sizeof(szGUName) - 1);
			ConvertUtf8ToGBK(strNameTmp);
			strncpy(alarm.szDeviceName, strNameTmp, sizeof(alarm.szDeviceName) - 1);

			if (0 == strcmp(httpObject->cmd, "PUonline"))
			{
				alarm.nAlarmType = NET_ALARM_PU_ONLINE;
				strcpy(alarm.szDeviceName, "CSG");
				strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104009);
			}
			else if (0 == strcmp(httpObject->cmd, "PULogout"))
			{
				alarm.nAlarmType = NET_ALARM_PU_LOGOUT;
				strcpy(alarm.szDeviceName, "CSG");
				strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104011);
			}
			else if (0 == strcmp(httpObject->cmd, "InformUserLogout"))
			{
				alarm.nAlarmType = NET_ALARM_USER_LOGIN_IN_THE_OTHER_PLACE;
				strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104012);
			}
			else if (0 == strcmp(httpObject->cmd, "PUAlarmOccur"))
			{
				for (i = 0; i < MAX_PARAM_NUM; i++)
				{
					if (0 == strcmp(httpObject->cmdparm[i].paramName, "AlarmType"))
					{
						//	LOGI("httpObject->cmdparm[%d].paramName == AlarmType",i);
						if (0 == strcmp(httpObject->cmdparm[i].paramValue, "0")) //I/O告警
						{
							alarm.nAlarmType = PU_ALARM_PRI_CHANNEL_I_O;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104013);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "1"))
						{
							alarm.nAlarmType = PU_ALARM_PRI_CHANNEL_MOVE_DETECTIVE;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104014);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "2"))
						{
							alarm.nAlarmType = PU_ALARM_PRI_CHANNEL_VIDEO_LOST;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104015);
						}
						/*else if ( 0 == strcmp(httpObject.cmdparm[i].paramValue, "3"))
						 {
						 alarm.nAlarmType =
						 }
						 */
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "4"))
						{
							alarm.nAlarmType = PU_ALARM_PRI_CHANNEL_HARDDISK;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104016);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "5"))
						{
							alarm.nAlarmType = PU_ALARM_PRI_CHANNEL_MDU_IP_ID_NOT_MATCH;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104017);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "10")) //I/O告警
						{
							alarm.nAlarmType = PU_ALARM_SEC_CHANNEL_I_O; 
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104018);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "11"))
						{
							alarm.nAlarmType = PU_ALARM_SEC_CHANNEL_MOVE_DETECTIVE;						
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104019);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "12"))
						{
							alarm.nAlarmType = PU_ALARM_SEC_CHANNEL_VIDEO_LOST;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104010);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "14"))
						{
							alarm.nAlarmType = PU_ALARM_SEC_CHANNEL_HARDDISK;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104021);
						}
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "15"))
						{
							alarm.nAlarmType = PU_ALARM_SEC_CHANNEL_MDU_IP_ID_NOT_MATCH;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104022);
						}
						//2014-6-10 add LinZh107  start
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "99"))
						{	
							memset(szGUName, 0, sizeof(szGUName));
							st_http_parseHttpParam(httpObject, "longitude1", szGUName); 
							alarm.dwLongitude = atof(szGUName);
							
							memset(szGUName, 0, sizeof(szGUName));
							st_http_parseHttpParam(httpObject, "latitude1", szGUName); 
							alarm.dwLatitude = atof(szGUName);
							
							memset(szGUName, 0, sizeof(szGUName));
							st_http_parseHttpParam(httpObject, "speed", szGUName); 
							alarm.dwSpeed = atof(szGUName);
							
							memset(szGUName, 0, sizeof(szGUName));
							st_http_parseHttpParam(httpObject, "subType", szGUName); 
							if (0 == strcmp(szGUName, "25"))	//当subtype的值为25时为传ck值的通道
							{
								alarm.nAlarmType = PU_ALARM_MEASUREMENT_CTRL_INFO;
								
								//识别设备ID	dev_index
								dev_index = 0;
								while(dev_index < MAX_ALARM_COUNT)
								{
									//	dwLongitude = -2146434623.00
									//	nAlarmType = 1280

									if(gckInfo[dev_index].devid == (int)alarm.dwLongitude)
										break;
									else if(gckInfo[dev_index].devid == 0){
										gckInfo[dev_index].devid = (int)alarm.dwLongitude;
										g_alarm_count ++;
									}
									else 
										dev_index++;
								} 
								if(dev_index == MAX_ALARM_COUNT)
									goto retout;
								//	注意:
								//	由于测控信息一次来一个值，这样对上层的读取会
								//	造成不必要的麻烦和时间开销，所以要在底层解析
								//	并存到某个struct 里，这样底层和上层只需通过一次交互
								//	就能实现所有值的同步，这里我自建一个新的
								//	ck_alarminfo 结构体传输

								gckInfo[dev_index].alarmtime = tv.tv_sec;
								int ntempType = (int)alarm.dwLatitude;
								ntempType = ntempType>>8;
								ntempType = ntempType&0xff;
								switch(ntempType)
								{
									//	ck_val[0] = pAlarmInfo->dwSpeed;
									case CK_SENSOR_TYPE_AIR_TEMPERATURE:	//==>"空气温度"
										gckInfo[dev_index].airtemp = alarm.dwSpeed;
										//LOGE("gckInfo[%d].airtemp = %0.2f",dev_index, alarm.dwSpeed);
										break;
 									case CK_SENSOR_TYPE_AIR_HUMIDITY:		//==>"空气湿度"
 										gckInfo[dev_index].airhumi = alarm.dwSpeed; 
 										//LOGI("gckInfo[%d].airhumi = %0.2f",dev_index, alarm.dwSpeed);
										break;
									case CK_SENSOR_TYPE_SOIL_TEMPERATURE:	//==>"土壤温度"
										gckInfo[dev_index].soiltemp = alarm.dwSpeed;
										break;
									case CK_SENSOR_TYPE_SOIL_MOISTURE:		//==>"土壤湿度"
										gckInfo[dev_index].soilhumi = alarm.dwSpeed;		
										break;
									case CK_SENSOR_TYPE_ILLUMINATION:		//==>"光照强度"
										gckInfo[dev_index].illuminance = alarm.dwSpeed;
										break;
									case CK_SENSOR_TYPE_CO2_CONCENTRATION:	//==>"二氧化碳"
										gckInfo[dev_index].CO2density = alarm.dwSpeed;
										break;
									case CK_SENSOR_TYPE_WATER_SATURATION:	//==>"土壤水饱和度"
										gckInfo[dev_index].water_sat = alarm.dwSpeed;
										break;
									case CK_SENSOR_TYPE_DAILY_RAINFALL:	//==>"日降雨量"
										gckInfo[dev_index].daily_rain = alarm.dwSpeed;
										break;
									case CK_SENSOR_TYPE_IONS:			//==>"负离子"
										gckInfo[dev_index].anion = alarm.dwSpeed;
										break;
									case CK_SENSOR_TYPE_PM25:			//==>"PM2.5"
										gckInfo[dev_index].pm25 = alarm.dwSpeed;
										break;
									case CK_SENSOR_TYPE_WIND_SPEED:		//==>"风速"
										gckInfo[dev_index].wind_speed = alarm.dwSpeed;
										break;
									default:
										break;										
								}
							}
							else
							{
								alarm.nAlarmType = PU_ALARM_SATELLITE_MAP_ALARM;									
								sprintf(alarm.szAlarmInfo, "GPS告警(%s); 经度:%0.5f, 纬度:%0.5f, 速度:%0.5f",
									alarm.szDeviceName, alarm.dwLongitude, alarm.dwLatitude, alarm.dwSpeed);	
							} 																			
						}//end paramValue="99")
						else if (0 == strcmp(httpObject->cmdparm[i].paramValue, "25")) //添加测控信息 Add 2014-6-4
						{
							LOGI("-type = 25 --httpObject.xml内容：--%s", httpObject->xml);
							alarm.nAlarmType = PU_ALARM_MEASUREMENT_CTRL_INFO;
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104062);
						}						
				//2014-6-10 add LinZh107  end
						
						else	// 未解析告警类型;
						{
							LOGI("alarm.nAlarmType = ALARM_NOT_PARSE_TYPE");
							alarm.nAlarmType = ALARM_NOT_PARSE_TYPE;  
							strcpy(alarm.szAlarmInfo, ID_ALARMINFO_104023);
						}

					}
					//	end  paramName == AlarmType
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "subType"))
					{
						alarm.dwParam1 = atoi(httpObject->cmdparm[i].paramValue);
						if (0 == alarm.dwParam1)
						{
							switch (alarm.nAlarmType)
							{
							case PU_ALARM_PRI_CHANNEL_HARDDISK:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104054, sizeof(alarm.szAlarmInfo) - 1);
								break;
							case PU_ALARM_PRI_CHANNEL_VIDEO_LOST:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104053, sizeof(alarm.szAlarmInfo) - 1);
								break;
							case PU_ALARM_PRI_CHANNEL_MOVE_DETECTIVE:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104052, sizeof(alarm.szAlarmInfo) - 1);
								break;
							case PU_ALARM_PRI_CHANNEL_I_O:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104051, sizeof(alarm.szAlarmInfo) - 1);
								break;
							case PU_ALARM_SEC_CHANNEL_HARDDISK:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104058, sizeof(alarm.szAlarmInfo) - 1);
								break;
							case PU_ALARM_SEC_CHANNEL_VIDEO_LOST:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104057, sizeof(alarm.szAlarmInfo) - 1);
								break;
							case PU_ALARM_SEC_CHANNEL_MOVE_DETECTIVE:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104056, sizeof(alarm.szAlarmInfo) - 1);
								break;
							case PU_ALARM_SEC_CHANNEL_I_O:
								strncpy(alarm.szAlarmInfo, ID_ALARMINFO_104055, sizeof(alarm.szAlarmInfo) - 1);
								break;
							}
						}
					}
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "AlarmText"))
					{
						//	LOGI("httpObject->cmdparm[%d].paramName == AlarmText",i);
						char *strText = httpObject->cmdparm[i].paramValue;
						ConvertUtf8ToGBK(strText);
						strncpy(alarm.szRemarks, strText, sizeof(alarm.szRemarks) - 1);
					}
				}
			}

#if 0
			else if (0 == strcmp(httpObject->cmd, "TwAlarmInfo"))
			{
				alarm.nAlarmType = TV_ALARM;
				for (i = 0; i < MAX_PARAM_NUM; i++)
				{
					//	CString str;
					char str[256];
					sprintf(str, "%s", httpObject->cmdparm[i].paramValue);
					//	str.Format("%s", httpObject.cmdparm[i].paramValue);
					ConvertUtf8ToGBK(str);							//
					//char *pTmp = TranslateUTF_8ToGB(httpObject.cmdparm[i].paramValue, strlen(httpObject.cmdparm[i].paramValue));
					if (0 == strcmp(httpObject->cmdparm[i].paramName, "TVID"))
					{
						sprintf(alarm.twInfo.szTVID, "%s", str);
					}
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "DisplayGUID"))
					{
						sprintf(alarm.twInfo.szDisplayGUID, "%s", str);
					}
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "DecoderGUID"))
					{
						sprintf(alarm.twInfo.szDecoderGUID, "%s", str);
					}
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "EncoderName"))
					{
						sprintf(alarm.twInfo.szEncoderName, "%s", str);
					}
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "WindowNo"))
					{
						alarm.twInfo.iWindowNo = atoi(str);
					}
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "RunType"))
					{
						alarm.twInfo.iRunType = atoi(str);
					}
					else if (0 == strcmp(httpObject->cmdparm[i].paramName, "RunName"))
					{
						sprintf(alarm.twInfo.szRunName, "%s", str);
					}
				}
			}

			pthread_mutex_lock(&pPara->lockalarmlist);	
			memcpy(&gckInfo[dev_index], &alarm.szAlarmInfo, sizeof(alarm.szAlarmInfo));
			pthread_mutex_unlock(&pPara->lockalarmlist);
#endif
			goto retout;
		}
	}

	//added by zhusy start
retout: 
	if (NULL != httpObject)
	{
		free(httpObject);
		httpObject = NULL;
	}
}

void clientlinkProcessNotifyMsg(clientlink *handle, int pdwserverpara, int nStatus)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pdwserverpara;
	AlarmInfo_list *pAlarm = malloc(sizeof(AlarmInfo));
	char szContent[TEXT_MAXLEN] = { 0 };
	struct timeval tv;

	if ((0 != nStatus) && (1 != nStatus))
		return;

	if (pAlarm != NULL)
	{
		memset(pAlarm, 0, sizeof(AlarmInfo));
		gettimeofday(&tv, NULL);
		pAlarm->alarminfo.dwAlarmTime = (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000; //time(NULL);//modified by zhusy
		
		strcpy(pAlarm->alarminfo.szDeviceName, ID_ALARMINFO_104024);

		if (0 == nStatus)
		{
			pAlarm->alarminfo.nAlarmType = NET_ALARM_CMU_CONNECT_INTERMIT;
			strcpy(pAlarm->alarminfo.szDeviceName, ID_ALARMINFO_104025);
		}
		else if (1 == nStatus)
		{
			pAlarm->alarminfo.nAlarmType = NET_ALARM_CMU_RECONNECT_SUCCESS;
			strcpy(pAlarm->alarminfo.szDeviceName, ID_ALARMINFO_104026);
		}

#if 0
		pthread_mutex_lock(&pPara->lockalarmlist);
		list_add_tail(&pAlarm->list, &pPara->alarm_list.list);
		pthread_mutex_unlock(&pPara->lockalarmlist);
#endif

	}

	//added by zhusy
	if (NULL != pAlarm)
	{
		free(pAlarm);
		pAlarm = NULL;
	}
}

int GetAlarmInfo(int pdwserverpara, ck_alarminfo *pAlarminfo, int index)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pdwserverpara;
	if(0 == g_alarm_count)
		return -1;

	index = index % g_alarm_count;
	if(index > MAX_ALARM_COUNT)
		return -1;

	pthread_mutex_lock(&pPara->lockResponseList);

	memcpy(pAlarminfo, &gckInfo[index], sizeof(ck_alarminfo));
	char  temp[32] = {0};//字符串
	sprintf(temp, "DevID: %010x", pAlarminfo->devid); 

	pthread_mutex_unlock(&pPara->lockResponseList);

	return 0;
}
