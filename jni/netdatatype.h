/*
 * netdatatype.h
 *
 *  Created on: 2009-10-13
 *      Author: pc
 */

#ifndef NETDATATYPE_H_
#define NETDATATYPE_H_

#include "httpAPI.h"
#include "list.h"
// -----------------------------------------------------------------------------
// Command_Type Defination : 主动请求时的命令类型
#define CMD_UNKNOWN            "CmdUnKnown"
#define USER_LOGIN_REQ         "UserLoginReq"
#define QUERYUSERAREA          "QueryUserArea"
#define DEVICELIST_REQ         "DeviceListReq"
#define CMU_KEEPALIVE          "Heart"
#define CMU_USERLOGOUT         "UserLogoutReq"
#define MDU_KEEPALIVE          "CU_MDUKeepaliveReq"
#define CMU_REAL_START_REQ     "RealVideoStartReq"
#define CMU_REAL_STOP_REQ      "RealVideoStopReq"
#define MDU_REAL_START_REQ     "StartMediaReq"
#define MDU_REAL_STOP_REQ      "StopMediaReq"
#define CMU_DOMECTRLQUERY      "DomeCtrlQuery" // 请求云台权限和服务器
#define CSG_CONTROLPTZ         "ControlPTZ"
#define CMU_SUBALARMINFO       "SubAlarmInfo"
#define CMU_GUCONFIG           "GUConfigReq"   // 请求设置PU和服务器信息
//////////////////////////////////////////////////////////////////////////
//电视墙请求命令
#define TW_CONFIG				"TWConfig"
#define TW_CONFIG_MODIFY		"TWConfigModify"
#define TW_MODIFY_QUERY			"TWModifyQuery"
#define TW_CONFIG_DELETE		"TWConfigDelete"
#define TW_QUERYTWCONFIG		"TWQueryTwConfig"

#define TOUR_CONFIG				"TWTourConfig"
#define TOUR_CONFIG_MODIFY		"TourConfigModify"
#define TOUR_MODIFY_QUERY		"TourModifyQuery"
#define TOUR_CONFIG_DELETE		"TourConfigDelete"
#define TW_QUERYTOURCONFIG		"TWQueryTourConfig"

#define GROUP_CONFIG			"TWGroupConfig"
#define GROUP_CONFIG_MODIFY		"GroupConfigModify"
#define GROUP_MODIFY_QUERY		"GroupModifyQuery"
#define GROUP_CONFIG_DELETE		"GroupConfigDelete"
#define TW_QUERYGROUPCONFIG		"TWQueryGroupConfig"

#define QUERY_TWLIST			"QueryTWList"
#define QUERY_TOURLIST			"QueryTourList"
#define QUERY_GROUPLIST			"QueryGroupList"
#define QUERY_TWCONFIG			"QueryTWConfig"
#define QUERY_TOURCONFIG		"QueryTourConfig"
#define QUERY_GROUPCONFIG		"QueryGroupConfig"

#define STEP_RUN				"TWPlay"
#define TOUR_RUN				"TWPlayTour"
#define GROUP_RUN				"TWPlayGroup"

#define QUERY_STATE_STEP		"TWQueryStepState"
#define QUERY_STATE_TOUR		"TWQueryTourState"
#define QUERY_STATE_GROUP		"TWQueryGroupState"

#define MATRIX_STEP_START		"MatrixStepStart"
#define MATRIX_TOUR_START		"MatrixTourStart"
#define MATRIX_GROUP_START		"MatrixGroupStart"
// #define MATRIX_STEP_STOP		"MatrixStepStop"
// #define MATRIX_TOUR_STOP		"MatrixTourStop"
// #define MATRIX_GROUP_STOP		"MatrixGroupStop"

//////////////////////////////////////////////////////////////////////////
//add 2008.06.17 by wy for AMU
#define AMU_QUERY_AREA_INFO								"QueryAreaInfo"
#define AMU_QUERY_DEFENS_INFO							"QueryDefenseInfo"
#define AMU_QUERY_MAP									"QueryMap"
#define AMU_QUERY_ACTION_LINK_CONFIG				    "QueryActionLinkCfg"
#define AMU_GET_GU_POS									"GetGUPos"
#define AMU_SET_GU_BYPASS								"SetGUByPass"
#define AMU_GET_GU_BYPASS								"GetGUByPass"
#define AMU_GET_GU_TYPE									"GetGUType"
#define AMU_SET_DEFENS_STATE							"SetDefensState"
#define AMU_GET_DEFENS_STATE							"GetDefensState"
//-------------------------------------------------------------------------------

typedef struct
{
	HTTP_OBJECT httpObject;  // 包中的内容
	long long dwTime;      // 包的时间戳 ms
} PACK_ELEMENT;

typedef struct _PACK_ELEMENT_list
{
	PACK_ELEMENT pack_element;
	struct list_head list;
} PACK_ELEMENT_list;

#include "dev_type.h"

typedef frame_head_t FRAMHEAD_T;

typedef struct
{
	unsigned char version :2;
	unsigned char p :1;
	unsigned char x :1;
	unsigned char cc :4;
	unsigned char pt :7;
	unsigned char m :1;

	unsigned short seq; /* sequence number */
	unsigned int ts; /* timestamp */
	unsigned char frmType;
	unsigned char channel;
	unsigned short session;
} RTP_FIXED_HEADER;

typedef struct
{
	unsigned char reserve1;
	unsigned char reserve2;
	unsigned short len; /*rtp data lengh*/
} RTP_HEADER_EXT;

#define		MTU_LEN		1400
#define		RTP_VIDEO_SSRC				100
#define		RTP_AUDIO_SSRC				101

#define		RTP_IVIDEO_FRAME			1
#define		RTP_BVIDEO_FRAME			2
#define		RTP_PVIDEO_FRAME			3
#define		RTP_AUDIO_FRAME				4
#define		RTP_UNKNOWN_FRAME			5

#endif /* NETDATATYPE_H_ */
