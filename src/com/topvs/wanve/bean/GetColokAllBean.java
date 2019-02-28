package com.topvs.wanve.bean;

import java.util.List;

/**
 * Created by zhou
 * on 2019/2/28.
 */

public class GetColokAllBean {

    /**
     * Success : true
     * Message :
     * Records : [{"PM_BH":"A-01","UserSNID":"20130227112138196","UserID":"yzdw-gly","UserName":"建设单位管理员","ClockTime":"2019-02-26 18:00:03","Lat":"23.007953","Lon":"113.756641","Address":"南山区科苑路5号"},{"PM_BH":"A-01","UserSNID":"20130227112138196","UserID":"yzdw-gly","UserName":"建设单位管理员","ClockTime":"2019-02-27 17:24:04","Lat":"23.008018","Lon":"113.756678","Address":"南山区科苑路5号"},{"PM_BH":"A-01","UserSNID":"20130227112138196","UserID":"yzdw-gly","UserName":"建设单位管理员","ClockTime":"2019-02-27 17:33:08","Lat":"23.00798","Lon":"113.756659","Address":"南山区科苑路5号"},{"PM_BH":"A-01","UserSNID":"20130227112138196","UserID":"yzdw-gly","UserName":"建设单位管理员","ClockTime":"2019-02-27 17:36:30","Lat":"23.01103616","Lon":"113.75254602","Address":"南山区科苑路5号"},{"PM_BH":"A-01","UserSNID":"20130227112138196","UserID":"yzdw-gly","UserName":"建设单位管理员","ClockTime":"2019-02-28 08:48:02","Lat":"23.008569","Lon":"113.756982","Address":"南山区科苑路5号"},{"PM_BH":"A-01","UserSNID":"20130227112138196","UserID":"yzdw-gly","UserName":"建设单位管理员","ClockTime":"2019-02-28 10:40:08","Lat":"23.007976","Lon":"113.756776","Address":"南山区科苑路5号"}]
     */

    private boolean Success;
    private String Message;
    private List<RecordsBean> Records;

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

    public List<RecordsBean> getRecords() {
        return Records;
    }

    public void setRecords(List<RecordsBean> Records) {
        this.Records = Records;
    }

    public static class RecordsBean {
        /**
         * PM_BH : A-01
         * UserSNID : 20130227112138196
         * UserID : yzdw-gly
         * UserName : 建设单位管理员
         * ClockTime : 2019-02-26 18:00:03
         * Lat : 23.007953
         * Lon : 113.756641
         * Address : 南山区科苑路5号
         */

        private String PM_BH;
        private String UserSNID;
        private String UserID;
        private String UserName;
        private String ClockTime;
        private String Lat;
        private String Lon;
        private String Address;

        public String getPM_BH() {
            return PM_BH;
        }

        public void setPM_BH(String PM_BH) {
            this.PM_BH = PM_BH;
        }

        public String getUserSNID() {
            return UserSNID;
        }

        public void setUserSNID(String UserSNID) {
            this.UserSNID = UserSNID;
        }

        public String getUserID() {
            return UserID;
        }

        public void setUserID(String UserID) {
            this.UserID = UserID;
        }

        public String getUserName() {
            return UserName;
        }

        public void setUserName(String UserName) {
            this.UserName = UserName;
        }

        public String getClockTime() {
            return ClockTime;
        }

        public void setClockTime(String ClockTime) {
            this.ClockTime = ClockTime;
        }

        public String getLat() {
            return Lat;
        }

        public void setLat(String Lat) {
            this.Lat = Lat;
        }

        public String getLon() {
            return Lon;
        }

        public void setLon(String Lon) {
            this.Lon = Lon;
        }

        public String getAddress() {
            return Address;
        }

        public void setAddress(String Address) {
            this.Address = Address;
        }

        @Override
        public String toString() {
            return "RecordsBean{" +
                    "PM_BH='" + PM_BH + '\'' +
                    ", UserSNID='" + UserSNID + '\'' +
                    ", UserID='" + UserID + '\'' +
                    ", UserName='" + UserName + '\'' +
                    ", ClockTime='" + ClockTime + '\'' +
                    ", Lat='" + Lat + '\'' +
                    ", Lon='" + Lon + '\'' +
                    ", Address='" + Address + '\'' +
                    '}';
        }
    }

    @Override
    public String toString() {
        return "GetColokAllBean{" +
                "Success=" + Success +
                ", Message='" + Message + '\'' +
                ", Records=" + Records +
                '}';
    }
}
