package com.topvs.wanve.bean;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public class goCCTVBean {


    /**
     * XMZ_SNID : XMZ_2017090716445440
     * ProNo : 0036
     */

    private String XMZ_SNID;
    private String ProNo;

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

    @Override
    public String toString() {
        return "goCCTVBean{" +
                "XMZ_SNID='" + XMZ_SNID + '\'' +
                ", ProNo='" + ProNo + '\'' +
                '}';
    }
}
