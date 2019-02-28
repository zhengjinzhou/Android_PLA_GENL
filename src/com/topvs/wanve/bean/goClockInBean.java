package com.topvs.wanve.bean;

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
     * StartWorkTime :
     * EndWorkTime :
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
                '}';
    }
}
