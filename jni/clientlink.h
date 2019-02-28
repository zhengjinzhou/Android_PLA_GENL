/*
 * clientlink.h
 *
 *  Created on: 2009-10-13
 *
 */

#ifndef CLIENTLINK_H_
#define CLIENTLINK_H_

#include "tcpsock.h"
#include <pthread.h>
#include <signal.h>
#include "st_cuapi.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _clientlink;

//新增测控告警信息2014-6-11 LinZh107
typedef struct ck_info_t
{
	int devid;
	int alarmtime;

	float airtemp;
	float airhumi;
	float soiltemp;
	float soilhumi;
	float CO2density;
	float illuminance;

	float water_sat;	//soil water saturation
	float daily_rain;	//daily rainfall
	float anion;		//
	float pm25;
	float wind_speed;
}ck_alarminfo;


typedef struct tag_Thrd_Para{
		struct _clientlink* pclient;
		int pdwserverpara;
	}THREAD_CLINTLINK_PARA;

typedef struct _clientlink
{
	THREAD_CLINTLINK_PARA thrdPara;
	int     m_bLinkFlag;   // socket的连接
	int		m_heartpthreadflag;
	int		m_recvpthreadflag;

	//CCriticalSection     m_csFlag;

	// 2008年2月15日新增
	//CList<DWORD, DWORD>  m_LoginFlagList;
	//CCriticalSection     m_csLoginFlagList;

	char       m_strDomainID[64]; //CString    m_strDomainID;//
	char       m_strAreaID[64];//
	char       m_strRemoteIP[64];//CString    m_strRemoteIP;//
	int		   m_nLoginedSum;//

	char       m_strLoginAreaID[128];  //CString    m_strLoginAreaID;	//add by wy 2008.07.12
	char       m_strLoginDomainID[32]; //CString    m_strLoginDomainID;  //add by wy 2008.07.24
	char       m_strLoginedRemoteIP[64]; //CString    m_strLoginedRemoteIP;	//the IP which the user has logined remotely
	//void *     m_hEventQuit;  // 退出标志
	tcpsocket_t *m_TcpSocket;   // socket对象及函数封装

	pthread_t     m_pThread_Recv;
	pthread_t     m_pThread_Heart;


}clientlink;

void	init_clientlink(clientlink * handle);
int 	uninit_clientlink(clientlink * handle);
void	clientlinkQuitInstance(clientlink *handle);
int		clientlinkStartThreads(clientlink *handle, int pdwserverpara);



int			clientlinkPrepareToLink(clientlink *handle);
int       	clientlinkRegisterServer(clientlink *handle, int pdwserverpara);
int			clientlinkSendHeart(clientlink *handle, int pdwserverpara);
void		clientlinkSetLinkFlag(clientlink *handle, int bConnected);
int			clientlinkGetLinkFlag(clientlink *handle);
void		clientlinkDisonnect(clientlink *handle);
int			clientlinkReconnect(clientlink *handle, int pdwserverpara);
void		clientlinkSendResponseMsg(clientlink *handle);
// 新增
//void		AddLoginFlag(struct _clientlink *handle, int dwFlag);
//int		GetLoginFlag(struct _clientlink *handle);

//add 2014-6-10 LinZh107 
int GetAlarmInfo(int pdwserverpara, ck_alarminfo *pAlarminfo, int devID);

void clientlinkProcessRecvData(clientlink *handle, int pdwserverpara, char* szBuf);

void clientlinkProcessNotifyMsg(clientlink *handle, int pdwserverpara, int nStatus);

void clientlinkSendResponseMsg(clientlink *handle);

void handleSignalQuit(int signo); // 2013-11-6

#define CHINESE_EDITION
//#define ENGLISH_EDITION

#ifdef CHINESE_EDITION
#define IDS_SERVER_DISCONNECTED "与服务器的断开连接！"
#define ID_AFXMESSBOX_101093    "堆分配请求失败, 请退出应用程序"
#define ID_AFXMESSBOX_101094    "无法创建DOMDocument对象，请检查是否安装了MS   XML   Parser   运行库"
#define ID_AFXMESSBOX_101095    "程序异常: 启动线程失败"
#define ID_ALARMINFO_104009     "前端监控设备(PU)上线通知"
#define ID_ALARMINFO_104011     "前端监控设备(PU)掉线通知"
#define ID_ALARMINFO_104012     "该用户已在其它地方登录"
#define ID_ALARMINFO_104013     "主通道:I/O探头告警"
#define ID_ALARMINFO_104014     "主通道:移动侦测告警"
#define ID_ALARMINFO_104015     "主通道:视频丢失告警"
#define ID_ALARMINFO_104016     "主通道:硬盘告警"
#define ID_ALARMINFO_104017     "主通道:MDU的IP与ID不匹配"
#define ID_ALARMINFO_104018     "从通道:I/O探头告警"
#define ID_ALARMINFO_104019     "从通道:移动侦测告警"
#define ID_ALARMINFO_104010     "从通道:视频丢失告警"
#define ID_ALARMINFO_104021     "从通道:硬盘告警"
#define ID_ALARMINFO_104022     "从通道:MDU的IP与ID不匹配"
#define ID_ALARMINFO_104023     "未解析告警类型"
#define ID_ALARMINFO_104024     "本地网络"
#define ID_ALARMINFO_104025     "网络提示: 正在重新连接CMU!"
#define ID_ALARMINFO_104026     "网络提示: 重连CMU成功!"
#define ID_ALARMINFO_104028     "(MDU)视频连接异常, 准备重连!"
#define ID_ALARMINFO_104029     "(MDU)重新请求视频成功!"
#define ID_ALARMINFO_104030     "(MDU)重新请求视频失败!"
#define ID_ALARMINFO_104031     "发生巡更告警"
#define ID_ALARMINFO_104051     "主通道:I/O探头告警结束"
#define ID_ALARMINFO_104052     "主通道:移动侦测告警结束"
#define ID_ALARMINFO_104053     "主通道:视频丢失告警结束"
#define ID_ALARMINFO_104054     "主通道:硬盘告警开始结束"
#define ID_ALARMINFO_104055     "从通道:I/O探头告警结束"
#define ID_ALARMINFO_104056     "从通道:移动侦测告警结束"
#define ID_ALARMINFO_104057     "从通道:视频丢失告警结束"
#define ID_ALARMINFO_104058     "从通道:硬盘告警结束"
#define ID_ALARMINFO_104059     "设备升级成功"
#define ID_ALARMINFO_104060     "设备升级失败"
#define ID_ALARMINFO_104061     "设备开始升级"

#define ID_ALARMINFO_104062     "测控信息"

//add by LinZh107 on 2015-8-14
#define CK_CH_TYPE_MEASUREMENT		0
#define CK_CH_TYPE_CONTROLLING		1

//SENSOR_TYPE
#define CK_SENSOR_TYPE_AIR_TEMPERATURE			0
#define CK_SENSOR_TYPE_AIR_HUMIDITY				1
#define CK_SENSOR_TYPE_SOIL_TEMPERATURE			2
#define CK_SENSOR_TYPE_SOIL_MOISTURE			3
#define CK_SENSOR_TYPE_ILLUMINATION				4
#define CK_SENSOR_TYPE_CO2_CONCENTRATION		5
#define CK_SENSOR_TYPE_WATER_SATURATION			6
#define CK_SENSOR_TYPE_DAILY_RAINFALL			7
#define CK_SENSOR_TYPE_IONS						8
#define CK_SENSOR_TYPE_PM25						9
#define CK_SENSOR_TYPE_WIND_SPEED				10

#else ifdef	EGLIST_DEDITION
#define IDS_SERVER_DISCONNECTED "Un-connect with server!"
#define ID_AFXMESSBOX_101093    "Heap allocation failed, please exit the software"
#define ID_AFXMESSBOX_101094    "Can not creat DOMDocument, please check if the system is installed with MS XML Parser running database"
#define ID_AFXMESSBOX_101095    "Program abnormal: start thread failed."
#define ID_ALARMINFO_104009     "Front-end surveillance device (PU) online information"
#define ID_ALARMINFO_104011     "Front-end surveillance device (PU) drop line information"
#define ID_ALARMINFO_104012     "This user already login somewhere"
#define ID_ALARMINFO_104013     "Main channel: I/O detector alarm"
#define ID_ALARMINFO_104014     "Main channel: motion detection alarm"
#define ID_ALARMINFO_104015     "Main channel: video lose alarm"
#define ID_ALARMINFO_104016     "Main channel: hard disk alram"
#define ID_ALARMINFO_104017     "Main channel: MDU's IP does not match with ID"
#define ID_ALARMINFO_104018     "Sub channel: I/O detector alarm"
#define ID_ALARMINFO_104019     "Sub channel: motion detection alarm"
#define ID_ALARMINFO_104010     "Sub channel: video lose alarm"
#define ID_ALARMINFO_104021     "Sub channel: hard disk alarm"
#define ID_ALARMINFO_104022     "Sub channel: MDU's IP does not match with ID"
#define ID_ALARMINFO_104023     "Do not analyze alarm type"
#define ID_ALARMINFO_104024     "Local network"
#define ID_ALARMINFO_104025     "Network hint: re-connect with CMU!"
#define ID_ALARMINFO_104026     "Network hint: re-connect with CMU successfully!"
#define ID_ALARMINFO_104028     "(MDU) Video connection abnormal, ready to re-connect!"
#define ID_ALARMINFO_104029     "(MDU) requst video successfully!"
#define ID_ALARMINFO_104030     "(MDU) request video failed!"
#define ID_ALARMINFO_104031     "Cyclic alarm happened !"
#define ID_ALARMINFO_104051     "main channel :I/O probe alarm stop !"
#define ID_ALARMINFO_104052     "main channel :moving monitor alarm stop!"
#define ID_ALARMINFO_104053     "main channel :lost Video_frame alarm stop!"
#define ID_ALARMINFO_104054     "main channel :HDD alarm stop!"
#define ID_ALARMINFO_104055     "sub channel :I/O probe alarm stop !"
#define ID_ALARMINFO_104056     "sub channel :moving monitor alarm stop!"
#define ID_ALARMINFO_104057     "sub channel :lost Video_frame alarm stop!"
#define ID_ALARMINFO_104058     "sub channel :HDD alarm stop!"
#define ID_ALARMINFO_104059     "Device update success !"
#define ID_ALARMINFO_104060     "Device update failed !"
#define ID_ALARMINFO_104061     "Device begin to update !"
#define ID_ALARMINFO_104062     "measurements info ."
#endif /*CHINESE_EDITION*/
#define  TEXT_MAXLEN	256

#ifdef __cplusplus
 }
#endif /* __cplusplus */


#endif /* CLIENTLINK_H_ */
