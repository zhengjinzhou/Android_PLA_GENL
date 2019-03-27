package com.topvs.wanve.bean;

import java.util.List;

/**
 * Created by zhou
 * on 2019/2/27.
 * 考勤打卡
 */

public class goClockInBean {

    /**
     * APP_ID : 1258164618
     * App_Name : jttz
     * Secret_ID : AKIDCIrKcNkvX2c3KDSXxGTUB6PcdigsjpWB
     * Secret_Key : AKH3YzGNIhw8C8xyqa8Vp27Nip5A3qKd
     * ProNo : A-01
     * UserSNID : 20130227112138196
     * UserName : 建设单位管理员
     * StartWorkTime : 08:30
     * EndWorkTime : 18:00
     * Lon : 113.756678
     * Lat : 23.008018
     * Distance : 500
     * ProName : 万维博通大厦
     * Points : [{"name":"打卡1","lon":"131.0123","lat":"23.345"},{"name":"打卡1","lon":"131.0123","lat":"23.345"},{"name":"打卡1","lon":"131.0123","lat":"23.345"}]
     */

    private String APP_ID;
    private String App_Name;
    private String Secret_ID;
    private String Secret_Key;
    private String ProNo;
    private String UserSNID;
    private String UserName;
    private String StartWorkTime;
    private String EndWorkTime;
    private String Lon;
    private String Lat;
    private String Distance;
    private String ProName;
    private List<PointsBean> Points;

    public String getAPP_ID() {
        return APP_ID;
    }

    public void setAPP_ID(String APP_ID) {
        this.APP_ID = APP_ID;
    }

    public String getApp_Name() {
        return App_Name;
    }

    public void setApp_Name(String App_Name) {
        this.App_Name = App_Name;
    }

    public String getSecret_ID() {
        return Secret_ID;
    }

    public void setSecret_ID(String Secret_ID) {
        this.Secret_ID = Secret_ID;
    }

    public String getSecret_Key() {
        return Secret_Key;
    }

    public void setSecret_Key(String Secret_Key) {
        this.Secret_Key = Secret_Key;
    }

    public String getProNo() {
        return ProNo;
    }

    public void setProNo(String ProNo) {
        this.ProNo = ProNo;
    }

    public String getUserSNID() {
        return UserSNID;
    }

    public void setUserSNID(String UserSNID) {
        this.UserSNID = UserSNID;
    }

    public String getUserName() {
        return UserName;
    }

    public void setUserName(String UserName) {
        this.UserName = UserName;
    }

    public String getStartWorkTime() {
        return StartWorkTime;
    }

    public void setStartWorkTime(String StartWorkTime) {
        this.StartWorkTime = StartWorkTime;
    }

    public String getEndWorkTime() {
        return EndWorkTime;
    }

    public void setEndWorkTime(String EndWorkTime) {
        this.EndWorkTime = EndWorkTime;
    }

    public String getLon() {
        return Lon;
    }

    public void setLon(String Lon) {
        this.Lon = Lon;
    }

    public String getLat() {
        return Lat;
    }

    public void setLat(String Lat) {
        this.Lat = Lat;
    }

    public String getDistance() {
        return Distance;
    }

    public void setDistance(String Distance) {
        this.Distance = Distance;
    }

    public String getProName() {
        return ProName;
    }

    public void setProName(String ProName) {
        this.ProName = ProName;
    }

    public List<PointsBean> getPoints() {
        return Points;
    }

    public void setPoints(List<PointsBean> Points) {
        this.Points = Points;
    }

    public static class PointsBean {
        /**
         * name : 打卡1
         * lon : 131.0123
         * lat : 23.345
         */

        private String name;
        private String lon;
        private String lat;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getLon() {
            return lon;
        }

        public void setLon(String lon) {
            this.lon = lon;
        }

        public String getLat() {
            return lat;
        }

        public void setLat(String lat) {
            this.lat = lat;
        }

        @Override
        public String toString() {
            return "PointsBean{" +
                    "name='" + name + '\'' +
                    ", lon='" + lon + '\'' +
                    ", lat='" + lat + '\'' +
                    '}';
        }
    }

    @Override
    public String toString() {
        return "goClockInBean{" +
                "APP_ID='" + APP_ID + '\'' +
                ", App_Name='" + App_Name + '\'' +
                ", Secret_ID='" + Secret_ID + '\'' +
                ", Secret_Key='" + Secret_Key + '\'' +
                ", ProNo='" + ProNo + '\'' +
                ", UserSNID='" + UserSNID + '\'' +
                ", UserName='" + UserName + '\'' +
                ", StartWorkTime='" + StartWorkTime + '\'' +
                ", EndWorkTime='" + EndWorkTime + '\'' +
                ", Lon='" + Lon + '\'' +
                ", Lat='" + Lat + '\'' +
                ", Distance='" + Distance + '\'' +
                ", ProName='" + ProName + '\'' +
                ", Points=" + Points +
                '}';
    }
}
