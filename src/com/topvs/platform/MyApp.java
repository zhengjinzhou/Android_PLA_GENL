package com.topvs.platform;

import android.app.Application;

//可用来存储全局变量 
//注意使用此方法时 需在manifest中声明
// <application android:name=".MyApp" ...
public class MyApp extends Application
{
	private String	password;
	public String getPassword()
	{
		return password;
	}

	public void setPassword(String s)
	{
		password = s;
	}

}