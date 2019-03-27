package com.topvs.wanve.util;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;

/**
 * Created by zhou
 * on 2018/7/24.
 */

public class BadgeNumUtil {
    private static final String INTENT_ACTION = "android.intent.action.BADGE_COUNT_UPDATE";
    private static final String INTENT_EXTRA_BADGE_COUNT = "badge_count";
    private static final String INTENT_EXTRA_PACKAGENAME = "badge_count_package_name";
    private static final String INTENT_EXTRA_ACTIVITY_NAME = "badge_count_class_name";

    /**
     * 华为
     * @param context
     * @param num
     */
    public static void setBadgeNum(Context context, int num){
        try{
            Bundle bunlde =new Bundle();
            bunlde.putString("package", context.getPackageName()); // com.test.badge is your package name
            bunlde.putString("class", "com.topvs.wanve.view.LoginActivity"); // com.test. badge.MainActivityis your apk main activity
            bunlde.putInt("badgenumber",num);
            context.getContentResolver().call(Uri.parse("content://com.huawei.android.launcher.settings/badge/"), "change_badge", null, bunlde);
        }catch(Exception e){
            Log.d("-----", "setBadgeNum: "+e.getMessage());
        }
    }

    /**
     * 其他？
     * @param context
    context     * @param badgeCount
     */
    public static void setBadgeCount(Context context, int badgeCount) {
        Intent intent = new Intent(INTENT_ACTION);
        intent.putExtra(INTENT_EXTRA_BADGE_COUNT, badgeCount);
        intent.putExtra(INTENT_EXTRA_PACKAGENAME, context.getPackageManager().getLaunchIntentForPackage(context.getPackageName()).getComponent().getPackageName());
        intent.putExtra(INTENT_EXTRA_ACTIVITY_NAME, context.getPackageManager().getLaunchIntentForPackage(context.getPackageName()).getComponent().getClassName());
        context.sendBroadcast(intent);
    }
}
