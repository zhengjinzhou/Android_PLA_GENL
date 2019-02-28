package com.topvs.wanve.util;

import android.content.Context;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.topvs.platform.R;

/**
 * Created by zhou
 * on 2019/2/25.
 */

public class ToastUtil {

    private static TextView mTextView;
    private static ImageView mImageView;
    /*public static void show(Context context, String str){
        Toast.makeText(context,str,Toast.LENGTH_LONG).show();
    }*/

    public static void show(Context context,String message){
        //加载Toast布局
        View toastRoot = LayoutInflater.from(context).inflate(R.layout.toast, null);
        //初始化布局控件
        mTextView = (TextView) toastRoot.findViewById(R.id.message);
        mImageView = (ImageView) toastRoot.findViewById(R.id.imageView);
        mImageView.setImageResource(R.drawable.icon);
        //为控件设置属性
        mTextView.setText(message);
        //Toast的初始化
        Toast toastStart = new Toast(context);
        //获取屏幕高度
        WindowManager wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        int height = wm.getDefaultDisplay().getHeight();
        //Toast的Y坐标是屏幕高度的1/3，不会出现不适配的问题
        toastStart.setGravity(Gravity.TOP, 0, height / 3);
        toastStart.setDuration(Toast.LENGTH_LONG);
        toastStart.setView(toastRoot);
        toastStart.show();
    }
}
