package com.topvs.wanve.bean;

import android.text.TextUtils;

/**
 * Created by zhou
 * on 2019/2/11.
 * 腾讯人脸识别
 */

public class UserInfo {
    /** 签名有效时长，单位秒 */
    public static int EFFECTIVE_DURATION = 30 * 60;//30分钟
    public static String APP_ID = "";
    public static String BUCKET_NAME = "";
    public static String SECRET_ID = "";
    public static String SECRET_KEY = "";

    public static boolean isEmpty() {
        return TextUtils.isEmpty(APP_ID)
                || TextUtils.isEmpty(BUCKET_NAME)
                || TextUtils.isEmpty(SECRET_ID)
                || TextUtils.isEmpty(SECRET_KEY)
                || EFFECTIVE_DURATION <= 0;
    }
}
