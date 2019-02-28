/*
 * st_cuapi.c
 *
 *  Created on: 2009-10-16
 *      Author: pc
 */

//----------------------------------------------------------------------
// 说明: 该文件中部分函数导出
//----------------------------------------------------------------------
//#include "stdafx.h"
//#include "ST_CULib.h"
#include "clientlink.h"
#include "st_cuapi.h"
#include "netglobal.h"
#include "VideoInstance.h"
#include "AudioInstance.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <android/log.h>
#include "xml/mxml.h"

#define null 0
#define  LOG_TAG    "st_cuapi.c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

int InitilLibInstance()
{
	LIB_INSTANCE_PARA *pPara = (LIB_INSTANCE_PARA*)malloc(sizeof(LIB_INSTANCE_PARA));
	if(pPara == NULL)
    {
		LOGE("malloc error\n");
		return -1;
	}
	memset(pPara, 0, sizeof(LIB_INSTANCE_PARA));

	init_clientlink(&pPara->gClientLink);

	INIT_LIST_HEAD(&pPara->ResponseList.list);
	pthread_mutex_init(&pPara->lockResponseList, NULL);
	return (int) pPara;
}

int DeleteLibInstance(int pServerPara)
{
    if(pServerPara == 0)
        return -1;
	LIB_INSTANCE_PARA *pPara = (LIB_INSTANCE_PARA*) (pServerPara);

	if (pPara == 0)
		return -1;

	pthread_mutex_destroy(&pPara->lockResponseList);

	if (pPara != NULL)
	{
		free(pPara);
		pServerPara = NULL;
	}
	return 0;
}

/*----设置命令超时时间， 建议在第一时间调用，以后不再调用----*/
void SetCmdTimeOut(int pServerPara, int secs)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;

	if (secs > 15000)
		secs = 10000; // 限制在15秒以内
	if (secs < 5000)
		secs = 5000; // 限制在5秒以上
	pPara->CmdTimeOut = secs;
}

void CancelWaitting(int pServerPara)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	theApp.m_bBlocking = 0;
}

/*----------------------------------------------------------------------
 // 注册窗口和消息, 在其他网络通信库函数之前
 -----------------------------------------------------------------------*/
int ClientStartUp(int pServerPara, unsigned int nMessage, int hWnd)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	P_LIB_INSTANCE_PARA pPara = NULL;
	pPara = (P_LIB_INSTANCE_PARA) pServerPara;

	pPara->nNetLibMsg = nMessage;
	pPara->hMsgWnd = hWnd;

	return 0;
}

int ClientCleanUp(int pServerPara)
{
	P_LIB_INSTANCE_PARA pPara = NULL;
	PACK_ELEMENT_list *cur_node = NULL, *curn_node = NULL;

	pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	pPara->hMsgWnd = NULL;
	pPara->nNetLibMsg = 0;
	// 清理全局缓冲区(堆)
	if (pServerPara == NULL)
		return -1;

	return 0;
}

int QueryPuRealRoute(int pServerPara, GUINFO guInfo, MDU_ROUTE_INFO *mduInfo)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	char buf[512] = { 0 }; // 2013-10-28
	int nSeqNum = CreateSeqByTime();
	int nBufLen = 0;
	int ret = 0;

	// 1.构造命令PACKAGE并发送到CMU
	nBufLen = CreatePack_CMU_RealStartReq(buf, nSeqNum, guInfo.DomainID, guInfo.PUID, guInfo.GUID);
	if (!pPara->gClientLink.m_TcpSocket->m_senddata(pPara->gClientLink.m_TcpSocket, buf, nBufLen))
	{
		ret = -11;
		goto error;
	}

	PACK_ELEMENT *packElement = (PACK_ELEMENT *) malloc(sizeof(PACK_ELEMENT));
	if(packElement == NULL)
		return -10;
	memset(packElement, 0, sizeof(PACK_ELEMENT));
	if (WaitForResponse(pServerPara, nSeqNum, CMU_REAL_START_REQ, packElement,
			pPara->CmdTimeOut) < 0)
	{
		ret = -12;
		goto error;
	}

	MDU_ROUTE_INFO mduRoute;
	memset(&mduRoute, 0, sizeof(MDU_ROUTE_INFO));
	if (GetRealMDURoute(packElement->httpObject.xml, &mduRoute) < 0) //strXML
	{
		ret = -13;  // 参考XML错误描述文件
		goto error;
	}

	strncpy(mduInfo->szLocalMduId, mduRoute.szLocalMduId, sizeof(mduInfo->szLocalMduId) - 1);
	strncpy(mduInfo->szRemoteMduId, mduRoute.szRemoteMduId, sizeof(mduInfo->szRemoteMduId) - 1);
	mduInfo->uLocalPort = mduRoute.uLocalPort;
	mduInfo->uRemoteCsgPort = mduRoute.uRemoteCsgPort;
	mduInfo->uRemoteMduPort = mduRoute.uRemoteMduPort;

	if ((pPara->gClientLink.m_TcpSocket->domain_nameparse(pPara->gClientLink.m_TcpSocket,
			mduRoute.szLocalMduIp, mduInfo->szLocalMduIp) < 0)
			|| (pPara->gClientLink.m_TcpSocket->domain_nameparse(pPara->gClientLink.m_TcpSocket,
					mduRoute.szRemoteCsgIp,	mduInfo->szRemoteCsgIp) < 0)
			|| (pPara->gClientLink.m_TcpSocket->domain_nameparse(pPara->gClientLink.m_TcpSocket,
					mduRoute.szRemoteMduIp,	mduInfo->szRemoteMduIp) < 0))
	{
		ret = -14;
	}

error:
	if (NULL != packElement)
	{
		free(packElement);
		packElement = NULL;
	}
	return ret;
}

//-----------------------------------------------------------------------
// 函数说明：登陆接口函数
//-----------------------------------------------------------------------
int RequestLogin(int pServerPara, char *lpszServerIP, unsigned int nPort, char *lpszUserID,
		char *lpszUserPwd, char *lpszLocalDomainID, char *lpRemoteIP, int *nSum)
{
	LOGI("----> RequestLogin.");
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	// 现在开始连接并登陆
	char szCMUIP[128] = { 0 };	//correct 2014-6-7 LinZh107
	unsigned int iCMUPort;
	if( pPara->gClientLink.m_bLinkFlag > 0)
		return -1;

	if(clientlinkPrepareToLink(&pPara->gClientLink)  < 0)
	{
		//my_log("error socket %d\n");
		return -2;
	}
	// 设置全局参数;
	theApp.m_nPort = nPort;
	strcpy(theApp.m_ServerIP, lpszServerIP);
	strcpy(theApp.m_UserID, lpszUserID);
	ConvertUtf8ToGBK(theApp.m_UserID);
	strcpy(theApp.m_UserPwd, lpszUserPwd);

	if (pPara->gClientLink.m_TcpSocket->set_tcp_link(pPara->gClientLink.m_TcpSocket, lpszServerIP, nPort) < 0)
	{
		LOGE("RequestLogin set tcp link error");
		return -3;
	}

	pPara->gClientLink.m_TcpSocket->get_tcp_link(pPara->gClientLink.m_TcpSocket, szCMUIP, (int*) &iCMUPort);
	sprintf(theApp.m_strRealServerIP, "%s", szCMUIP);

	if (pPara->gClientLink.m_TcpSocket->m_connect(pPara->gClientLink.m_TcpSocket, pPara->CmdTimeOut) < 0)
	{
		LOGE("connect error");
		//clientlinkDisonnect(&pPara->gClientLink);
		uninit_clientlink(&pPara->gClientLink);
		return -4;
	}
	else
	{
		int ret;
		pPara->gClientLink.m_bLinkFlag = 1;

		ret = clientlinkRegisterServer(&pPara->gClientLink, pServerPara);
		if (ret == 0)
		{
			strncpy(lpRemoteIP, pPara->gClientLink.m_strRemoteIP, 60 - 1);
			strncpy(lpszLocalDomainID, pPara->gClientLink.m_strDomainID, 32 - 1);
			(*nSum) = pPara->gClientLink.m_nLoginedSum;

			return clientlinkStartThreads(&pPara->gClientLink, pServerPara);
		}
		else
		{
			LOGE("RegisterServer error ret = %d",ret);
			clientlinkDisonnect(&pPara->gClientLink);
			uninit_clientlink(&pPara->gClientLink);
			return ret;
		}
	}
}

//获取验证码
int GetCheckCode(int pServerPara, char* lpszServerIP, int nPort, char* lpszUserID, char* lpszUserPwd)
{
	P_LIB_INSTANCE_PARA pPara = NULL;
	pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	char szCMUIP[128] = { 0 };
	unsigned int iCMUPort;
	char buf[1024] = { 0 };
	int nBufLen = 0;
	int nSeqNum = CreateSeqByTime();

	if (clientlinkPrepareToLink(&pPara->gClientLink) < 0)
	{
		//my_log("error socket %d\n");
		return -1;
	}

	// 设置全局参数;
	theApp.m_nPort = nPort;
	strcpy(theApp.m_ServerIP, lpszServerIP);
	strcpy(theApp.m_UserID, lpszUserID);
	strcpy(theApp.m_UserPwd, lpszUserPwd);

	if (pPara->gClientLink.m_TcpSocket->set_tcp_link(pPara->gClientLink.m_TcpSocket, lpszServerIP, nPort) < 0)
	{
		//my_log("RequestLogin set tcp link error \n");
		return -2;
	}
	pPara->gClientLink.m_TcpSocket->get_tcp_link(pPara->gClientLink.m_TcpSocket, szCMUIP, (int*) &iCMUPort);

	sprintf(theApp.m_strRealServerIP, "%s", szCMUIP);
	if (pPara->gClientLink.m_TcpSocket->m_connect(pPara->gClientLink.m_TcpSocket, pPara->CmdTimeOut) < 0)
	{
		clientlinkDisonnect(&pPara->gClientLink);
		uninit_clientlink(&pPara->gClientLink);
		return -3;
	}

	char str1[20] = { 0 };
	int ret = 0;
	char strUserID[32];
	sprintf(strUserID, "%s", theApp.m_UserID);
	char paramBuf[110];
	sprintf(paramBuf, "User=%s", strUserID);

	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "GetCheckCode", paramBuf, NULL);
	buf[nBufLen] = '\0';
	if (pPara->gClientLink.m_TcpSocket->m_senddata(pPara->gClientLink.m_TcpSocket, buf, nBufLen) <= 0)
	{
		return -4;
	}

	char xmlbuf[1024];
	memset(xmlbuf, 0, sizeof(xmlbuf));

	pPara->gClientLink.m_TcpSocket->recv_httppackage(pPara->gClientLink.m_TcpSocket, (void *) xmlbuf,
			sizeof(xmlbuf) - 1, 5000);

	tree = mxmlLoadString(NULL, xmlbuf, MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		ret = -6;
		//my_log("mxmlLoadString fail \n");
		goto error;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "result", NULL, NULL, MXML_DESCEND);

	strncpy(str1, mxmlElementGetAttr(node, (const char*) "code"), sizeof(str1) - 1);

	if (strcmp("0", str1) == 0)
	{
		ret = 0;
	}
	else
	{
		ret = -7;
	}
	error: mxmlDelete(tree);
	return ret; // 只有在这里才返回成功

}

//登录时需验证码
int RequestLoginEx(int pServerPara, char* lpszServerIP, int nPort, char* lpszUserID, char* lpszUserPwd,
		char* lpszLocalDomainID, char* lpRemoteIP, int* nSum, char* lpszCheckCode)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	char szCMUIP[60] = { 0 };
	unsigned int iCMUPort;

	// 现在开始连接并登陆
	if (pPara->gClientLink.m_bLinkFlag > 0)
	{
		return -1;
	}

	if (clientlinkPrepareToLink(&pPara->gClientLink) < 0)
	{
		//my_log("error socket %d\n");
		return -1;
	}
	// 设置全局参数;
	theApp.m_nPort = nPort;
	//theApp.m_ServerIP = lpszServerIP;
	strcpy(theApp.m_ServerIP, lpszServerIP);
	strcpy(theApp.m_UserID, lpszUserID);
	strcpy(theApp.m_UserPwd, lpszUserPwd);
	strcpy(theApp.m_strCheckCode, lpszCheckCode);

	if (pPara->gClientLink.m_TcpSocket->set_tcp_link(pPara->gClientLink.m_TcpSocket, lpszServerIP, nPort) < 0)
	{
		//my_log("RequestLogin set tcp link error \n");
		return -1;
	}
	pPara->gClientLink.m_TcpSocket->get_tcp_link(pPara->gClientLink.m_TcpSocket, szCMUIP, (int*) &iCMUPort);

	sprintf(theApp.m_strRealServerIP, "%s", szCMUIP);
	if (pPara->gClientLink.m_TcpSocket->m_connect(pPara->gClientLink.m_TcpSocket, pPara->CmdTimeOut) < 0)
	{
		//my_log("connect error\n");

		clientlinkDisonnect(&pPara->gClientLink);
		uninit_clientlink(&pPara->gClientLink);
		return -1;
	}
	else
	{
		int hr;

		pPara->gClientLink.m_bLinkFlag = 1;
		hr = clientlinkRegisterServer(&pPara->gClientLink, pServerPara);

		if (hr == 0)
		{
			strncpy(lpRemoteIP, pPara->gClientLink.m_strRemoteIP, 128 - 1);
			strncpy(lpszLocalDomainID, pPara->gClientLink.m_strDomainID, 32 - 1); //(LPSTR)(LPCSTR)pPara->gClientLink.m_strDomainID
			(*nSum) = pPara->gClientLink.m_nLoginedSum;
			clientlinkStartThreads(&pPara->gClientLink, pServerPara);
			return 0;
		}

		//my_log("RegisterServer error\n");
		clientlinkDisonnect(&pPara->gClientLink);
		uninit_clientlink(&pPara->gClientLink);
		return hr;

	}
	return -1;
}

int GrabLogin(int pServerPara) //独占模式
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;

	char buf[512] = { 0 };
	int nBufLen = 0;
	int nSeqNum = CreateSeqByTime();
	char strParam[128];
	PACK_ELEMENT *packElement = (PACK_ELEMENT *) malloc(sizeof(PACK_ELEMENT));
	if(packElement == NULL)
		return -1;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	char str1[20] = { 0 };
	int ret = 0;

	memset(buf, 0, sizeof(buf));

	sprintf(strParam, "User=%s", theApp.m_UserID);
	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "EnsureUserLogin", strParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';
	if (pPara->gClientLink.m_TcpSocket->m_senddata(pPara->gClientLink.m_TcpSocket, buf, nBufLen) <= 0)
	{
		ret = -1;
		//my_log("send Grablogin fail !!!!\n");
		goto error;
	}

	memset(buf, 0, sizeof(buf));

	if (WaitForResponse(pServerPara, nSeqNum, "EnsureUserLogin", packElement,
			(long long) 3000) < 0)
	{
		ret = -1;
		goto error;
	}

	tree = mxmlLoadString(NULL, (const char*) packElement->httpObject.xml,
	MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		ret = -1;
		//my_log("mxmlLoadString fail \n");
		goto error;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "result", NULL, NULL, MXML_DESCEND);

	strncpy(str1, mxmlElementGetAttr(node, (const char*) "code"), sizeof(str1) - 1);

	if (strcmp("0", str1) == 0)
	{
		ret = 0;

		goto error;
	}
	else
	{
		ret = -1;
		//my_log("mxmlLoadString code fail \n");
	}
	error: if (NULL != packElement)
	{
		free(packElement);
		packElement = NULL;
	}
	mxmlDelete(tree);
	return ret; // 只有在这里才返回成功
}

int RequestLogout(int pServerPara)
{
	LIB_INSTANCE_PARA * pPara = (LIB_INSTANCE_PARA *)pServerPara;
	PACK_ELEMENT_list *cur_node = NULL, *curn_node = NULL;
	pPara->isExit = 1;
	clientlinkQuitInstance(&pPara->gClientLink);
	// 清理全局缓冲区(堆)
    clientlinkDisonnect(&pPara->gClientLink);
    int ret = uninit_clientlink(&pPara->gClientLink);
    if (ret != 0) {
        return -1;
    }
	cur_node = list_entry((&pPara->ResponseList.list)->next, PACK_ELEMENT_list, list);
	curn_node = list_entry(cur_node->list.next, PACK_ELEMENT_list, list);
	while (1)
	{
		//temp = pos;              // 记录此次循环的POSITION
		PACK_ELEMENT* pElement = NULL; //&(cur_node->pack_element);

		if (&cur_node->list == (&pPara->ResponseList.list))
			break;

		pElement = &(cur_node->pack_element);
		list_del(&cur_node->list);
		free(cur_node);
		cur_node = NULL;
		pPara->msg_list_num--;
		cur_node = curn_node;
		curn_node = list_entry(curn_node->list.next, PACK_ELEMENT_list, list);
	}

	memset(theApp.m_UserID, 0, sizeof(theApp.m_UserID));
	memset(theApp.m_UserPwd, 0, sizeof(theApp.m_UserPwd));
	return 0;
}

int GetDeviceList(int pServerPara, const char* lpszQueryID, int nType, DEVICE_NODE* pNode,
		int nMaxCount, int *pCount) //LPCTSTR lpszQueryID
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	if (pServerPara == NULL){
		return -1;
	}

	char* buf = (char*) malloc(256);
	if(buf == NULL)
		return -11;
	memset(buf, 0, (sizeof(char) * 256));

	int nBufLen = 0;
	int nSeqNum = 0;
	int ret = 0;

	char *pStr = NULL;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	int nCount = 0;
	DEVICE_NODE dev_node;
	int nRealCount = 0;

	mxml_node_t *pNode_Parent = NULL, *pNode_Child = NULL;
	char *bstr = NULL, *bstrTemp = NULL;
	*pCount = 0;

	// Create cmd
	srand(time(NULL));
	nSeqNum = rand();
	if (lpszQueryID == NULL){ // 查询根目录
		//LOGI("LibAPI_GetDeviceList 1");
		nBufLen = CreatePack_DevceReq(buf, nSeqNum, pPara->gClientLink.m_strDomainID, \
				pPara->gClientLink.m_strAreaID);
	}
	else{
		if (nType == 0)
			nBufLen = CreatePack_DevceReq(buf, nSeqNum, lpszQueryID, "");  // 域类型
		else if (nType == 1)
			nBufLen = CreatePack_DevceReq(buf, nSeqNum, "", lpszQueryID);  // 区类型
		//LOGI("LibAPI_GetDeviceList 2");
	}

	if (pPara->gClientLink.m_TcpSocket->m_senddata(pPara->gClientLink.m_TcpSocket, \
			buf, nBufLen) <= 0){
		ret = -1;
		goto error;
	}

	PACK_ELEMENT *packElement = (PACK_ELEMENT *) malloc(sizeof(PACK_ELEMENT));
	if(NULL == packElement){
		ret = -1;
		goto error;
	}
	memset(packElement, 0, sizeof(PACK_ELEMENT));
	if (WaitForResponse(pServerPara, nSeqNum, DEVICELIST_REQ, packElement, \
			pPara->CmdTimeOut) < 0){
		ret = -1;
		goto error;
	}

	//LOGI("LibAPI_GetDeviceList 3.1");
	pStr = packElement->httpObject.xml;
	tree = mxmlLoadString(NULL, pStr, MXML_NO_CALLBACK);
	if (tree == NULL){
		ret = -1;
		goto error;
	}

	//LOGI("LibAPI_GetDeviceList 3.2");
	node = root = tree;
	node = mxmlFindElement(node, root, "parameters", NULL, NULL, MXML_DESCEND);
	if (node == NULL){	//add by LinZh107
		ret = -1;
		goto error;
	}
	//LOGI("LibAPI_GetDeviceList 3.3");
	memset(&dev_node, 0, sizeof(DEVICE_NODE));
	pNode_Parent = node->child;
	if (lpszQueryID == NULL)
	{
		bstr = pNode_Parent->value.element.name;
		if (strcmp(bstr, "Domain") == 0){
			dev_node.nType = 0;
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "DomainID");
			sprintf(dev_node.DomainID, "%s", (char*) bstrTemp);
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "DomainName");
			sprintf(dev_node.DomainName, "%s", (char*) bstrTemp);
		}
		else if (strcmp(bstr, "Area") == 0){
			//LOGI("LibAPI_GetDeviceList 4");
			dev_node.nType = 1;
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "AreaID");
			sprintf(dev_node.AreaID, "%s", (char*) bstrTemp);
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "AreaName");
			sprintf(dev_node.AreaName, "%s", (char*) bstrTemp);
		}
		pNode[nRealCount] = dev_node;
		nRealCount++;
	}

RESTART:
	node = pNode_Parent;
	while (!pPara->isExit)
	{
		pNode_Child = mxmlWalkNext(node, root,	MXML_DESCEND);
		if (pNode_Child == NULL)
			break;

		node = pNode_Child;

		if (nRealCount >= nMaxCount)
			break;

		memset(&dev_node, 0, sizeof(dev_node));

		bstr = pNode_Child->value.element.name;

		if (pNode_Child->value.element.name == NULL)
			continue;
		if (strcmp(bstr, "Domain") == 0)
		{
			dev_node.nType = 0;
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Child, (const char*) "DomainID");
			sprintf(dev_node.DomainID, "%s", (char*) bstrTemp);
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Child, (const char*) "DomainName");
			sprintf(dev_node.DomainName, "%s", (char*) bstrTemp);

		}
		else if (strcmp(bstr, "Area") == 0)
		{
			dev_node.nType = 1;
			bstrTemp = pNode_Parent->value.element.name;
			if (bstrTemp == NULL)
				continue;

			if (0 == strcmp((char *) bstrTemp, "Domain"))
			{
				bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "DomainID");
				sprintf(dev_node.DomainID, "%s", (char*) bstrTemp);
				bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "DomainName");
				sprintf(dev_node.DomainName, "%s", (char*) bstrTemp);
			}
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Child, (const char*) "AreaID");
			sprintf(dev_node.AreaID, "%s", (char*) bstrTemp);
			bstrTemp = (char*) mxmlElementGetAttr(pNode_Child, (const char*) "AreaName");
			sprintf(dev_node.AreaName, "%s", (char*) bstrTemp);

		}
		else if (strcmp(bstr, "GU") == 0)
		{
			if (pNode_Child->child == NULL)
				continue;

			dev_node.nType = 2;

			bstr = pNode_Parent->value.element.name;
			if (bstr == NULL)
				continue;

			if (strcmp(bstr, "Domain") == 0)
			{
				bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "DomainID");
				sprintf(dev_node.DomainID, "%s", (char*) bstrTemp);
				bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, (const char*) "DomainName");
				sprintf(dev_node.DomainName, "%s", (char*) bstrTemp);
			}
			else if (strcmp(bstr, "Area") == 0)
			{
				bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, "AreaID");
				sprintf(dev_node.AreaID, "%s", (char*) bstrTemp);
				bstrTemp = (char*) mxmlElementGetAttr(pNode_Parent, "AreaName");
				sprintf(dev_node.AreaName, "%s", (char*) bstrTemp);
			}

			node = mxmlFindElement(pNode_Child, root, "DomainID", NULL, NULL, MXML_DESCEND);
			sprintf(dev_node.guInfo.DomainID, "%s", node->child->value.text.string);
			node = mxmlFindElement(pNode_Child, root, "DomainType", NULL, NULL, MXML_DESCEND);
			dev_node.guInfo.DomainType = atoi(node->child->value.text.string);
			node = mxmlFindElement(pNode_Child, root, "GUID", NULL, NULL, MXML_DESCEND);
			sprintf(dev_node.guInfo.GUID, "%s", node->child->value.text.string);
			//LOGE("dev_node.guInfo.GUID[28]=%c",dev_node.guInfo.GUID[28]);			
			//dev_node.guInfo.GUID[28] = '6'; //modify by LinZh107  1为主码流，6为从码流

			node = mxmlFindElement(pNode_Child, root, "HasDome", NULL, NULL, MXML_DESCEND);
			(strcmp(node->child->value.text.string, "1") == 0) ?
					(dev_node.guInfo.HasDome = 1) : (dev_node.guInfo.HasDome = 0);
			node = mxmlFindElement(pNode_Child, root, "GUType", NULL, NULL, MXML_DESCEND);
			dev_node.guInfo.GUType = atoi(node->child->value.text.string);
			
			node = mxmlFindElement(pNode_Child, root, "GUName", NULL, NULL, MXML_DESCEND);
			sprintf(dev_node.guInfo.GUName, "%s", node->child->value.text.string);
			
			node = mxmlFindElement(pNode_Child, root, "PUID", NULL, NULL, MXML_DESCEND);
			sprintf(dev_node.guInfo.PUID, "%s", node->child->value.text.string);
			
			node = mxmlFindElement(pNode_Child, root, "State", NULL, NULL, MXML_DESCEND);
			(strcmp(node->child->value.text.string, "1") == 0) ?
					(dev_node.guInfo.bState = 1) : (dev_node.guInfo.bState = 0);
			node = mxmlFindElement(pNode_Child, root, "UserRight", NULL, NULL, MXML_DESCEND);
			dev_node.guInfo.right = atoi(node->child->value.text.string);

			dev_node.guInfo.EnTcpPView = 1;
		}
		else
			continue;

		pNode[nRealCount] = dev_node;
		nRealCount++;
	}

	*pCount = nRealCount;   // 传出参数赋值---数据条目统计

	error1:
	//	mxmlIndexDelete(nodeindex);
	mxmlDelete(root);

error: if (NULL != buf)
	{
		free(buf);
		buf = NULL;
	}

	if (NULL != packElement)
	{
		free(packElement);
		packElement = NULL;
	}

	return ret;
}

void* CreateVideoInstance(int pServerPara, CLIENTINFO client)
{
	VideoInstance_t *pIns = (VideoInstance_t *)videoi_CreateInstance(&client);
	if (pIns == NULL){
		LOGE("CreateVideoInstance fail!");
		return NULL;
	}

	if (videoi_InitInstance(pIns) != 0)
	{
		//LOGI("CreateVideoInstance success!");
		free(pIns);
		pIns = NULL;
	}

	return pIns;
}

int CancelVideoInstance(int pServerPara, void* lHandle)
{
	VideoInstance_t* pIns = (VideoInstance_t*) lHandle;

	if (NULL == pIns)
		return -1;

	// just send BYE cmd and do nothing above.
	videoi_CancelInvite(pIns);

	videoi_QuitInstance(pIns);

	return 0;
}

// 注意此函数在实现时要满足可重入性要求
int RealVideoPreviewStart(int pServerPara, void* lHandle, MDU_ROUTE_INFO mduInfo)
{
	VideoInstance_t* pIns = (VideoInstance_t*) lHandle;
	if (pIns == NULL)
		return -21;

	if (videoi_ConnectToServer(pIns, pServerPara, mduInfo.szLocalMduIp, mduInfo.uLocalPort) < 0)
	{
		LOGE("videoi_ConnectToServer Error!");
		return -22;
	}
	if (videoi_InviteMedia(pIns, pServerPara, mduInfo, NULL) < 0)
	{
		LOGE("videoi_InviteMedia Error!");
		return pIns->m_ErrCode;
	}
	videoi_StartThreads(pIns, pServerPara);
	return 0;
}

// 注意此函数在实现时要满足可重入性要求
int RealVideoPreviewStop(int pServerPara, void* lHandle)
{
	if (lHandle == NULL)
		return -1;
	VideoInstance_t* pIns = (VideoInstance_t*) lHandle;
	videoi_BreakSession(pIns);

	return 0;
}

int QueryCsgRoute(int pServerPara, GUINFO guInfo, MDU_ROUTE_INFO *mduInfo)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	char buf[256] = { 0 };
	int nBufLen = 0;
	int nSeqNum = CreateSeqByTime();
	char strParam[128];
	char *pStr = NULL;
	char *bstr = NULL;
	char* pszContent = NULL;
	int ret = 0;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	PACK_ELEMENT *packElement;

	sprintf(strParam, "User=%s&PUID=%s&DomainID=%s", theApp.m_UserID, guInfo.PUID, guInfo.DomainID);
	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "QueryCSGRoute", strParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';
	if (!pPara->gClientLink.m_TcpSocket->m_senddata(pPara->gClientLink.m_TcpSocket, buf, nBufLen))
		return -1;

	memset(buf, 0, sizeof(buf));
	//	PACK_ELEMENT packElement;
	packElement = (PACK_ELEMENT *) malloc(sizeof(PACK_ELEMENT));

	if(packElement == NULL)
	{
		ret = -1;
		goto error;
	}

	if (WaitForResponse(pServerPara, nSeqNum, "QueryCSGRoute",
			packElement, pPara->CmdTimeOut) < 0)
	{
		if (NULL != packElement)
		{
			free(packElement);
			packElement = NULL;
		}
		return -1;
	}

	pStr = packElement->httpObject.xml;
	// Parse xml
	tree = mxmlLoadString(NULL, pStr, MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		ret = -1;
		goto error;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "result", NULL, NULL, MXML_DESCEND);
	bstr = (char*) mxmlElementGetAttr(node, (const char*) "code");
	if (strcmp("0", bstr) == 0)
	{
		node = root;
		node = mxmlFindElement(node, root, "CSGIP", NULL, NULL, MXML_DESCEND);
		pszContent = node->child->value.text.string;
		if (pPara->gClientLink.m_TcpSocket->domain_nameparse(pPara->gClientLink.m_TcpSocket, pszContent,
				mduInfo->szRemoteCsgIp) < 0)
		{
			ret = -1;
			goto error;
		}
		node = root;
		node = mxmlFindElement(node, root, "CSGPORT", NULL, NULL, MXML_DESCEND);
		bstr = node->child->value.text.string;
		mduInfo->uRemoteCsgPort = (int) atoi((char*) bstr);
	}

	error: if (NULL != packElement)
	{
		free(packElement);
		packElement = NULL;
	}
	return ret; // 只有在这里才返回成功
}

int DomeControl(int pServerPara, GUINFO guInfo,	char* msgType,
		int nSpeed, const char* cmdType, const char* param)
{
	int hr;
	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (hr != 0)
			return -2;
	}

	int nVideoId = 0, nGuType = 0;
	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);

	unsigned int uPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;
	char strIP[64] = { 0 };
	sprintf(strIP, "%s", theApp.m_ServerIP);

	char strUserID[32] = { 0 };
	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	char paramBuf[256] = { 0 };
	if (0 == strcmp(msgType, "ControlPTZ"))
	{
		if (0 == guInfo.DomainType)
		{
			memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
			strncpy(mduInfo.szRemoteCsgIp, "NULL", sizeof(mduInfo.szRemoteCsgIp) - 1);
		}
		sprintf(paramBuf,
				"CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&ptzId=%d&speed=%d&cmd=%s&param=%s&guType=%d&DomainID=%s",
				strUserID, mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, nVideoId, nSpeed,
				cmdType, param, nGuType, guInfo.DomainID);
	}
	else if (0 == strcmp(msgType, "SetPresetPTZ"))
	{
		if (0 == guInfo.DomainType)
		{
			memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
			strncpy(mduInfo.szRemoteCsgIp, "NULL", sizeof(mduInfo.szRemoteCsgIp) - 1);
		}
		sprintf(paramBuf,
				"CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&ptzId=%d&presetIndex=%s&guType=%d&DomainID=%s",
				strUserID, mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, nVideoId, param,
				nGuType, guInfo.DomainID);
	}
	else if (0 == strcmp(msgType, "DelPresetPTZ"))
	{
		if (0 == guInfo.DomainType)
		{
			memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
			strcpy(mduInfo.szRemoteCsgIp, "NULL");
		}
		sprintf(paramBuf,
				"CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&ptzId=%d&presetIndex=%s&guType=%d&DomainID=%s",
				strUserID, mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, nVideoId, param,
				nGuType, guInfo.DomainID);
	}
	else
		return -3;

	int nBufLen = 0;
	char httpBuf[1024] = {0};
	int nSeqNum = CreateSeqByTime();
	st_http_makeHttp("GET", nSeqNum, (char*) httpBuf, &nBufLen, msgType, paramBuf, 0);

	tcpsocket_t csgLink;
	if (init_p_socket_tcp(&csgLink) < 0)
		return -4;
	if (csgLink.set_tcp_link(&csgLink, strIP, uPort) < 0)
		return -5;
	if (csgLink.m_connect(&csgLink, 500) < 0)	//msec = 1000  LinZh107
	{
		LOGE("DomeControl connect_req err_code = -6");
		return -6;
	}
	if (csgLink.m_senddata(&csgLink, httpBuf, nBufLen) < 0)
	{
		csgLink.close_p_socket(&csgLink);
		return -7;
	}
	memset(httpBuf, 0, sizeof(httpBuf));
	// 这个时间是否会影响另一次连接
	//csgLink.recv_httppackage(&csgLink, httpBuf, sizeof(httpBuf) - 1, 100); //msec =1000
	csgLink.close_p_socket(&csgLink);

	return 0;
}

int DomeCtrlRequest(int pServerPara, char* pDomainID, char* pGUID, int *pCode)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;

	// 发送至CMU并检索消息队列
	char bufQuery[1024] = { 0 };
	int nSeqNum = CreateSeqByTime();
	int nBufLen = CreatePack_CMU_DomeCtrlQuery(bufQuery, nSeqNum, pGUID, pDomainID);
	if (pPara->gClientLink.m_TcpSocket->m_senddata(pPara->gClientLink.m_TcpSocket, bufQuery, nBufLen) < 0)
	{
		LOGE("Send data error!");
		return -1;
	}
	//LOGI("DomeCtrlRequest Send data!");
	PACK_ELEMENT *packElement = (PACK_ELEMENT *) malloc(sizeof(PACK_ELEMENT));
	if (packElement == NULL)
	{
		return -2;
	}
	int nResult = WaitForResponse(pServerPara, nSeqNum,	CMU_DOMECTRLQUERY, packElement,	500);//pPara->CmdTimeOut);
	if (nResult < 0)
	{
		LOGE("DomeReq: WaitForResponse return FALSE");
		if (NULL != packElement)
		{
			free(packElement);
			packElement = NULL;
		}
		return -3;
	}
	//LOGI("DomeReq: WaitForResponse return TRUE");

	if (NULL != packElement)
	{
		free(packElement);
		packElement = NULL;
	}
	return 0;
}

int GetPuImageEncodePara(int pServerPara, GUINFO guInfo, IMAGEENCODEPARAM* lpParam)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	int hr;
	char *bstr[9];
	char *bstrtemp = NULL;
	char buf[1400];
	char paramBuf[256];
	char strUserID[32];
	int nSeqNum = CreateSeqByTime();
	int nBufLen = 0;
	int nVideoId = 0;
	int nGuType = 0;
	//int ret = 0;
	int i;

	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	mxml_node_t *pNode_Child = NULL;
	MDU_ROUTE_INFO mduInfo;
	tcpsocket_t csgLink;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));

	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return 0x80008035;
	}
	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket tcp fail.\n");
		return -1;
	}

	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;

//	//my_log("csgLink.m_strServerIP=%s",(char*)csgLink.m_strServerIP);
//	sprintf(temp,"%d",csgLink.m_nServerPort);
//	//my_log("m_nServerPort=%s",temp);

	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);

	if (0 == guInfo.DomainType)
	{
		memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
		strcpy(mduInfo.szRemoteCsgIp, "NULL");
	}

	memset(paramBuf, 0, sizeof(paramBuf));
	sprintf(paramBuf, "CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&guType=%d&DomainID=%s", strUserID,
			mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, 1, guInfo.DomainID);

	st_http_makeHttp("GET", nSeqNum, (char*) buf, &nBufLen, "GetPuImageEncodePara",
			paramBuf, NULL);// HTTP_LIB:: zhusy modified 2009-06-24

	buf[nBufLen] = '\0';

	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP,
			csgLink.m_nServerPort) < 0){
		return -1;
	}
	if (csgLink.m_connect(&csgLink, 100) < 0){	//msec = 1000
		return -1;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen)){
		csgLink.close_p_socket(&csgLink);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	csgLink.recv_httppackage(&csgLink, (void *) buf, sizeof(buf) - 1, 1000); //msec =1000

	csgLink.close_p_socket(&csgLink);

	tree = mxmlLoadString(NULL, buf, MXML_NO_CALLBACK);
	if (tree == NULL){
		return -1;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "parameters", NULL, NULL, MXML_DESCEND);
	if (node == NULL){
		return -1;
	}
	pNode_Child = node->child; //pNode_Child所指向的是paramters所有子结点中的第一个

//	pNode_Child = pNode_Child->next;
//	bstrtemp = pNode_Child->value.element.name;//得到子结点的名字

	for (i = 0; i < 9; i++)
	{
		bstr[i] = malloc(64);
		if(NULL == bstr[i])
			goto reout;
		bstrtemp = pNode_Child->child->value.text.string;
		sprintf(bstr[i], "%s", bstrtemp);
		pNode_Child = pNode_Child->next;
	}

	lpParam->video_format = atoi(bstr[0]);
	lpParam->resolution = atoi(bstr[1]);
	lpParam->bitrate_type = atoi(bstr[2]);
	lpParam->level = atoi(bstr[3]);
	lpParam->frame_rate = atoi(bstr[4]);
	lpParam->Iframe_interval = atoi(bstr[5]);
	lpParam->prefer_frame = atoi(bstr[6]);
	lpParam->Qp = atoi(bstr[7]);
	lpParam->encode_type = atoi(bstr[8]);

	reout:
	for (i = 0; i < 9; i++)
	{
		if (NULL != bstr[i])
		{
			free(bstr[i]);
			bstr[i] = NULL;
		}
	}
	return 0;
}

int SetPuImageEncodePara(int pServerPara, GUINFO guInfo, IMAGEENCODEPARAM param)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	int hr;
	char buf[1400];
	int nSeqNum = CreateSeqByTime();
	int nVideoId = 0;
	int nGuType = 0;
	int nBufLen = 0;
	char *bstr = NULL;
	char strParam[1024];
	char strUserID[32];
	tcpsocket_t csgLink;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return 0x80008035;
	}

	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket tcp fail.\n");
		return -1;
	}

	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;

	memset(buf, 0, sizeof(buf));
	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);
	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	if (0 == guInfo.DomainType)
	{
		memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
		strcpy(mduInfo.szRemoteCsgIp, "NULL");
	}
	sprintf(strParam,
			"CUID=%s&CSGIP=%s&CSGPORT=%d&DomainID=%s&PUID=%s&videoId=%d\
						 &video_format=%d&resolution=%d&bitrate_type=%d\
						 &level=%d&frame_rate=%d&Iframe_interval=%d\
						 &prefer_frame=%d&Qp=%d&encode_type=%d&guType=%d",
			strUserID, mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.DomainID, guInfo.PUID, nVideoId,
			param.video_format, param.resolution, param.bitrate_type, param.level, param.frame_rate,
			param.Iframe_interval, param.prefer_frame, param.Qp, param.encode_type, nGuType);

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "SetPuImageEncodePara", strParam, 0); // HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';

	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP, csgLink.m_nServerPort) < 0)
	{
		//my_log("set_tcp_link fail.\n");
		return -1;
	}
	if (csgLink.m_connect(&csgLink, 500) < 0) //msec = 1000
	{
		//my_log("connect fail.\n");
		return -1;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen))
	{
		//my_log("senddata fail.\n");
		csgLink.close_p_socket(&csgLink);
		return -1;
	}

	csgLink.recv_httppackage(&csgLink, buf, sizeof(buf) - 1, 1000); //msec =1000

	csgLink.close_p_socket(&csgLink);

	tree = mxmlLoadString(NULL, buf, MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		//my_log("tree is null.");
		return -1;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "result", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
	{
		//my_log("node is NULL.");
		return -1;
	}
	bstr = (char*) mxmlElementGetAttr(node, (const char*) "code");
	if (strcmp("0", bstr) != 0)
	{
		//my_log("SetPuImageEncodePara error.");
		return -1;
	}
	return 0;
}

int GetPuImageDisplayPara(int pServerPara, GUINFO guInfo, int* contrast, int* bright,
		int* hue, int* saturation)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	int hr;
	int index;
	char *bstr[4];
	char *bstrtemp = NULL;
	int nVideoId = 0;
	int nGuType = 0;
	char buf[1400];
	int nBufLen = 0;
	char strParam[1024];
	char strUserID[32];
	int nSeqNum = CreateSeqByTime();
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	mxml_node_t *pNode_Child = NULL;
	tcpsocket_t csgLink;
	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	memset(buf, 0, sizeof(buf));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return 0x80008035;
	}
	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket fail.\n");
		return -1;
	}
	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;

	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);

	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	if (0 == guInfo.DomainType)
	{
		memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
		strcpy(mduInfo.szRemoteCsgIp, "NULL");
	}

	sprintf(strParam, "CUID=%s&CSGIP=%s&PUID=%s&videoId=%d&guType=%d", strUserID, "127.0.0.1", guInfo.PUID, nVideoId,
			nGuType);

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "GetPuImageDisplayPara", strParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';

	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP, csgLink.m_nServerPort) < 0)
	{
		//my_log("set_tcp_link fail.\n");
		return -1;
	}
	if (csgLink.m_connect(&csgLink, 500) < 0) //msec = 1000
	{
		//my_log("connect fail.\n");
		return -1;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen))
	{
		//my_log("senddata fail.\n");
		csgLink.close_p_socket(&csgLink);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	csgLink.recv_httppackage(&csgLink, buf, sizeof(buf) - 1, 1000); //msec =1000

	csgLink.close_p_socket(&csgLink);

	tree = mxmlLoadString(NULL, buf, MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		//my_log("tree is null.");
		return -1;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "parameters", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
	{
		//my_log("node is NULL.");
		return -1;
	}
	pNode_Child = node->child;
	for (index = 0; index < 4; index++)
	{
		bstr[index] = malloc(64);
		bstrtemp = pNode_Child->child->value.text.string;
		sprintf(bstr[index], "%s", bstrtemp);
		pNode_Child = pNode_Child->next;
	}
	*contrast = atoi(bstr[0]);
	*bright = atoi(bstr[1]);
	*hue = atoi(bstr[2]);
	*saturation = atoi(bstr[3]);

	for (index = 0; index < 4; index++)
	{
		if (NULL != bstr[index])
		{
			free(bstr[index]);
			bstr[index] = NULL;
		}
	}

	return 0;
}

int DefaultPuImageDisplayPara(int pServerPara, GUINFO guInfo, int* contrast, int* bright,
		int* hue, int* saturation)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	int hr;
	int index;
	char *bstr[4];
	char *bstrtemp = NULL;
	char buf[1400];
	int nBufLen = 0;
	int nSeqNum = CreateSeqByTime();
	int nVideoId = 0;
	int nGuType = 0;
	char strUserID[32];
	char strParam[1024];
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	mxml_node_t *pNode_Child = NULL;
	tcpsocket_t csgLink;
	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return 0x80008035;
	}
	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket error!");
		return -1;
	}
	memset(buf, 0, sizeof(buf));

	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;
	//my_log("....guinfo.guid=%s",(char*)guInfo.GUID);
	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);
	ConvertGBKToUtf8(strUserID);
	//my_log("....nVideoId=%d",nVideoId);
	//my_log("....nGuType=%d",nGuType);

	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);
	
	if (0 == guInfo.DomainType)
	{
		memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
		strcpy(mduInfo.szRemoteCsgIp, "NULL");
	}

	sprintf(strParam, "CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&guType=%d&DomainID=%s", strUserID,
			mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, 1, guInfo.DomainID);

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "DefaultPuImageDisplayPara", strParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';
	//my_log("my send contrl: %s\n", buf);
	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP, csgLink.m_nServerPort) < 0)
	{
		//my_log("set_tcp_link fail.\n");
		return -1;
	}
	if (csgLink.m_connect(&csgLink, 500) < 0)	//msec = 1000
	{
		//my_log("connect fail.\n");
		return -1;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen))
	{
		//my_log("senddata fail.\n");
		csgLink.close_p_socket(&csgLink);
		return -1;
	}

	memset(buf, 0, sizeof(buf));
	csgLink.recv_httppackage(&csgLink, buf, sizeof(buf) - 1, 2000); //msec =1000
	//my_log("my send contrl: %s\n", buf);
	tree = mxmlLoadString(NULL, buf, MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		//my_log("...tree is null.");
		return -1;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "parameters", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
	{
		//my_log("node is NULL.");
		return -1;
	}
//	pNode_Child = node->child;//pNode_Child所指向的是paramters所有子结点中的第一个
//	pNode_Child = pNode_Child->next;
//	bstrtemp = pNode_Child->value.element.name;//得到子结点的名字
//	//my_log("bstrtemp=%s",bstrtemp);

	pNode_Child = node->child;
	for (index = 0; index < 4; index++)
	{
		bstr[index] = malloc(64);
		if(NULL == bstr[index])
			goto reout;
		bstrtemp = pNode_Child->child->value.text.string;
		//my_log("bstrtemp..=%s",bstrtemp);
		sprintf(bstr[index], "%s", bstrtemp);
		pNode_Child = pNode_Child->next;
	}

	*contrast = atoi(bstr[0]);
	*bright = atoi(bstr[1]);
	*hue = atoi(bstr[2]);
	*saturation = atoi(bstr[3]);

	reout:
	for (index = 0; index < 4; index++)
	{
		if (NULL != bstr[index])
		{
			free(bstr[index]);
			bstr[index] = NULL;
		}
	}
	csgLink.close_p_socket(&csgLink);
	return 0;
}

int SetPuImageDisplayPara(int pServerPara, GUINFO guInfo, int contrast, int bright,
		int hue, int saturation)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	int hr;
	char *bstr = NULL;
	char buf[1400];
	int nBufLen = 0;
	char strParam[1024];
	char strUserID[32];
	int nSeqNum = CreateSeqByTime();
	int nVideoId = 0;
	int nGuType = 0;
	tcpsocket_t csgLink;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return 0x80008035;
	}

	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket error!");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);
	//my_log("....nVideoId=%d",nVideoId);
	//my_log("....nGuType=%d",nGuType);
	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;

	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	if (0 == guInfo.DomainType)
	{
		memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
		strcpy(mduInfo.szRemoteCsgIp, "NULL");
	}

	sprintf(strParam,
			"CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&contrast=%d&bright=%d&hue=%d&saturation=%d&guType=%d&DomainID=%s",
			strUserID, mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, contrast, bright, hue,
			saturation, nGuType, guInfo.DomainID);

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "SetPuImageDisplayPara", strParam, NULL);// HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';

	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP, csgLink.m_nServerPort) < 0)
	{
		//my_log("set_tcp_link fail.\n");
		return -1;
	}
	if (csgLink.m_connect(&csgLink, 1000) < 0)	//msec = 1000
	{
		//my_log("connect fail.\n");
		return -1;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen))
	{
		//my_log("senddata fail.\n");
		csgLink.close_p_socket(&csgLink);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	csgLink.recv_httppackage(&csgLink, buf, sizeof(buf) - 1, 1000); //msec =1000
//	//my_log("buf=%s",buf);

	csgLink.close_p_socket(&csgLink);

	return 0;
}

int GetPuPtzParam(int pServerPara, GUINFO guInfo, PTZPARAM* lpParam)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	int hr;
	int index;
	PTZPARAM param;
	char buf[1400];
	int nBufLen = 0;
	char *bstr[7];
	char *bstrtemp = NULL;
	int nSeqNum = CreateSeqByTime();
	int nVideoId = 0;
	int nGuType = 0;
	tcpsocket_t csgLink;
	MDU_ROUTE_INFO mduInfo;
	char strUserID[32];
	char strParam[1024];
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	mxml_node_t *pNode_Child = NULL;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return 0x80008035;
	}

	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket error!");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);

	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;

	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	sprintf(strParam, "CUID=%s&CSGIP=%s&PUID=%s&videoId=%d&guType=%d", strUserID, "127.0.0.1", guInfo.PUID, nVideoId,
			nGuType);

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "GetPuPtzParam", strParam, NULL); // HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';
	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP, csgLink.m_nServerPort) < 0)
	{
		//my_log("set_tcp_link fail.\n");
		return -1;
	}
	if (csgLink.m_connect(&csgLink, 500) < 0) //msec = 1000
	{
		//my_log("connect fail.\n");
		return -1;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen))
	{
		//my_log("senddata fail.\n");
		csgLink.close_p_socket(&csgLink);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	csgLink.recv_httppackage(&csgLink, buf, sizeof(buf) - 1, 1000); //msec =1000
	//my_log("buf=%s",buf);
	csgLink.close_p_socket(&csgLink);

	tree = mxmlLoadString(NULL, buf, MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		//my_log("tree is null.");
		return -1;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "parameters", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
	{
		//my_log("node is NULL.");
		return -1;
	}
	pNode_Child = node->child;
	for (index = 0; index < 7; index++)
	{
		bstr[index] = malloc(64);
		bstrtemp = pNode_Child->child->value.text.string;
		sprintf(bstr[index], "%s", bstrtemp);
		pNode_Child = pNode_Child->next;
	}

	param.serial_no = atoi(bstr[0]);
	param.ptz_type = atoi(bstr[1]);
	param.ptz_addr = atoi(bstr[2]);
	param.serial_param.baut_rate = atoi(bstr[3]);
	param.serial_param.data_bit = atoi(bstr[4]);
	param.serial_param.verify_rule = atoi(bstr[5]);
	param.serial_param.stop_bit = atoi(bstr[6]);
	*lpParam = param;

	for (index = 0; index < 7; index++)
	{
		if (NULL != bstr[index])
		{
			free(bstr[index]);
			bstr[index] = NULL;
		}
	}

	return 0;
}

int SetPuPtzParam(int pServerPara, GUINFO guInfo, PTZPARAM param)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	char buf[1400];
	int nBufLen = 0;
	int nSeqNum = CreateSeqByTime();
	int nVideoId = 0;
	int nGuType = 0;
	char strParam[1024];
	char strUserID[32];
	int hr;
	char *bstr = NULL;
	tcpsocket_t csgLink;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return 0x80008035;
	}

	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket error!");
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);

	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;

	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	if (0 == guInfo.DomainType)
	{
		memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
		strcpy(mduInfo.szRemoteCsgIp, "NULL");
	}

	sprintf(strParam,
			"CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d\
					 &ptzAddr=%d&ptzType=%d&serialNo=%d&baudRate=%d&dataBit=%d&verifyRule=%d&stopBit=%d&guType=%d&DomainID=%s",
			strUserID, mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, param.ptz_addr,
			param.ptz_type, param.serial_no, param.serial_param.baut_rate, param.serial_param.data_bit,
			param.serial_param.verify_rule, param.serial_param.stop_bit, nGuType, guInfo.DomainID);

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "SetPuPtzParam", strParam, NULL);// HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';

	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP, csgLink.m_nServerPort) < 0)
	{
		//my_log("set_tcp_link fail.\n");
		return -1;
	}
	if (csgLink.m_connect(&csgLink, 500) < 0)	//msec = 1000
	{
		//my_log("connect fail.\n");
		return -1;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen))
	{
		//my_log("senddata fail.\n");
		csgLink.close_p_socket(&csgLink);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	csgLink.recv_httppackage(&csgLink, buf, sizeof(buf) - 1, 1000); //msec =1000

	csgLink.close_p_socket(&csgLink);

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "result", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
	{
		//my_log("node is NULL.");
		return -1;
	}
	bstr = (char*) mxmlElementGetAttr(node, (const char*) "code");
	if (strcmp("0", bstr) != 0)
	{
		//my_log("SetPuPtzParam error.");
		return -1;
	}
	return 0;
}

int RemoteAlarmEnable(int pServerPara, GUINFO guInfo, int bEnabel)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;
	int hr;
	char buf[1400];
	int nSeqNum = CreateSeqByTime();
	int nVideoId = 0;
	int nGuType = 0;
	int nBufLen = 0;
	char *bstr = NULL;
	char strParam[1024];
	char strUserID[32];
	tcpsocket_t csgLink;
	mxml_node_t *root = NULL, *node = NULL, *tree = NULL;
	MDU_ROUTE_INFO mduInfo;
	memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
	if (guInfo.DomainType == 1)
	{
		hr = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
		mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
		if (0 != hr)
			return -1;
	}

	if (init_p_socket_tcp(&csgLink) < 0)
	{
		//my_log("init socket tcp fail.\n");
		return -2;
	}

	sprintf(csgLink.m_strServerIP, "%s", theApp.m_ServerIP);
	csgLink.m_nServerPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;

	memset(buf, 0, sizeof(buf));
	GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);
	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);
	char szBool[32] = { 0 };
	if (bEnabel > 0)
		sprintf(szBool, "%s", "TRUE");
	else
		sprintf(szBool, "%s", "FALSE");

	if (0 == guInfo.DomainType)
	{
		memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
		strcpy(mduInfo.szRemoteCsgIp, "NULL");
	}
	sprintf(strParam, "CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&enable=%s&guType=%d&DomainID=%s", strUserID,
			mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId, szBool, nGuType, guInfo.DomainID);

	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "RemoteAlarmEnable", strParam, 0); // HTTP_LIB:: zhusy modified 2009-06-24
	buf[nBufLen] = '\0';

	if (csgLink.set_tcp_link(&csgLink, (char*) csgLink.m_strServerIP, csgLink.m_nServerPort) < 0)
	{
		//my_log("set_tcp_link fail.\n");
		return -3;
	}
	if (csgLink.m_connect(&csgLink, 500) < 0) //msec = 1000
	{
		//my_log("connect fail.\n");
		return -4;
	}
	if (!csgLink.m_senddata(&csgLink, buf, nBufLen))
	{
		//my_log("senddata fail.\n");
		csgLink.close_p_socket(&csgLink);
		return -5;
	}

	memset(buf, 0, sizeof(buf));
	csgLink.recv_httppackage(&csgLink, buf, sizeof(buf) - 1, 5000); //msec =1000

	csgLink.close_p_socket(&csgLink);

	tree = mxmlLoadString(NULL, buf, MXML_NO_CALLBACK);
	if (tree == NULL)
	{
		//my_log("tree is null.");
		return -6;
	}

	root = tree;
	node = root;
	node = mxmlFindElement(node, root, "result", NULL, NULL, MXML_DESCEND);
	if (node == NULL)
	{
		//my_log("node is NULL.");
		return -7;
	}
	bstr = (char*) mxmlElementGetAttr(node, (const char*) "code");
	if (strcmp("0", bstr) != 0)
	{
		//my_log("SetPuImageEncodePara error.");
		return -8;
	}
	return 0;
}

//2014-6-9  LinZh107  add SubNotifyInfo
int SubNotifyInfo(int pServerPara)
{
	P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA) pServerPara;

	char buf[1024] = { 0 };
	int nBufLen = 0;
	int nSeqNum = CreateSeqByTime();

	//CString  strParam;
	//strParam.Format("User=%s", theApp.m_UserID);
	char strUserID[32] = { 0 };
	sprintf(strUserID, "%s", theApp.m_UserID);
	ConvertGBKToUtf8(strUserID);

	char strParam[150] = { 0 };
	sprintf(strParam, "User=%s", strUserID);
	st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "SubNotifyInfo", strParam, NULL);
	buf[nBufLen] = '\0';
	if (pPara->gClientLink.m_TcpSocket->m_senddata(pPara->gClientLink.m_TcpSocket, buf, nBufLen) < 0)	//m_senddata_tcp
	{
//		LOGE("SubNotifyInfo: SendData error!");
		return -11;
	}
	memset(buf, 0, sizeof(buf));	//	ZeroMemory(buf, sizeof(buf));
	PACK_ELEMENT *packElement  = (PACK_ELEMENT *) malloc(sizeof(PACK_ELEMENT));
	if(NULL == packElement)
	{
		return -13;
	}
	if (0 > WaitForResponse(pServerPara, nSeqNum, "SubNotifyInfo", packElement,
			pPara->CmdTimeOut))
	{
		if (NULL != packElement)
		{
			free(packElement);
			packElement = NULL;
		}
		return -12;
	}
	//2014-6-9 改到这里
	/*
	 char *pStr = packElement->httpObject.xml;
	 try
	 {
	 int hr = 0;
	 MSXML2:IXMLDOMDocumentPtr  pXml = NULL;
	 hr = pXml.CreateInstance(_T("Msxml2.DOMDocument"));
	 TESTHR(hr);
	 if(!LoadXml(pXml, pStr, FALSE))
	 throw(0);
	 _bstr_t  bstr;
	 bstr =  pXml->getElementsByTagName(_bstr_t("result"))->Getitem(0)->Getattributes()->getNamedItem(_bstr_t("code"))->Gettext();
	 //LOGI("--- The result code=\"%s\" ---", (char*)bstr);
	 if(atoi((char*)bstr) != 0)
	 return ST_FAILED;
	 }
	 catch(...)
	 {
	 return 0x80008002;
	 }
	 */
	if (NULL != packElement)
	{
		free(packElement);
		packElement = NULL;
	}
	return 0;
}

int IsChartOpen(int pServerPara)
{
    P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA)pServerPara;
    P_AUDIOINSTANCE_T pAInst = pPara->pAudioInstance;
    if (pAInst) {
        return 1;
    }
    return 0;
}

int ChartWithOne(int pServerPara, CLIENTINFO* pClient)
{
    P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA)pServerPara;

    P_AUDIOINSTANCE_T pAInst = audioi_CreateInstance();
    if (NULL == pAInst) {
        return -1;
    }

    pAInst->lHandle = pClient->lHandle;
    pAInst->lpFunc = (RECVCALLBACK)(pClient->lpFunc);
    pAInst->nProtocolType = pClient->nProtocolType;
    pAInst->playinst = pClient->playinst;
    //pAInst->m_guInfo = pClient->guInfo;
    memcpy(&pAInst->m_guInfo, &pClient->guInfo, sizeof(GUINFO));

    MDU_ROUTE_INFO mduInfo;
    memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));

    if(0 != QueryPuRealRoute(pServerPara, pClient->guInfo, &mduInfo)){
        return -1;
    }

    if(0 > audioi_InitInstance(pAInst, pClient->nProtocolType) )
        return -1;

    if(0 > audioi_ConnectToServer(pAInst, pServerPara, mduInfo.szLocalMduIp, mduInfo.uLocalPort))
    {
        LOGE("LinZh107 -- Connect Mdu Failed!\n");
        return -2;
    }

    if (audioi_InviteMedia(pAInst, pServerPara, mduInfo, NULL) != 0) {
        audioi_QuitInstance(pAInst);
        LOGE("LinZh107 -- audioi_InviteMedia Failed!\n");
        LOGE("pAInst->m_ErrCode = %0x\n", pAInst->m_ErrCode);
        return -1;
    }

    pPara->pAudioInstance = pAInst;

    if(0 != audioi_StartAudioRecv(pAInst, pServerPara)){
        LOGE("LinZh107 -- audioi_StartAudioRecv Failed!\n");
        return -1;
    }

    return 0;
}

int SendVoiceData(int pServerPara, const void* lpBuf, int nLen)
{
    P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA)pServerPara;
    return audioi_SendVoiceData(pPara->pAudioInstance, lpBuf, nLen);
}

int StopChart(int pServerPara)
{
    P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA)pServerPara;
    P_AUDIOINSTANCE_T pAInst = pPara->pAudioInstance;

    int ret;
    ret = audioi_StopAudioRecv(pAInst);
    ret = audioi_QuitInstance(pAInst);
    audioi_DeleteInstance(pAInst);

    pPara->pAudioInstance = NULL;
    return ret;
}

int IoControl(int pServerPara, GUINFO guInfo, int bStatus, int bReturn)
{
    P_LIB_INSTANCE_PARA pPara = (P_LIB_INSTANCE_PARA)pServerPara;
    int ret = 0;
    char buf[1024] = {0};
    int nBufLen = 0;
    int nSeqNum = CreateSeqByTime();
    int nVideoId = 0;
    int nGuType = 0;
    char strParam[256];
    char strUserID[128];

    MDU_ROUTE_INFO mduInfo;
    memset(&mduInfo, 0, sizeof(MDU_ROUTE_INFO));
    if(guInfo.DomainType == 1)
    {
        ret = QueryCsgRoute(pServerPara, guInfo, &mduInfo);
        mduInfo.uRemoteCsgPort += PORT_CSG_INCREASE_NUM;
        if( 0 != ret)
            return -1;
    }

    GetVideoIdFromGuid(guInfo.GUID, &nVideoId, &nGuType);
    sprintf(strUserID,"%s", theApp.m_UserID);
    ConvertGBKToUtf8(strUserID);

    if(0 == guInfo.DomainType){
        memset(mduInfo.szRemoteCsgIp, 0, sizeof(mduInfo.szRemoteCsgIp));
        strcpy(mduInfo.szRemoteCsgIp, "NULL");
    }


    char szStatus[32] = {0};
    if (bStatus)
        strcpy(szStatus, "TRUE");
    else
        strcpy(szStatus, "FALSE");

    sprintf(strParam, "CUID=%s&CSGIP=%s&CSGPORT=%d&PUID=%s&videoId=%d&status=%s&guType=%d&DomainID=%s",
            strUserID, mduInfo.szRemoteCsgIp, mduInfo.uRemoteCsgPort, guInfo.PUID, nVideoId,
            szStatus, nGuType, guInfo.DomainID);

    st_http_makeHttp("GET", nSeqNum, buf, &nBufLen, "IoControl", strParam, NULL);
    buf[nBufLen] = '\0';

    int uPort = theApp.m_nPort + PORT_CSG_INCREASE_NUM;
    if (uPort == 0) {
        return -1;
    }
    char pCsgIP[64] = {0};
    sprintf(pCsgIP, "%s", theApp.m_ServerIP);

    tcpsocket_t csgLink;
    if(init_p_socket_tcp(&csgLink) < 0)
        return -4;
    if(csgLink.set_tcp_link(&csgLink, pCsgIP, uPort) < 0)
        return -5;
    if(csgLink.m_connect(&csgLink, 500) < 0)//msec = 1000 //LinZh107
    {
        return -6;
    }
    if(csgLink.m_senddata(&csgLink, buf, nBufLen) < 0)
    {
        csgLink.close_p_socket(&csgLink);
        return -7;
    }
    memset(buf, 0, sizeof(buf));

    csgLink.recv_httppackage(&csgLink, buf, sizeof(buf)-1, 500); //msec =1000
    csgLink.close_p_socket(&csgLink);
    if (!bReturn)    // 分析接收到的数据
    {
        int hr = 0;
        char *bstr;
        char *pXmlBuf = strstr(buf, "\r\n\r\n");
        if(!pXmlBuf)
            return -1;
        pXmlBuf = pXmlBuf + strlen("\r\n\r\n");
    }
    ///////////////////////////////////////////
    /*buf	char [1024]	"HTTP/1.1 200 OK\r\n
     Seq:1115322681\r\n
     Content-Type:text/xml\r\n
     Command:IoControl\r\n
     Content-Length:147\r\n
     Date:Thu,02Jul20151153:26 GMT\r\n
     Content-Type:text/xml;charset=UTF-8\r\n
     Server:ivs\r\n\r\n
     <?xml version=\"1.0\" encoding=\"UTF-8\"?><response command=\"IoControl\"><result\n
     code=\"0\">SUCCESS</result><CUID>cx</CUID><CSGIP>NULL</CSGIP></response>\n"	*/

    return 0;
}

