/*
 * LibAPI.java
 * nvsplayer
 * Packed by LinZh107 at 2014-11-5 
 * Copyright 2014 TOPVS. All rights reserved.
 * remark: make sure this file was  put to package named “com.topvs.plarform”
 * */

package com.topvs.platform;

import android.graphics.Bitmap;

public class LibAPI {
	
	static{     
        System.loadLibrary("nvsplayer");
    }
	
	
	//初始化  使用so库时 首先调用InitLibInstance进行初始化
	//不在使用时 调用DeleteLibInstance 释放内存
	
    public native static int  InitLibInstance();
    /* 接口函数说明： 初始化解码器和网络库
     * 参数：无
     *
     * 返回值：成功返回：Instance handler， 失败返回 -1。
     * */
    
    
    public native static int  RequestLogin(String  ip_addr,int port, String user_name,String password);
    /* 接口函数说明： 请求登录
     * 参数：
     * String			ip_addr        	//平台ip地址
     * int				port        	//平台网络端口(默认:9100)
     * String          	user_name        	//登录用户名
     * String          	password    	//登录密码
     *
     * 返回值：成功返回：0， 失败返回值及原因如下。
     * 				case -1 :
						str = "已经被登录";
						break;
					case -2 :
					case -3 :					
					case -4 :
						str = "网络连接超时";
						break;
					case -5:							
					case -6:							
					case -7:
						str = "服务端没有回应";
						break;
					case -8:
					case -9:
						str = "网络不够通畅，解析服务信息失败";
						break;
					case -10:
						str = "密码错误..";
						break;
					case -11:
						str = "SMU服务器无连接..";
						break;							
					case -12:
						str = "该用户已在其他地方登陆..";
						break;	 
					case -13:
						str = "登陆请求被拒绝, 用户名未激活";
						break;	 
					case -14:
						str = "登陆请求被拒绝, 用户名已超过使用期限";
						break;	 
					case -15:
						str = "共享用户数超出限定值";
						break;	  
					default :
						str = "登录设备失败，错误码" + ret; 
						break;
     * */
    

    //public native static String[]  GetDeviceList(int[] intArray);   
    //modify by LinZh107 2015-07-01 for Net IO/Control
    public native static void GetDeviceList(int DevNumArray[], int CamStatusArray[], int SWiStatusArray[], int SWoStatusArray[],
    		int AreaChildArray[], String[] ArrayAreaStr, String[] ArrayGUID, String[] ArrayGUName);
    /* 接口函数说明： 获取设备列表，仅返回目录和音视频节点，不同于iOS平台
     * 参数：
     * int		DevNumArray[3]  	//[out] 存储各种设备节点数
              		DevNumArray[0]	存储该平台的总摄像头数(默认MAX_NODE_NUM最大支持到1000个）
              		DevNumArray[1]	存储该平台的网络开关输入数
                    DevNumArray[1]	存储该平台的网络开关输出数
     * int 		CamStatusArray[]	//[out] 存储摄像头在线与否状态，对应于DevNumArray[0]
     * int		SWiStatusArray[]	//[out] 存储网络开关输入状态， 对应于DevNumArray[1]
     * int		SWoStatusArray[]	//[out] 存储网络开关输出状态， 对应于DevNumArray[2]
     * int 		AreaChildArray[]	//[out]	AreaChildArray[0] -- AreaChildArray[99] 用于存储对应区域下面的设备数,
     * 								//AreaChildArray[100]存储区域个数.
     * String	ArrayAreaStr[]	//[out]	存储区域组的名字	
     * String	ArrayGUID[]		//[out] 存储设备的ID，（设备隶属于某个Area下）
     * String	ArrayGUName[]	//[out]	存储设备的名字。
     * 
     * */
    
    public native static int  StartPlay(int index, int viindex);
    /* 接口函数说明： 播放实时视频 index表示选中的第几项节点，从0开始
     * 参数：
     * int      index       	//节点标识，范围为:0 ~ intArray[0]-1;
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值：成功返回：0， 失败返回 值和对应说明如下
     * 				case -3:
						Toast.makeText(DeviceListActivity.this, "RealVideo Preview Start error!", Toast.LENGTH_SHORT).show();
						break;
					case -11:
						Toast.makeText(DeviceListActivity.this, "Create StartReq CMD error!", Toast.LENGTH_SHORT).show();
						break;
					case -12:
						Toast.makeText(DeviceListActivity.this, "Wait for response timeout!", Toast.LENGTH_SHORT).show();
						break;
					case -13:
					case -14:
						Toast.makeText(DeviceListActivity.this, "Failed to analyze the ipAddr!", Toast.LENGTH_SHORT).show();
						break;
     * */
    
    
    public native static int  StopPlay(int viindex);
    /* 接口函数说明：停止播放实时视频
     * 参数：无
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值：成功返回：0。
     * */   
    
 
    public native static int  GetFrameNum(int viindex);
    /* 接口函数说明： 获取本地缓冲的视频帧数
     * 参数：无
     * int		viindex  		//[in] view index, for multiple view support
     *
     * 返回值： 返回缓冲链表中的视频帧数 num。
     * */
    
    
  //2014-5-12 LinZh107 更换解bmp方案
    public native static int GetVideoFrame(int[] vParams, Bitmap bitmap, int viindex);
    /* 接口函数说明： 获取1帧视频帧(已经解码成完整的位图数据)。区别于iOS
     * 参数：
     * int[] 	vParams     //[in/out] 存放视频分辨率 width:_vParams[0] height:_vParams[1]
     * Bitmap 	bitmap   		//[out] java可显示的位图对象的引用
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值： 成功返回：帧速率frame_rate， 失败返回 <0 值。
     * 		example:
     * 		public int videoRect[] = {352, 288}; // 352x288 为初始化值，获取一个帧以后此数组将保存真正的视频分辨率
     * 		
     * 		while(!isExit)
     * 		{
     * 				if (videoRect[0] < scaleWidth) // 107.视频分辨率小于屏幕，底层SWS_SCALE不进行bitmap尺寸变换
					{
						mBitmap = Bitmap.createBitmap(videoRect[0], videoRect[1], Bitmap.Config.RGB_565);
					}
					else
					// 107.视频分辨率 >= 屏幕，底层 SWS_SCALE 进行bitmap缩小变换
					{
						videoRect[0] = scaleWidth;
						videoRect[1] = scaleHeight;
						mBitmap = Bitmap.createBitmap(videoRect[0], videoRect[1], Bitmap.Config.RGB_565);
					}

					frame_rate = LibAPI.getVideoFrame(videoRect, mBitmap); // 底层创建的位图没有进行缩放
					.....
					render
					.....
			}
     */
    
    
    public native static int  GetAudioParam(int[] AudioParams, int viindex);	//add by  LinZh107增加语音输出
    /* 接口函数说明： 获取音频帧结构参数,区别于iOS
     * 参数：
     * int 		AudioParams[2]	//AudioParams[0]*AudioParams[1]表示一个音频帧的数据(PCM)长度;
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值： 0表示成功， <0表示失败。
     * */
    
    
    public native static short[]  GetAudioFrame(int frame_cnt, int viindex);//add by  LinZh107增加语音输出
    /* 接口函数说明： 获取音频帧,区别于iOS
     * 参数：
     * int 		frame_cnt		//一次获取的帧数
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值： 成功返回PCM格式的short数组，长度为 frame_cnt * AudioParams[0] * AudioParams[1]， 失败返回NULL。
     * 			音频编码参数如下：
      			sampleRateInHz = 8000;
      			bufferSize = frame_cnt * AudioParams[0] * AudioParams[1];
				track = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRateInHz, AudioFormat.CHANNEL_OUT_MONO,
			        AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
     * */
    

    public native static int  DomeControl(int index, int state, int iSpeed, String cmd);
    /* 接口函数说明： 云台转向及镜头焦距控制。
     * 参数：
     * int 			index		//[in] 摄像头标识，见StartPlay()
     * int          state       //[in] 1为开始动作，0为停止动作
     * int          iSpeed      //[in] 云台动作的速率，范围为0--10,推荐为5
     * String       cmd         //[in] 云台动作命令标识，可取值如下：
                                        "PL" ---> 左旋转
                                        "PR" ---> 右旋转
                                        "TU" ---> 上扬
                                        "TD" ---> 下摆
                                        "ZOUT" ---> 焦距拉远，缩小
                                        "ZIN" ---> 焦距拉近，放大                                
     *
     * 返回值： 成功返回：0， 失败返回 <0 的值。
     * */
    
        
    public native static int  StartRecord(String path, int viindex);
    /* 接口函数说明： 开始监控录像
     * 参数：
     * String	path      		//[in] 保存完整路径
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值： 成功返回 0， 失败返回 <0 的值。
     * */
    
    
    public native static int  StopRecord(int viindex);
    /* 接口函数说明： 停止监控录像
     * 参数：无
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值：立刻成功时返回 0， 返回-1时会应等待到I帧出现为止在退出。时间长度视帧率和I帧间隔而定，2-8s不等
     * */
    
    
    // 播放录像 path录像文件的路径 ary[0]返回帧格式  ary[1]帧率
    public native static int  OpenRecordFile(String path, int viindex);
    /* 接口函数说明： 回放本地录像，只是将文件按读进缓冲，不做显示
     * 参数：
	 * String		path
	 * int[] 		outAry			//new int[2];
	 * 									outAry[0] 表示 video_format;
	 * 												VIDEO_FORMAT_D1	
													VIDEO_FORMAT_HD1
													VIDEO_FORMAT_CIF
													VIDEO_FORMAT_QCIF
										outAry[1] 表示 frame_rate;
     * 
     * 返回值：成功时返回 0， 失败时返回-1
     * remark：
     * 	调用该函数后还要依次循环调用getRecordFrame、 GetVideoFrame 和 GetAudioFrame等函数来获取实际帧数据并渲染显示
     * */
        
    //deprecated
    public native static int  GetRecordFrame(boolean[] isExit, int viindex);
    /* 接口函数说明：将录像的音视频帧读进缓冲区
     * 参数：
	 * boolean[] 	isExit		//[in] app请求退出标志，控制不同线程的同步
	  							//public boolean[] isExit = new boolean[1];true为退出标志
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值： 返回值小于0表示已到录像文件末尾	>=0 表示获取了一帧数据
     * remark：
     * 	调用该函数后还要依次循环调用getRecordFrame、 GetVideoFrame 和 GetAudioFrame等函数来获取实际帧数据并渲染显示
     * 	其中getRecordFrame、 GetVideoFrame 和 GetAudioFrame都应该为独立的线程里
     * */
    

    //replace the GetRecordFrame() by LinZh107 on 2015-8-21
    public native static int  GetRecordVideoFrame(int params, int[] vParams, Bitmap bitmap, int viindex);
    /* 接口函数说明： 获取1帧视频帧(已经解码成完整的位图数据) 
     * 参考 GetVideoFrame() 接口。
     * 参数：
     * int 		params		//struct mp4TracksParam_t instance handle.
     * 
     * */ 
    
    
    public native static short[]  GetRecordAudioFrame(int params, int frame_cnt, int viindex);
    /* 接口函数说明： 从录制文件中获取音频帧,
     * 参考 GetAudioFrame() 接口
     * 参数：
     * int 		params		//struct mp4TracksParam_t instance handle.
     */		
    	
    
    public native static int  CloseRecordFile(int params, int viindex);
    /* 接口函数说明： 回放本地录像
     * 参数：无
     * int		viindex  		//[in] view index, for multiple view support
     * 
     * 返回值：成功时返回 0， 失败时返回-1
     * */
    
    
	// bEnble 0表示撤防 1表示布防
    public native static int RemoteAlarmEnable(int index,int bEnble); 
    /* 接口函数说明：远程布防开关
     * 参数：
     * int		index		// 节点标识，见 StartPlay()
     * int		bEnble		// bEnble取值  0表示撤防   1表示布防
     * 
     * 返回值：成功时返回 0， 失败时返回  < 0
     * */
    
    
    public native static int GetdevAlarmInfo(Object alarminfo); //add 2014-6-5 LinZh107
    /* 接口函数说明：获取测控信息，应定时刷新，即将此函数放入一个timer里面
     * 参数：无
     * 		注意:此函数不带参，但是要求java层有一个alarminfo对象用于存放数据，如：
     * 		public static ck_AlarmInfo alarminfo = new ck_AlarmInfo();
     * 		ck_AlarmInfo类结构如下:
     * 			package com.topvs.plarform;
				public class ck_AlarmInfo
				{
					public int devid;		//设备ID
					public int alarmtime;	//信息发生本地时间
					
					public float airtemp;	//气温
					public float airhumi;	//湿度
					public float soiltemp;	//土温
					public float soilhumi;	//湿度
					public float CO2density;	//CO2浓度
					public float illuminance;	//光照度	
				}
     * 
     * 返回值：成功时返回 0， 失败时返回  < 0
     * */
    
    
    public native static int  RequestLogout();
    /* 接口函数说明：登出设备
     * 参数：无
     * 
     * 返回值：成功时返回 0
     * */
    
    
    public native static int  DeleteLibInstance();
    /* 接口函数说明： 释放解码器并释放内存
     * 参数：无
     *
     * 返回值：成功返回：0。 异常放回 -1
     * */
    
    
    public native static int StartChart(int curindex, int viindex);
    //2015-06-16  	LinZh107
    /* 接口函数说明：创建对讲线程, 主要处理的是将接收的音频放入列表(接收后还是要用以上GetAudioParam()来解码，不处理发送)
     * 参数：
     * int      curindex    	//相机节点标识，范围为:0 ~ intArray[0]-1;
     * int		viindex  		//[in] view index, for multiple view support
     *
     * 返回值：成功返回：0， 失败返回-1。
     * */
    
    public native static int SendVoiceData(short[] sArray, int dataLen);
    //2015-06-16  	LinZh107
    /* 接口函数说明：发送音频到前端
     * 参数：
     * short[] sArray       //原始的音频数据(即PCM数据,不包含任何头)
     * int bufLen			//sizeof VoiceArray[] (PCM长度)
     *
     * 返回值：成功返回：发送长度(bufLen)， 失败返回  < 0。
     * */
    
    public native static int StopChart();
    //2015-06-16  	LinZh107
    /* 接口函数说明：退出对讲线程
     * 参数：无
     *
     * 返回值：成功返回：0， 失败返回-1。
     * */
    
    public native static int IoControl(int iNodeIndex, int bStatus, int bReturn);
    //2015-07-02  	LinZh107
    /* 接口函数说明：NetWork Switch/IO control
     * 参数：
     * int          iNodeIndex  //[in] io index of devlistNode
     * int          bStatus     //[in] 1为开始动作，0为停止动作
     * int          bReturn     //[in] return immediately, equal to set or get flags
     *
     * 返回值： 成功返回：0， 失败返回 <0 的值。
     * */
    
    public native static int setDecoderParam(int viindex, int videoMaxWidth, int decodRate);
    //2015-07-10  	LinZh107
    /* 接口函数说明：	set decode Param for ffmpegLib to decode HD-frame without delay
     * 参数：
     * int		viindex  		//[in] view index, for multiple view support
     * int		videoMaxWidth   //[in] if the video width larger then this value define it's a HD-frame
     * int		decodRate     	//[in] while video width bigger then this width, abandon some
     * 									frame to make sure the decoding work without delay.
     *
     * 返回值： 成功返回：0， 失败返回 <0 的值。
     * */
    
    //*********************************************************************
    //以下为保留接口

    //最后一个参数code为验证码
    //public native static int  RequestLoginEx(String  ip_addr,int port,String user_name,String pwd,String code);
    
    //从平台获取验证码
    //public native static int  GetCheckCode(String  ip_addr,int port,String user_name,String pwd);

    //视频参数
    //  brightness <--> byteArray[0];//亮度值,范围:0-255
	//  contrast   <--> byteArray[1];//对比度值,范围:0-255
	//  hue        <--> byteArray[2];//色调值,范围:0-255
	//  saturation <--> byteArray[3];		//饱和度值,范围:0-255
    public native static int  GetPuImageDisplayPara(int index,int[] ary);//设置
    
    
    public native static int  SetPuImageDisplayPara(int index,int[] ary);//获取
    
    //编码参数
    // video_format <--> intArray[0];
	//resolution    <--> intArray[1];
	//bitrate_type  <--> intArray[2];
	//level		    <-->  intArray[3];
	//frame_rate    <-->  intArray[4];
	//Iframe_interval  <-->  intArray[5];
	//prefer_frame	   <-->  intArray[6];
	//Qp   			   <--> intArray[7];
	//encode_type      <--> intArray[8];
    public native static int  GetPuImageEncodePara(int index,int[] intArray);
    
    public native static int  SetPuImageEncodePara(int index,int[] intArray);
    //***********************************************************************
    

    public native static int  SetPUinfo(String msg);
    
}
