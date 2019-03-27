package com.topvs.wanve.view;

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.topvs.platform.R;
import com.topvs.wanve.base.Constant;
import com.topvs.wanve.bean.UserInfo;
import com.topvs.wanve.bean.goClockInBean;
import com.topvs.wanve.util.SpUtil;
import com.topvs.wanve.view.fragment.DaKaFragment;
import com.topvs.wanve.view.fragment.DkFragment;
import com.topvs.wanve.view.fragment.JlFragment;
import android.support.v4.app.FragmentTransaction;
public class RlsbActivity extends AppCompatActivity implements DaKaFragment.CallBackValue {

    private DaKaFragment dkFragment;
    private JlFragment jlFragment;
    private Fragment[] fragments;
    private int currentTabIndex = 0;
    private TextView tvJl;
    private TextView tvDaka;
    private FrameLayout fragment_content;
    private FragmentTransaction trx;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_rlsb);

        initView();
        init();
    }

    /**
     * 获取到的控件id
     */
    private void initView() {
        fragment_content = findViewById(R.id.fragment_content);
        tvDaka = findViewById(R.id.tvDaka);
        tvJl = findViewById(R.id.tvJl);

        findViewById(R.id.iv_back).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });


        findViewById(R.id.tvDaka).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showTabFragment(0);
            }
        });

        findViewById(R.id.tvJl).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showTabFragment(1);
            }
        });
    }


    private void init() {
        dkFragment = new DaKaFragment();
        jlFragment = new JlFragment();

        fragments = new Fragment[]{dkFragment,jlFragment};

        trx = getSupportFragmentManager().beginTransaction();
        trx.add(R.id.fragment_content,dkFragment)
                .hide(dkFragment)
                .add(R.id.fragment_content,jlFragment)
                .hide(jlFragment)
                .show(dkFragment)
                .commit();

        setCheckColor(0);

        initUserInfo();
    }

    //腾讯人脸识别初始化
    private void initUserInfo() {
        goClockInBean bean = (goClockInBean) SpUtil.getObject(this, Constant.goClockIn, goClockInBean.class);
        Log.d("腾讯人脸识别初始化", "initUserInfo: "+bean.toString());
        //请根据你的具体情况填写以下信息，否则无法体验
        UserInfo.APP_ID = bean.getAPP_ID();
        UserInfo.BUCKET_NAME = bean.getApp_Name();
        UserInfo.SECRET_ID = bean.getSecret_ID();
        UserInfo.SECRET_KEY = bean.getSecret_Key();
        UserInfo.EFFECTIVE_DURATION = 1800; //签名有效时长，单位秒

    }

    private void showTabFragment(int pos){
        FragmentTransaction trx = getSupportFragmentManager().beginTransaction();
        trx.hide(fragments[currentTabIndex]);
        setUnCheckColor(currentTabIndex);
        if(!fragments[pos].isAdded()){
            trx.add(R.id.fragment_content,fragments[pos]);
        }
        setCheckColor(pos);
        trx.show(fragments[pos]);
        trx.commit();
        currentTabIndex = pos;
    }

    public void setFragment(int pos){
        showTabFragment(pos);
        setCheckColor(pos);
    }

    private void setCheckColor(int pos) {
        switch (pos){
            case 0:
                tvDaka.setBackgroundColor(getResources().getColor(R.color.white));
                tvDaka.setTextColor(getResources().getColor(R.color.bt_rlsb));

                tvJl.setBackgroundColor(getResources().getColor(R.color.bt_rlsb));
                tvJl.setTextColor(getResources().getColor(R.color.white));
                break;
            case 1:
                tvDaka.setBackgroundColor(getResources().getColor(R.color.bt_rlsb));
                tvDaka.setTextColor(getResources().getColor(R.color.white));

                tvJl.setBackgroundColor(getResources().getColor(R.color.white));
                tvJl.setTextColor(getResources().getColor(R.color.bt_rlsb));
                break;
        }
    }

    private void setUnCheckColor(int currentTabIndex) {

    }

    @Override
    public void SendMessageValue(String strValue) {
        Log.d("", "SendMessageValue: "+strValue);
        showTabFragment(1);
    }
}
