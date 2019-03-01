package com.topvs.wanve.view;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import com.topvs.platform.DeviceListActivity;
import com.topvs.platform.LibAPI;
import com.topvs.platform.R;
import com.topvs.wanve.base.Constant;
import com.topvs.wanve.bean.UserBean;
import com.topvs.wanve.util.SpUtil;
import com.topvs.wanve.util.ToastUtil;
import com.topvs.wanve.util.WebServiceUtil;
import com.topvs.wanve.widget.LoadDialog;

import org.ksoap2.serialization.SoapObject;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.StringReader;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class LoginActivity extends Activity {
    public final static int REQUEST_READ_PHONE_STATE = 1;
    private EditText et_user;
    private EditText et_psd;
    private CheckBox cb_remember;
    private CheckBox cb_automaticity;
    private boolean isRemember;
    private boolean isAutomaticity;
    private String etUser;
    private String etPsd;
    private static final String TAG = "LoginActivity";
    protected LoadDialog dialog;
    int m_InSDCard = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);


        init();
    }

    /**
     * 初始化的
     */
    private void init() {

        dialog = new LoadDialog(this,false,"加载中...");

        initView();
        //Android5.0以上状态栏颜色修改
        if(Build.VERSION.SDK_INT >= 21){
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    |View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
            getWindow().setStatusBarColor(Color.TRANSPARENT);
        }

        //权限申请
        initPermission();

        initCheckBox();
        initInfo();
    }

    /**
     * 获取缓存
     */
    private void initInfo() {
        UserBean userBean = (UserBean) SpUtil.getObject(this, Constant.UserBean,UserBean.class);
        if (userBean != null && userBean.isAtomatic()){
            etUser = userBean.getUser();
            etPsd = userBean.getPsd();
            FirstLogin(userBean);
            return;
        }
        if (userBean != null && userBean.isRemember()){
            cb_remember.setChecked(userBean.isRemember());
            et_user.setText(userBean.getUser());
            et_psd.setText(userBean.getPsd());
        }
    }

    /**
     * 获取viewid
     */
    private void initView() {
        et_user = findViewById(R.id.et_user);
        et_psd = findViewById(R.id.et_psd);
        cb_remember = findViewById(R.id.cb_remember);
        cb_automaticity = findViewById(R.id.cb_automaticity);

        findViewById(R.id.bt_login).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                LoginWebServer();
            }
        });

        //视频监控
        findViewById(R.id.bt_Spjk).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                initPLALogin();
            }
        });
    }

    /**
     * 视频监控的登录
     */
    boolean m_bInited = false;
    private void initPLALogin() {
        /*m_name = editUser.getText().toString();
        m_pwd = editPwd.getText().toString();
        m_ipAddr = editIPAddr.getText().toString();112.74.94.235:8089
        m_port = Integer.parseInt(editPort.getText().toString());*/
        String m_name = "wane";
        String m_pwd = "wanve";
        String m_ipAddr = "112.74.94.235";
        int m_port = 9901;
        if (!m_bInited) {
            if (LibAPI.InitLibInstance() != -1)
                m_bInited = true;
            else
                m_bInited = false;
        }
        if (m_bInited == true) {
            int ret = 0;
            ret = LibAPI.RequestLogin(m_ipAddr, m_port, m_name, m_pwd);
            Log.d(TAG, "LibAPI.RequestLogin return " + ret);
            if (ret != 0) {
                String str = "";
                switch (ret) {
                    case -1:
                        str = "已经被登录";
                        break;
                    case -2:
                    case -3:
                    case -4:
                        str = "网络连接超时";
                        break;
                    case -5:
                    case -6:
                    case -7:
                        str = "服务端没有回应";
                        break;
                    case -8:
                    case -9:
                        str = "网络不够通畅，解析服务信息失败";
                        break;
                    case -10:
                        str = "密码错误..";
                        break;
                    case -11:
                        str = "SMU服务器无连接..";
                        break;
                    case -12:
                        str = "该用户已在其他地方登陆..";
                        break;
                    case -13:
                        str = "登陆请求被拒绝, 用户名未激活";
                        break;
                    case -14:
                        str = "登陆请求被拒绝, 用户名已超过使用期限";
                        break;
                    case -15:
                        str = "共享用户数超出限定值";
                        break;
                    default:
                        str = "登录设备失败，错误码" + ret;
                        break;
                }
                ToastUtil.show(this,str);
            } else {
                Intent in = new Intent(this, DeviceListActivity.class);
                in.putExtra("INSDCARD", m_InSDCard);
                startActivity(in);
            }
        }
    }

    /**
     * 初次登录
     * <p>
     * 使用webServer
     */
    private void LoginWebServer() {
        final UserBean userBean = new UserBean();
        etUser = et_user.getText().toString().trim();
        etPsd = et_psd.getText().toString().trim();

        /*********************下面这里只对南山生效**********************/
        /*if (etPsd.length() < 7){
            ToastUtil.show(getApplicationContext(), "密码强度不够，请到后台更改！");
            return;
        }

        Pattern p = Pattern.compile("^[A-Za-z]+$");
        Matcher m = p.matcher(etPsd);
        boolean isValid = m.matches();
        if (isValid){
            ToastUtil.show(getApplicationContext(), "密码强度不够，请到后台更改！");
            return;
        }

        Pattern pattern = Pattern.compile("[0-9]*");
        Matcher isNum = pattern.matcher(etPsd);
        if (isNum.matches()){
            ToastUtil.show(getApplicationContext(), "密码强度不够，请到后台更改！");
            return;
        }*/
        /*********************上面这里只对南山生效**********************/

        userBean.setPsd(etPsd);
        userBean.setUser(etUser);
        userBean.setRemember(isRemember);
        userBean.setAtomatic(isAutomaticity);
        Log.d(TAG, "et_psd: " + etPsd);
        if (TextUtils.isEmpty(etUser)) {
            ToastUtil.show(getApplicationContext(), "用户名不能为空");
            return;
        }
        if (TextUtils.isEmpty(etPsd)) {
            ToastUtil.show(getApplicationContext(), "用户名不能为空");
            return;
        }
        FirstLogin(userBean);//初次都能来
    }

    /**
     * 动态权限申请
     */
    private void initPermission() {

        List<String> permissionList = new ArrayList<>();
        if (ContextCompat.checkSelfPermission(this,Manifest.permission.WRITE_EXTERNAL_STORAGE) != PERMISSION_GRANTED){
            permissionList.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        }
        if (ContextCompat.checkSelfPermission(this,Manifest.permission.READ_EXTERNAL_STORAGE) != PERMISSION_GRANTED){
            permissionList.add(Manifest.permission.READ_EXTERNAL_STORAGE);
        }
        if (ContextCompat.checkSelfPermission(this,Manifest.permission.READ_PHONE_STATE) != PERMISSION_GRANTED){
            permissionList.add(Manifest.permission.READ_PHONE_STATE);
        }
        if (ContextCompat.checkSelfPermission(this,Manifest.permission.ACCESS_COARSE_LOCATION) != PERMISSION_GRANTED){
            permissionList.add(Manifest.permission.ACCESS_COARSE_LOCATION);
        }

        if (!permissionList.isEmpty()){  //申请的集合不为空时，表示有需要申请的权限
            ActivityCompat.requestPermissions(this,permissionList.toArray(new String[permissionList.size()]),1);
        }else { //所有的权限都已经授权过了

        }
    }

    //勾选框逻辑
    private void initCheckBox() {
        cb_remember.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                isRemember = isChecked;
            }
        });
        cb_automaticity.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                isAutomaticity = isChecked;
                cb_remember.setChecked(isChecked);
            }
        });
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        switch (requestCode) {
            case REQUEST_READ_PHONE_STATE:
                if (grantResults.length > 0){ //安全写法，如果小于0，肯定会出错了
                    for (int i = 0; i < grantResults.length; i++) {

                        int grantResult = grantResults[i];
                        if (grantResult == PackageManager.PERMISSION_DENIED){ //这个是权限拒绝
                            String s = permissions[i];
                            ToastUtil.show(this,s+"权限被拒绝了,请自行到应用管理权限开启");
                        }else{ //授权成功了
                            //do Something
                        }
                    }
                }
                break;
            default:
                break;
        }
    }

    /**
     * 初次登录
     * @param userBean
     */
    private void FirstLogin(final UserBean userBean) {
        HashMap<String, String> data = new HashMap<>();
        data.put("userID", etUser);
        data.put("userPSW", etPsd);
        dialog.show();
        WebServiceUtil.callWebService(Constant.LOGIN_URL, "CheckUserLogin", data, new WebServiceUtil.WebServiceCallBack() {
            @Override
            public void callBack(SoapObject result) {
                Log.d(TAG, "callBack: " + Constant.LOGIN_URL);
                if (result != null) {
                    Log.d(TAG, "callBack: " + result.toString());
                    String login = result.toString();
                    String substring = login.substring(44);
                    Log.d(TAG, "callBack: "+substring);
                    if (substring.contains("OK")) {
                        SpUtil.putObject(LoginActivity.this,Constant.UserBean,userBean);
                        SpUtil.putString(LoginActivity.this,"isok","OK");//保存ok
                        TwoLogin(etUser);
                        return;
                    }
                    else if (substring.contains("ok")) {
                        SpUtil.putObject(LoginActivity.this,Constant.UserBean,userBean);
                        SpUtil.putString(LoginActivity.this,"isok","ok");//保存ok
                        TwoLogin(etUser);
                        return;
                    }
                    else if (substring.contains("fail")) {
                        ToastUtil.show(getApplicationContext(), "密码错误");
                        dialog.dismiss();
                        return;
                    } else if (substring.contains("noExists")) {
                        ToastUtil.show(getApplicationContext(), "账号不存在");
                        dialog.dismiss();
                        return;
                    } else if (substring.contains("isLock")) {
                        ToastUtil.show(getApplicationContext(), "账号被锁");
                        dialog.dismiss();
                        return;
                    } else if (substring.contains("sysErr")) {
                        ToastUtil.show(getApplicationContext(), "系统异常");
                        dialog.dismiss();
                        return;
                    } else if (substring.contains("noOpenUse")) {
                        ToastUtil.show(getApplicationContext(), "账号被禁用");
                        dialog.dismiss();
                        return;
                    }
                } else {
                    ToastUtil.show(getApplicationContext(), "请求失败");
                }
            }
        });
    }

    /**
     * 第二次验证登录
     * <p>
     * 验证服务器是否变更
     */
    private void TwoLogin(final String user) {
        MediaType MEDIA_TYPE_MARKDOWN = MediaType.parse("text/xml; charset=utf-8");//根据C#大牛那边写的头文件 以及登录验证方式
        OkHttpClient okHttpClient = new OkHttpClient();
        String psw = URLEncoder.encode(Constant.PSW);
        Log.d(TAG, "加密后的密码: " + psw);
        final String strXML = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" +
                "<REQUEST>" +
                "<SP_ID>ToEIM_PIC</SP_ID>" +
                "<PASSWORD>" + psw + "</PASSWORD>" +
                "<USER>"+user+"</USER>" +
                "</REQUEST>";
        Request request = new Request.Builder().url(Constant.ssoUrl)
                .post(RequestBody.create(MEDIA_TYPE_MARKDOWN, strXML)).build();
        okHttpClient.newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                Log.d("", "onFailure: 第二次登录失败 " + e.getMessage());
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                String string = response.body().string();
                SpUtil.putString(getApplicationContext(),Constant.UserName,user);
                Log.d(TAG, "onResponse: 第二次登录成功" + string);
                try {
                    parseXMLWithPull(string);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    //xml解析
    public void parseXMLWithPull(String xmlData) throws Exception {
        XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
        XmlPullParser parser = factory.newPullParser();
        parser.setInput(new StringReader(xmlData));
        int eventType = parser.getEventType();
        String resp_code = "";
        String resp_desc = "";
        String websession = "";

        while (eventType != XmlPullParser.END_DOCUMENT) {
            String nodeName = parser.getName();
            //Log.d(TAG, "parseXMLWithPull: "+nodeName);
            switch (eventType) {
                // 开始解析某个结点
                case XmlPullParser.START_TAG: {
                    if ("WEBSESSION".equals(nodeName)) {
                        websession = parser.nextText();
                    } else if ("RESP_CODE".equals(nodeName)) {
                        resp_code = parser.nextText();
                    } else if ("RESP_DESC".equals(nodeName)) {
                        resp_desc = parser.nextText();
                    }
                    break;
                }
                // 完成解析某个结点
                case XmlPullParser.END_TAG: {
                    if ("RESPONSE".equals(nodeName)) {
                        Log.d("MainActivity", "WEBSESSION is " + websession);
                        Log.d("MainActivity", "RESP_CODE is " + resp_code);
                        Log.d("MainActivity", "RESP_DESC is " + resp_desc);
                        if (resp_code.equals("0000")) {
                            //最后一次验证，即为第三次验证
                            startActivity(HomeActivity.newIntent(getApplicationContext(), Constant.iniUrl + websession));
                            dialog.dismiss();
                            finish();
                        } else {
                            final String des = resp_desc;
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    ToastUtil.show(getApplicationContext(), des);
                                    dialog.dismiss();
                                }
                            });
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            eventType = parser.next();
        }
    }
}
