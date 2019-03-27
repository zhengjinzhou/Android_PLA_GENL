package com.topvs.wanve.service;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.util.Log;

import com.google.gson.Gson;
import com.topvs.wanve.base.Constant;
import com.topvs.wanve.bean.NumBean;
import com.topvs.wanve.bean.UserBean;
import com.topvs.wanve.util.BadgeNumUtil;
import com.topvs.wanve.util.SpUtil;

import java.io.IOException;
import java.util.List;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

/**
 * Created by zhou
 * on 2019/3/19.
 */


public class StartService extends Service implements Runnable{
    private Thread thread;
    private int num = 0;
    public StartService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }

    //创建服务时候调用，第一次创建
    @Override
    public void onCreate() {
        super.onCreate();
        Log.d("MyServer", "onCreate: 创建服务");
        //onCreate的时候创建初始化
        thread = new Thread( this);
        thread.start();
    }
    //每次服务启动调用，每次启动
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d("MyServer", "onCreate: 启动服务");
        //如果服务并停止了，重新生成一个新的
        if(thread.isInterrupted()){
            thread = new Thread(this);
            thread.start();
        }
        return Service.START_STICKY;
        //return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void run() {
        int i=0;
        while (true){
            try {
                //每10秒钟进行一次输出
                Thread.sleep(10000);
                UserBean userBean = (UserBean) SpUtil.getObject(this, Constant.UserBean, UserBean.class);
                if (userBean == null)
                    return;
                String user = userBean.getUser();
                String url = Constant.BASE_URL + "/wan_mpda_pic/Handlers/CommFieldManagementHandler.ashx?action=GetUserTodoCount&UserId="+user;
                OkHttpClient okHttpClient = new OkHttpClient();
                Request request = new Request.Builder().url(url).build();
                okHttpClient.newCall(request).enqueue(new Callback() {
                    @Override
                    public void onFailure(Call call, IOException e) {

                    }

                    @Override
                    public void onResponse(Call call, Response response) throws IOException {
                        String string = response.body().string();
                        if (string==null)return;
                        Log.d("打印", "onResponse: "+string);
                        Gson gson = new Gson();
                        NumBean numBean = gson.fromJson(string, NumBean.class);
                        BadgeNumUtil.setBadgeNum(getApplicationContext(),numBean.getCount());
                        BadgeNumUtil.setBadgeCount(getApplicationContext(),numBean.getCount());
                    }
                });
                // openApp("com.zhou.sslgcj");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

    }

    //    private void openApp(View v, String packageName) {
    private void openApp(String packageName) {
        //Context context = v.getContext();
        PackageInfo pi = null;
        //PackageManager pm = context.getPackageManager();
        PackageManager pm = getPackageManager();
        try {
            pi = pm.getPackageInfo(packageName, 0);
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }

        Intent resolveIntent = new Intent(Intent.ACTION_MAIN, null);
        resolveIntent.addCategory(Intent.CATEGORY_LAUNCHER);
        resolveIntent.setPackage(pi.packageName);

        List<ResolveInfo> apps = pm.queryIntentActivities(resolveIntent, 0);

        ResolveInfo ri = apps.iterator().next();
        if (ri != null ) {
            packageName = ri.activityInfo.packageName;
            String className = ri.activityInfo.name;

            Intent intent = new Intent(Intent.ACTION_MAIN);
            intent.addCategory(Intent.CATEGORY_LAUNCHER);

            ComponentName cn = new ComponentName(packageName, className);
            intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            intent.setComponent(cn);
            startActivity(intent);
        }
    }

    //服务销毁的时候
    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d("MyServer", "onCreate: 销毁服务");
        Intent intent = new Intent("com.zhou.service.destroy");
        sendBroadcast(intent);
    }
}
