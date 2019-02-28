package com.topvs.wanve.bean;

import java.io.Serializable;

/**
 * Created by zhou
 * on 2019/2/25.
 */
public class UserBean implements Serializable {
    private String user;//用户名
    private String psd;//密码
    private boolean remember;//记住登录
    private boolean atomatic;//自动登录

    public String getUser() {
        return user;
    }

    public void setUser(String user) {
        this.user = user;
    }

    public String getPsd() {
        return psd;
    }

    public void setPsd(String psd) {
        this.psd = psd;
    }

    public boolean isRemember() {
        return remember;
    }

    public void setRemember(boolean remember) {
        this.remember = remember;
    }

    public boolean isAtomatic() {
        return atomatic;
    }

    public void setAtomatic(boolean atomatic) {
        this.atomatic = atomatic;
    }

    @Override
    public String toString() {
        return "UserBean{" +
                "user='" + user + '\'' +
                ", psd='" + psd + '\'' +
                ", remember=" + remember +
                ", atomatic=" + atomatic +
                '}';
    }
}
