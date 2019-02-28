package com.topvs.platform;

import com.topvs.wanve.view.LoginActivity;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;

public class SplashActivity extends Activity {

    //延迟3秒 
    private static final long SPLASH_DELAY_MILLIS = 200;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.splash);

        // 使用Handler的postDelayed方法，3秒后执行跳转到MainActivity 
        new Handler().postDelayed(new Runnable() {
            public void run() {
                //Intent intent = new Intent(SplashActivity.this, NVSPlayer.class);
                Intent intent = new Intent(SplashActivity.this, LoginActivity.class);
                SplashActivity.this.startActivity(intent);
                SplashActivity.this.finish();
            }
        }, SPLASH_DELAY_MILLIS);
    }
}