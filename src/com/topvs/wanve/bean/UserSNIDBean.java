package com.topvs.wanve.bean;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public class UserSNIDBean {

    /**
     * Success : true
     * PhyPath : D:\WAN_MPDA_QW_wanve\UserAttachment\\UQW_2019022617485923935504945938819928663.jpg
     * VirtualPath : http://47.107.91.155:8080/Wan_MPDA_ViewQW/UserAttachment/\UQW_2019022617485923935504945938819928663.jpg
     */

    private boolean Success;
    private String PhyPath;
    private String VirtualPath;

    public boolean isSuccess() {
        return Success;
    }

    public void setSuccess(boolean Success) {
        this.Success = Success;
    }

    public String getPhyPath() {
        return PhyPath;
    }

    public void setPhyPath(String PhyPath) {
        this.PhyPath = PhyPath;
    }

    public String getVirtualPath() {
        return VirtualPath;
    }

    public void setVirtualPath(String VirtualPath) {
        this.VirtualPath = VirtualPath;
    }

    @Override
    public String toString() {
        return "UserSNIDBean{" +
                "Success=" + Success +
                ", PhyPath='" + PhyPath + '\'' +
                ", VirtualPath='" + VirtualPath + '\'' +
                '}';
    }
}
