package com.topvs.wanve.bean;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public class goCCTVBean {


    /**
     * XMZ_SNID : XMZ_20160202101943593
     * ProNo : A-01
     * CameraID :
     * CameraPwd :
     */

    private String XMZ_SNID;
    private String ProNo;
    private String CameraID;
    private String CameraPwd;

    public String getXMZ_SNID() {
        return XMZ_SNID;
    }

    public void setXMZ_SNID(String XMZ_SNID) {
        this.XMZ_SNID = XMZ_SNID;
    }

    public String getProNo() {
        return ProNo;
    }

    public void setProNo(String ProNo) {
        this.ProNo = ProNo;
    }

    public String getCameraID() {
        return CameraID;
    }

    public void setCameraID(String CameraID) {
        this.CameraID = CameraID;
    }

    public String getCameraPwd() {
        return CameraPwd;
    }

    public void setCameraPwd(String CameraPwd) {
        this.CameraPwd = CameraPwd;
    }

    @Override
    public String toString() {
        return "goCCTVBean{" +
                "XMZ_SNID='" + XMZ_SNID + '\'' +
                ", ProNo='" + ProNo + '\'' +
                ", CameraID='" + CameraID + '\'' +
                ", CameraPwd='" + CameraPwd + '\'' +
                '}';
    }
}
