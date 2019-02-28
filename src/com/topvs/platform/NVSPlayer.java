package com.topvs.platform;

import com.topvs.platform.R;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

public class NVSPlayer extends Activity {
    static final String TAG = "NVSPLayer";
    private ProgressDialog progressDialog = null;

    private DatabaseHelper dbHelper = null;
    public static final int IPLIST_ID = Menu.FIRST;
    public static final int EXIT_ID = Menu.FIRST + 1;
    private Button btnLogin;
    private Button btnSetting;
    EditText editUser;
    EditText editPwd;
    EditText editIPAddr;
    EditText editPort;
    //	EditText editPlatform;
    CheckBox saveBox;

    public static final String PREFS_NAME = "MyPrefsFile";
    //	String m_platform = null;
    String m_ipAddr = null;
    int m_port = 0;
    String m_pwd = null;
    String m_name = null;
    boolean m_bSavePWD;
    int m_InSDCard;

    static final int IPCONFIG_REQUEST = 0;
    boolean m_bInited = false;
//	private DatabaseHelper dbHelper = null;

    private Resources rs;

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        setContentView(R.layout.main);
        rs = getResources();

        btnLogin = (Button) findViewById(R.id.btn_login);
        btnLogin.setOnClickListener(listener);

        btnSetting = (Button) findViewById(R.id.btn_setting);
        btnSetting.setOnClickListener(listener);

        // 载入配置文件
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);
        m_ipAddr = settings.getString("config_ipAddr", "");
        m_port = settings.getInt("config_port", 9901);
        m_name = settings.getString("config_user", "");
        m_pwd = settings.getString("config_pwd", "");

//		m_platform = settings.getString("config_platform", "");		
        m_InSDCard = settings.getInt("config_InSDCard", 0);
        m_bSavePWD = settings.getBoolean("config_savePWD", true);

        if (!m_ipAddr.isEmpty())
            btnSetting.setText(rs.getString(R.string.ipconf_plat) + ": " + m_ipAddr);

        saveBox = (CheckBox) findViewById(R.id.btn_savepwd);
        if (m_bSavePWD)
            saveBox.setChecked(true);

        editUser = (EditText) findViewById(R.id.configusr);
        editUser.setText(m_name);

        if (m_pwd.endsWith("107"))
            m_pwd = "";
        editPwd = (EditText) findViewById(R.id.configpwd);
        editPwd.setText(m_pwd);

        editIPAddr = (EditText) findViewById(R.id.configipaddr);
        editIPAddr.setText(m_ipAddr);
        editIPAddr.setVisibility(View.INVISIBLE); // Date: 2013-10-23 Author: yms

        editPort = (EditText) findViewById(R.id.configloginport);
        editPort.setText(String.valueOf(m_port));
        editPort.setVisibility(View.INVISIBLE); // Date: 2013-10-23 Author:yms

//		editPlatform = (EditText) findViewById(R.id.configplatname);
//		editPlatform.setText(String.valueOf(m_platform));
//		editPlatform.setVisibility(View.INVISIBLE); // Date: 2013-10-23 Author: yms

        // 获取屏幕密度（方法3）
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        dbHelper = new DatabaseHelper(this);
    }

    protected void LoginEvent() {
        m_name = editUser.getText().toString();
        m_pwd = editPwd.getText().toString();
        m_ipAddr = editIPAddr.getText().toString();
        m_port = Integer.parseInt(editPort.getText().toString());
//		m_platform = editPlatform.getText().toString();
        Log.d(TAG, "name:" + m_name + " pwd:" + m_pwd + " ipAddr:" + m_ipAddr + " port:" + m_port);

        if (m_ipAddr.equals("") || m_port == 0)
            Toast.makeText(NVSPlayer.this, "Wrong IP or Port.", Toast.LENGTH_SHORT).show();
        else if (m_name.isEmpty() || m_pwd.equals(""))
            Toast.makeText(NVSPlayer.this, rs.getString(R.string.app_hintpwd), Toast.LENGTH_SHORT).show();
        else if (!m_bInited) {
            if (LibAPI.InitLibInstance() != -1)
                m_bInited = true;
            else
                m_bInited = false;
        }

		/*
         * URL myurl; try { myurl = new URL("http://"+addr);
		 * HttpURLConnection httpconn = (HttpURLConnection)
		 * myurl.openConnection(); httpconn.setConnectTimeout(6000);
		 * httpconn.setReadTimeout(6000);
		 * 
		 * if (httpconn.getResponseCode() != HttpURLConnection.HTTP_OK)
		 * { Toast.makeText( NVSPlayer.this, "连接服务器失败。"+addr,
		 * Toast.LENGTH_LONG).show(); return; }
		 * 
		 * } catch (Exception e) { // TODO Auto-generated catch block
		 * Toast.makeText( NVSPlayer.this, "连接服务器失败。",
		 * Toast.LENGTH_LONG).show(); return; }
		 */
        if (m_bInited == true) {
            progressDialog = ProgressDialog.show(NVSPlayer.this,
                    rs.getString(R.string.dev_loading),
                    rs.getString(R.string.dev_loading_device));
            int ret = 0;
            ret = LibAPI.RequestLogin(m_ipAddr, m_port, m_name, m_pwd);
            progressDialog.dismiss();
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
                Toast.makeText(NVSPlayer.this, str, Toast.LENGTH_SHORT).show();
            } else {
                // 保存密码
//				MyApp appState = ((MyApp) getApplicationContext());
//				appState.setPassword(m_pwd);

                Intent in = new Intent(NVSPlayer.this, DeviceListActivity.class);
                in.putExtra("INSDCARD", m_InSDCard);
                startActivity(in);
            }
        }
    }

    OnClickListener listener = new OnClickListener() {
        public void onClick(View v) {
            if (v.getId() == R.id.btn_login) {
                LoginEvent();
                m_bInited = false;
            } else if (v.getId() == R.id.btn_setting) {
                Log.d(TAG, "ipAddr:" + m_ipAddr + " port:" + m_port);
                Intent in = new Intent(NVSPlayer.this, IPListActivity.class);
                in.putExtra("ipAddr", m_ipAddr);
                in.putExtra("port", m_port);
                in.putExtra("INSDCARD", m_InSDCard);
                startActivityForResult(in, IPCONFIG_REQUEST);
            }
        }
    };

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == IPCONFIG_REQUEST) {
            if (resultCode == RESULT_OK) {
                m_ipAddr = (String) data.getCharSequenceExtra("IP");
                m_port = data.getIntExtra("PORT", 9901);
                //m_platform = (String) data.getCharSequenceExtra("PLATFORM");
                m_name = (String) data.getCharSequenceExtra("USER");
                m_pwd = (String) data.getCharSequenceExtra("PWD");

                m_InSDCard = data.getIntExtra("INSDCARD", 0);

                editUser.setText(m_name);
                editPwd.setText(m_pwd);
                editIPAddr.setText(m_ipAddr);
                editPort.setText(String.valueOf(m_port));
                //editPlatform.setText(m_platform);
                btnSetting.setText(rs.getString(R.string.ipconf_plat) + ": " + m_ipAddr);
            }
        }
    }

    @Override
    protected void onStop() {
        super.onStop();

        // 写入配置文件。可以使用SharedPreferences.Editor来辅助解决。
        SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);

        SharedPreferences.Editor editor = settings.edit();
        editor.putString("config_ipAddr", m_ipAddr);
        editor.putInt("config_port", m_port);
        //editor.putString("config_platform", m_platform);
        editor.putInt("config_InSDCard", m_InSDCard);
        m_bSavePWD = saveBox.isChecked();
        editor.putBoolean("config_savePWD", m_bSavePWD);
        if (m_bSavePWD) {
            editor.putString("config_user", editUser.getText().toString());
            editor.putString("config_pwd", editPwd.getText().toString());
            Log.d(TAG, "after saved PWD!");
        } else {
            editor.putString("config_user", m_name);
            editor.putString("config_pwd", "107");
            Log.d(TAG, "Do not save PWD!");
        }
        editor.commit(); // 一定要记得提交
    }

    protected void onDestroy() {
        super.onDestroy();
        System.gc();
        dbHelper.close();
    }
}
