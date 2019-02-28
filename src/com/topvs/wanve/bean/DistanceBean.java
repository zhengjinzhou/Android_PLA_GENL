package com.topvs.wanve.bean;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public class DistanceBean {

    /**
     * Success : false
     * Message : 您所处位置距离工程项目太远,无法签到！
     */

    private boolean Success;
    private String Message;

    public boolean isSuccess() {
        return Success;
    }

    public void setSuccess(boolean Success) {
        this.Success = Success;
    }

    public String getMessage() {
        return Message;
    }

    public void setMessage(String Message) {
        this.Message = Message;
    }

    @Override
    public String toString() {
        return "DistanceBean{" +
                "Success=" + Success +
                ", Message='" + Message + '\'' +
                '}';
    }
}
