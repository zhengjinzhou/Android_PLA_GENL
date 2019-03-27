package com.topvs.platform;

import android.app.Application;

import com.tencent.map.geolocation.TencentGeofence;
import com.tencent.mapsdk.raster.model.Marker;

import java.util.ArrayList;

//可用来存储全局变量 
//注意使用此方法时 需在manifest中声明
// <application android:name=".MyApp" ...
public class MyApp extends Application {


    /**
     * 用于底图上显示已添加的 TencentGeofence
     */
    private static ArrayList<Marker> sFenceItems = new ArrayList<Marker>();

    /**
     * 用于在 ListView 中显示已添加的 TencentGeofence
     */
    private static ArrayList<TencentGeofence> sFences = new ArrayList<TencentGeofence>();

    /**
     * 记录已触发的 TencentGeofence 事件
     */
    private static ArrayList<String> sEvents = new ArrayList<String>();


    public static ArrayList<Marker> getFenceItems() {
        return sFenceItems;
    }

    public static ArrayList<TencentGeofence> getFence() {
        return sFences;
    }

    /**
     * 返回最新添加的围栏
     */
    public static TencentGeofence getLastFence() {
        if (sFences.isEmpty()) {
            return null;
        }
        return sFences.get(sFences.size() - 1);
    }

    public static ArrayList<String> getEvents() {
        return sEvents;
    }

    @Override
    public void onCreate() {
        super.onCreate();
//		TencentExtraKeys.setTencentLog(new TencentLog() {
//
//			@Override
//			public void println(String arg0, int arg1, String arg2) {
//				Log.i(arg0, arg2);
//			}
//
//		});
    }

    private String password;

    public String getPassword() {
        return password;
    }

    public void setPassword(String s) {
        password = s;
    }

}