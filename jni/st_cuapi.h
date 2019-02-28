/*
 * st_cuapi.h
 *
 *  Created on: 2009-10-13
 *      Author: pc
 */

#ifndef ST_CUAPI_H_
#define ST_CUAPI_H_

#include "httpAPI.h"
#include "SipAPI.h"
#include "list.h"

//#include "netglobal.h"

//#pragma comment(lib, "HttpLib.lib")
#  ifdef __cplusplus
extern "C"
{
#  endif /* __cplusplus */

	//typedef int int;
	typedef int DWORD;
	typedef int COLORREF;
	//typedef long int HRESULT;
	typedef const char *LPCTSTR;
	typedef unsigned long long ULONG64;
	typedef long int LONG;

// Error Code, 所有错误类型都为非正值
#define ST_OK                   S_OK
#define ST_FAILED               S_FALSE
#define ST_ERR_UNKNOWN          0x80000000    // 未知错误类型

#define ST_ERR_SOCKET           0x80008001
#define ST_ERR_XML              0x80008002
#define ST_COM_ERROR            0x80008003
#define ST_ERR_HTTPDOC          0x80008004
#define ST_ERR_SENDDATA         0x80008015
#define ST_ERR_DOMAIN_PASE		0x80008016	//域名解析失败

#define ST_TIMEDOUT             0x80008021
#define ST_ERR_RELOGIN          0x80008030
#define ST_ERR_REJECT           0x80008031
#define ST_ERR_SMU              0x80008032    // 这个错误表示CMU连接SMU故障

#define ST_ERR_ROUTE            0x80008033
#define ST_ERR_COMMAND			0x80008034	  //CMU不支持的命令
//the rule of port offset  in the net communication define (mdu_base_port)
#define ST_MDU_VOICE_OFFSET		1 //语音对讲时基于MDU端口的偏移量

#define VOD_BASE_MDU_OFFSET		2
#define VOD_BASE_MSU_OFFSET		1

//the rule of port offset  in the net communication define (cmu_base_port)
#define PORT_CSG_INCREASE_NUM	1	
//////////////////////////////////////////////////////////////////////////

//device type define
//1-音视频主码流，2-报警输入，3-报警输出,4-解码通道，5-显示通道 6-音视频从码流
//A-测控量-遥测，B-测控量-遥信，C-测控量-遥控，D-测控量-遥调
#define GU_TYPE_AV_MASTER           0x01
#define GU_TYPE_ALARM_INPUT         0x02
#define GU_TYPE_ALARM_OUTPUT        0x03
#define GU_TYPE_DECODE_CH           0x04
#define GU_TYPE_DISPLAY_CH          0x05
#define GU_TYPE_AV_SLAVE            0x06
#define GU_TYPE_CK_CH               0x07

#define REMOTE_MEASURE						17	//	A
#define REMOTE_SEMAPHORE					18	//	B
#define REMOTE_CONTROL						19	//	C
#define REMOTE_ADJUST						20	//	D
//alarm type define
#define NET_ALARM_PU_ONLINE						0x0000	//pu上线

#define NET_ALARM_PU_LOGOUT						0x0001	//pu下线

#define NET_ALARM_CMU_CONNECT_INTERMIT			0x0002	//CMU连接中断

#define NET_ALARM_CMU_RECONNECT_SUCCESS			0x0003	//重连CMU成功

#define NET_ALARM_VIDEO_INTERMIT				0x0004	//视频中断

#define NET_ALARM_VIDEO_RECONNECT_SUCCEED		0x0005	//重连视频成功

#define NET_ALARM_VIDEO_RECONNECT_FAILED		0x0006	//重连视频失败

#define NET_ALARM_USER_LOGIN_IN_THE_OTHER_PLACE	0x0007	//该用户已在其它地方登录

#define PU_ALARM_PRI_CHANNEL_I_O					0x0100	//主通道:I/O探头告警

#define PU_ALARM_PRI_CHANNEL_MOVE_DETECTIVE			0x0101	//主通道:移动侦测告警

#define PU_ALARM_PRI_CHANNEL_VIDEO_LOST				0x0102	//主通道:视频丢失告警

#define PU_ALARM_PRI_CHANNEL_HARDDISK				0x0104	//主通道:硬盘告警

#define PU_ALARM_PRI_CHANNEL_MDU_IP_ID_NOT_MATCH	0x0105	//主通道:MDU的IP与ID不匹配

#define PU_ALARM_SEC_CHANNEL_I_O					0x0110	//从通道:I/O探头告警

#define PU_ALARM_SEC_CHANNEL_MOVE_DETECTIVE			0x0111	//从通道:移动侦测告警

#define PU_ALARM_SEC_CHANNEL_VIDEO_LOST				0x0112	//从通道:视频丢失告警

#define PU_ALARM_SEC_CHANNEL_HARDDISK				0x0114	//从通道:硬盘告警

#define PU_ALARM_SEC_CHANNEL_MDU_IP_ID_NOT_MATCH	0x0115	//从通道:MDU的IP与ID不匹配

#define PU_ALARM_SATELLITE_MAP_ALARM				0x0199	//卫星地图告警
#define TV_ALARM									0x0200	//电视墙告警
#define PU_ALARM_UPDATE_START						0x0300	//设备开始升级
#define PU_ALARM_UPDATE_SCCESS						0x0301	//升级成功
#define PU_ALARM_UPDATE_FAILED						0x0302	//升级失败
#define PU_ALARM_REMOTE_MEEASURE					0x0400	//遥测
#define PU_ALARM_MEASUREMENT_CTRL_INFO				0X0500	//测控信息 
#define ALARM_NOT_PARSE_TYPE						0xFFFF	//无法解析的警告类型
//right-control style define
#define	RIGHT_DOMERIGHT		0x00000001		//云台控制	1	

#define	RIGHT_PARMGET		0x00000002		//参数查询	2

#define	RIGHT_PARMCONFIG	0x00000004		//参数设置  3

#define	RIGHT_AUDIOTALK		0x00000008		//语音对讲	4

#define	RIGHT_PLAYBACK		0x00000010		//历史回放	5

#define	RIGHT_ALARMRECV		0x00000020		//告警接收	6

#define	RIGHT_3DCONTROL		0x00000040		//3D球控制	7

#define	RIGHT_EQUIPIP		0x00000080		//设备IP	8

#define	RIGHT_MATRIX		0x00000100		//矩阵控制	9

	typedef void (*RECVCALLBACK)(void* lHandle, void* lpBuf, int lSize, void* lDecoderID);

	//mp4v2-2.0.0 record callback
	typedef void (*RECORDCALLBACK)(void* fileHandle, int video_id, int audio_id, void *buf, int size, int keyframe);

	//ffmpeg-2.3.0 record calbback
	typedef void (*FFMPEGRECORDCB)(void* fileHandle, int stream_index, void *buf, int size, int ftype, void* text);

	int InitilLibInstance();
	int DeleteLibInstance(int pServerPara);

	typedef struct _GUINFO {
		char DomainID[32];	//支持多域系统
		int DomainType;		//域类型： 0 本域; 1 子域; 2 父域
		char GUID[32];
		char GUName[256];
		char PUID[32];
		int GUType;

		int HasDome;
		int right;
// 		int DomeRight;
//		int EnConfig;
//		int EnTalk;
//		int EnBackPlay;
		int EnTcpPView; //应用程序赋值

		int Bypass;
		int bState;
		int dwLastTime;
	} GUINFO; // The most important struct type, include all informations of a device;

	// 2008/06/05 增加数据结构
	typedef struct _DEVICE_NODE {
		int nType; // 类型决定下面几个参数的有效性
		char DomainName[32];
		char DomainID[32];
		char AreaName[32];
		char AreaID[32];
		GUINFO guInfo;
	} DEVICE_NODE;

	typedef struct _DEVICE_LIST {
		DEVICE_NODE *nodearray; //(256)
		int nodenum;
		struct list_head list;
	} DEVICE_LIST;

	//add by LinZh107
	typedef struct _AREA_LIST{
		int childNum;
		char AreaName[32];
		char AreaID[32];
	}AREA_LIST;

	int GetDeviceList(int pServerPara, const char* lpszQueryID, int nType,
			DEVICE_NODE* pNode, int nMaxCount, int *pCount);

	typedef struct _CLIENTINFO {
		void* lHandle;			// 窗口句柄
		RECVCALLBACK lpFunc;	// 对应的回调函数指针
		RECORDCALLBACK record_Func;	//record callback
		FFMPEGRECORDCB rcFunc;
		GUINFO guInfo;			// 摄像机描述
		long nProtocolType;		// 协议类型 0-UDP 1-TCP
		void *playinst;
	} CLIENTINFO;

	typedef struct _MEDIA_DATA_INFO {
		unsigned int packages;		// 总包数
		unsigned int packages_rate;	// 包率
		unsigned int lost;			// 总丢包
		unsigned int lostrate;		// 丢包率
		unsigned int rate;			// 码率
		ULONG64 size;				// 总大小
	} MEDIA_DATA_INFO;

	typedef struct _TwAlarmInfo {
		char szTVID[32];
		char szDisplayGUID[32];
		char szDecoderGUID[32];
		int iWindowNo;
		int iRunType;				//0 单步，1 巡回，2 群组
		char szEncoderName[32];
		char szRunName[32];
	} TwAlarmInfo;

	typedef struct _AlarmInfo {
		char szDomainid[32];
		char szPuid[32];
		char szGuid[32];            // GUID
		double dwLongitude;        	// 经度
		double dwLatitude;          // 纬度
		double dwSpeed;				// 速度
		char szDeviceName[64];      // 告警源设备名称
		char szAlarmInfo[128];      // 告警内容
		char szRemarks[256];		// 告警备注信息
		int nAlarmType;			   	// 告警类型
		int dwAlarmTime;	       	// 告警发生时间
		int dwParam1;              	// 附加参数
		TwAlarmInfo twInfo;
	} AlarmInfo;

	typedef struct _AlarmInfo_list {
		AlarmInfo alarminfo;
		struct list_head list;
	} AlarmInfo_list;

	//Time-out interval, in milliseconds , 只能在ST_ClientStartUp之前调用
	void SetCmdTimeOut(int pServerPara, int dwMilliseconds);

	//通过消息传递主动通知上层应用程序(包括告警信息\视频终止信息等)
	//int  ClientStartUp(int pServerPara, unsigned int nMessage, HWND hWnd);
	int ClientCleanUp(int pServerPara);

	//登陆相关
	int GetCheckCode(int pServerPara, char* lpszServerIP, int nPort, char* lpszUserID, char* lpszUserPwd);
	int RequestLoginEx(int pServerPara, char* lpszServerIP, int nPort, char* lpszUserID, char* lpszUserPwd,
	        char* lpszLocalDomainID, char* lpRemoteIP, int* nSum, char* lpszCheckCode);

	int GetLoginFlag(int pServerPara);
	int RequestLogin(int pServerPara, char* lpszServerIP, unsigned int nPort, char* lpszUserID, char* lpszUserPwd,
	        char * lpszLocalDomainID, char * lpRemoteIP, int* nSum); // LPCTSTR ---->char*   LPTSTR--> char *
	int GrabLogin(int pServerPara);
	int RequestLogout(int pServerPara);

	int QueryDeviceList(int pServerPara, LPCTSTR lpszXmlFile);

	//实时视频浏览(2008/02/28)
	void CancelWaitting(int pServerPara);                       // 终止一个全局的阻塞调用
	void* CreateVideoInstance(int pServerPara, CLIENTINFO client);  // 创建一个视频连接实例
	int CancelVideoInstance(int pServerPara, void* lHandle);

	typedef struct tag_mdu_route_info {
		char szLocalMduIp[128];
		char szLocalMduId[32];
		unsigned int uLocalPort;
		char szRemoteCsgIp[128];
		unsigned int uRemoteCsgPort;
		char szRemoteMduIp[128];
		char szRemoteMduId[32];
		unsigned int uRemoteMduPort;
	} MDU_ROUTE_INFO;

	int QueryPuRealRoute(int pServerPara, GUINFO guInfo, MDU_ROUTE_INFO *mduInfo);
	int QueryCsgRoute(int pServerPara, GUINFO guInfo, MDU_ROUTE_INFO *mduInfo);
	//int QueryPURoute(int pServerPara, char * lpszDomanID, char * lpszPUID, char* lpszSevIp, unsigned int& uPort); //LPCTSTR->char*
	int RealVideoPreviewStart(int pServerPara, void* lHandle, MDU_ROUTE_INFO mduInfo);
	int RealVideoPreviewStop(int pServerPara, void* lHandle);      // 终止视频连接

	// 音频对讲部分
    int IsChartOpen(int pServerPara);
    // 传输协议 nProtocolType 0-UDP, 1-TCP
    int ChartWithOne(int pServerPara, CLIENTINFO* client);
    int SendVoiceData(int pServerPara, const void* lpBuf, int nLen);
    int StopChart(int pServerPara);

	//前端设备图像编码参数
	typedef struct _IMAGEENCODEPARAM {
		int video_format;           	//制式选择 0:PAL   1:NTSC
		int resolution;             	//分辨率,0:QCIF,1:CIF,2:HD1,3:D1
		int bitrate_type;           	//位率类型,0:CBR,1:VBR
		int level;            	//画质,200kbit3000kbit
		int frame_rate;			//每秒侦数
		int Iframe_interval;    //I侦间隔
		int prefer_frame;		//侦率优先
		int Qp;					//量化系数
		int encode_type;		//0:音视频流, 1:视频流, 2:音频流
	} IMAGEENCODEPARAM;

	int GetPuImageEncodePara(int pServerPara, GUINFO guInfo, IMAGEENCODEPARAM* lpParam);
	int SetPuImageEncodePara(int pServerPara, GUINFO guInfo, IMAGEENCODEPARAM param);
	int GetPuImageDisplayPara(int pServerPara, GUINFO guInfo, int* contrast, int* bright,
			int* hue, int* saturation);
	int DefaultPuImageDisplayPara(int pServerPara, GUINFO guInfo, int* contrast,
			int* bright, int* hue, int* saturation);
	int SetPuImageDisplayPara(int pServerPara, GUINFO guInfo, int contrast,
			int bright, int hue, int saturation);

	//串口参数
	typedef struct _SERIALPORTPARAM {
		int data_bit;       //数据位,值:5/6/7/8 默认8
		int stop_bit;       //停止位,值:0/1/2 默认1
		int reserve[2];
		int verify_rule;    //验证规则,-1:无,0:奇校验,1:偶校验, 默认-1
		int baut_rate;      //波特率,协议特定 默认9600
	} SERIALPORTPARAM;

	//int   GetPuSerialPort(int pServerPara, GUINFO& guInfo, int serialNo , SERIALPORTPARAM* lpParam);
	//int   SetPuSerialPort(int pServerPara, GUINFO& guInfo, int serialNo , SERIALPORTPARAM  param);

	//云台设置个与控制
	typedef struct _PTZPARAM {
		int ptz_addr;       //云台地址0 ~ 255
		int ptz_type;       //云台类型索引.PELCO_D-1,PELCO_P-2,松下-3,YAAN-4,santachi-5,santachi_in-6,矩阵-9
		int serial_no;      //串口号,值默认为1
		SERIALPORTPARAM serial_param;
		int reserve;
	} PTZPARAM;

	typedef struct _ptzcontrol_t {
		char csgIp[32];
		char cmd[32];
		char param[32];
		char msgtype[32];
		unsigned int csgport;
		int speed;
		GUINFO guInfo;
	} PTZControl;

	int GetPuPtzParam(int pServerPara, GUINFO guInfo, PTZPARAM* lpParam);
	int SetPuPtzParam(int pServerPara, GUINFO guInfo, PTZPARAM param);

	// 前端控制
	int IoControl(int pServerPara, GUINFO guInfo, int bStatus, int bReturn);//0-不返回 1-不等待回应立即返回

	//pCode：返回的错误代码
	int DomeCtrlRequest(int pServerPara, char* lpszDomainID, char* lpszGUID, int *pCode);
	int DomeControl(int pServerPara, GUINFO guInfo,	char* msgType,
			int nSpeed, const char* cmdType, const char* param);
	int RemoteAlarmEnable(int pServerPara, GUINFO guInfo, int bEnabel);

	typedef struct _CST_CULibApp {
		//added by zhusy
		char m_ServerIP[60];      		//   地址;
		char m_strRealServerIP[60];		//
		int m_nPort;
		char m_UserID[20];				//CString    m_UserID;
		char m_UserPwd[20];				//CString    m_UserPwd;
		int m_bBlocking;
		char m_strCheckCode[10];		//验证码
	} CST_CULibApp;

	extern CST_CULibApp theApp;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ST_CUAPI_H_ */
