package com.topvs.platform;

public class ck_AlarmInfo {
    public int devid;        //设备ID
    public int alarmtime;    //信息发生本地时间
    public float airtemp;    //气温
    public float airhumi;    //湿度
    public float soiltemp;    //土温
    public float soilhumi;    //湿度
    public float CO2density;    //CO2浓度
    public float illuminance;    //光照度

    public float water_sat;    //soil water saturation
    public float daily_rain;    //daily rainfall
    public float anion;        //
    public float pm25;
    public float wind_speed;


    @Override
    public String toString() {
        return "ck_AlarmInfo{" +
                "devid=" + devid +
                ", alarmtime=" + alarmtime +
                ", airtemp=" + airtemp +
                ", airhumi=" + airhumi +
                ", soiltemp=" + soiltemp +
                ", soilhumi=" + soilhumi +
                ", CO2density=" + CO2density +
                ", illuminance=" + illuminance +
                ", water_sat=" + water_sat +
                ", daily_rain=" + daily_rain +
                ", anion=" + anion +
                ", pm25=" + pm25 +
                ", wind_speed=" + wind_speed +
                '}';
    }
}

