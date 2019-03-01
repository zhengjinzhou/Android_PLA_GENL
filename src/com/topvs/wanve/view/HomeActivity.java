package com.topvs.wanve.view;

import android.Manifest;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.ClipData;
import android.content.ComponentName;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Parcelable;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.support.annotation.RequiresApi;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.CookieManager;
import android.webkit.CookieSyncManager;
import android.webkit.DownloadListener;
import android.webkit.JavascriptInterface;
import android.webkit.JsResult;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;
import com.google.gson.Gson;
import com.topvs.platform.DeviceListActivity;
import com.topvs.platform.LibAPI;
import com.topvs.platform.NVSPlayer;
import com.topvs.platform.R;
import com.topvs.wanve.base.Constant;
import com.topvs.wanve.bean.goCCTVBean;
import com.topvs.wanve.bean.goClockInBean;
import com.topvs.wanve.util.SpUtil;
import com.topvs.wanve.util.ToastUtil;
import com.topvs.wanve.widget.LoadDialog;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.IOException;
import java.io.StringReader;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.List;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.MediaType;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;

public class HomeActivity extends Activity {

    private TextView tv_index;
    private TextView tv_map;
    private TextView tv_sum;
    private TextView tv_center;
    private WebView webView;
    private LinearLayout ll;
    private String websession;
    private String[] brief_url;
    private Uri imageUri;
    private final static int FILECHOOSER_RESULTCODE = 1;// 表单的结果回调</span>
    private static final String TAG = "MainActivity";
    private ValueCallback<Uri[]> mUploadCallbackAboveL;
    private ValueCallback<Uri> mUploadMessage;// 表单的数据信息
    protected LoadDialog dialog;
    private ImageView iv_login;
    private LinearLayout layout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        dialog = new LoadDialog(this,false,"加载中...");
        initView();
        init();
        initOnClick();
    }

    /**
     * 点击事件
     */
    private void initOnClick() {
        findViewById(R.id.ivHome).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                /**勤智的不同*/
                //TwoLogin("capital_index");
                /**勤智的不同*/
                TwoLogin("projectList");
            }
        });

        findViewById(R.id.iv_login).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                newDialog();
            }
        });

        findViewById(R.id.tv_index).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin("projectList");
            }
        });

        findViewById(R.id.tv_map).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showPopupProject();
            }
        });
        findViewById(R.id.tv_sum).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showPopupSum();
            }
        });

        findViewById(R.id.tv_center).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showPopupCenter();
            }
        });
    }

    /**
     * 退出与切换账号
     */
    private void newDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        AlertDialog dialog = builder.create();
        View inflate = LayoutInflater.from(this).inflate(R.layout.dialog_new, null);
        dialog.setView(inflate, 0, 0, 0, 0);
        dialog.show();
        inflate.findViewById(R.id.tv_signout).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });
        inflate.findViewById(R.id.tv_change).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SpUtil.clear();
                startActivity(new Intent(getApplicationContext(),LoginActivity.class));
                finish();
            }
        });
    }

    /**
     * 弹出框 统计报表
     */
    private void showPopupSum() {
        View contentView = LayoutInflater.from(this).inflate(R.layout.pop_sum, null);
        final PopupWindow pop = new PopupWindow(contentView,
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, true);
        pop.setContentView(contentView);
        pop.setWidth(ViewGroup.LayoutParams.WRAP_CONTENT);
        pop.setHeight(ViewGroup.LayoutParams.WRAP_CONTENT);
        pop.setBackgroundDrawable(new BitmapDrawable());
        pop.setOutsideTouchable(true);
        pop.setAnimationStyle(R.anim.mypop_anim);
        pop.showAtLocation(layout, Gravity.BOTTOM, 0, layout.getHeight());
        //按项目状态
        contentView.findViewById(R.id.tv_pro_stat).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[6]);
                pop.dismiss();
            }
        });
        //项目科室
        contentView.findViewById(R.id.tv_department).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[7]);
                pop.dismiss();
            }
        });
        //项目类别
        contentView.findViewById(R.id.tv_type).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[8]);
                pop.dismiss();
            }
        });
        //项目投资
        contentView.findViewById(R.id.tv_tze).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[9]);
                pop.dismiss();
            }
        });
        //汇总统计
        contentView.findViewById(R.id.tv_reference).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[10]);
                pop.dismiss();
            }
        });
        //品牌公开
        contentView.findViewById(R.id.tv_brand).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[11]);
                pop.dismiss();
            }
        });

    }

    /**
     * 第二次验证登录
     * <p>
     * 验证服务器是否变更
     *
     * @param s
     */
    private void TwoLogin(final String s) {
        dialog.show();
        String username = SpUtil.getString(getApplicationContext(), Constant.UserName, "");
        MediaType MEDIA_TYPE_MARKDOWN = MediaType.parse("text/xml; charset=utf-8");//根据C#大牛那边写的头文件 以及登录验证方式
        OkHttpClient okHttpClient = new OkHttpClient();
        String psw = URLEncoder.encode(Constant.PSW);
        Log.d(TAG, "加密后的密码: " + psw);
        final String strXML = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" +
                "<REQUEST>" +
                "<SP_ID>ToEIM_PIC</SP_ID>" +
                "<PASSWORD>" + psw + "</PASSWORD>" +
                "<USER>" + username + "</USER>" +
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
                Log.d(TAG, "onResponse: 第二次登录成功" + string);
                try {
                    parseXMLWithPull(string, s);//对xml的解析
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
    }

    /**
     * xml的pull解析
     *
     * @param xmlData
     * @throws Exception
     */
    public void parseXMLWithPull(String xmlData, final String s) throws Exception {
        XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
        XmlPullParser parser = factory.newPullParser();
        parser.setInput(new StringReader(xmlData));
        int eventType = parser.getEventType();
        String resp_code = "";
        String resp_desc = "";
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
                            //startActivity(WebActivity.newIntent(getApplicationContext(), Constant.iniUrl + websession + "&OpenType=" + s));
                            Log.d(TAG, "parseXMLWithPull: 111111111111111111111111111111");
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    initWeb(Constant.iniUrl + websession + "&OpenType=" + s);
                                }
                            });
                            dialog.dismiss();
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
    /**
     * 弹出框  项目
     */
    private void showPopupProject() {
        TwoLogin(brief_url[0]);
    }

    /**
     * 弹出框 知识中心
     */
    private void showPopupCenter() {
        View contentView = LayoutInflater.from(this).inflate(R.layout.pop_center, null);
        final PopupWindow pop = new PopupWindow(contentView,
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, true);

        pop.setBackgroundDrawable(new BitmapDrawable());
        pop.setOutsideTouchable(true);
        pop.setAnimationStyle(R.anim.mypop_anim);
        //pop.showAsDropDown(tv_center, Gravity.TOP, 0);
        pop.showAtLocation(layout, Gravity.BOTTOM, layout.getHeight(), layout.getHeight());

        //内部资料
        contentView.findViewById(R.id.tv_reference_private).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick: " + brief_url[1]);
                TwoLogin(brief_url[1]);
                pop.dismiss();
            }
        });
        //图书期刊
        contentView.findViewById(R.id.tv_book).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Log.d(TAG, "onClick: " + brief_url[2]);
                TwoLogin(brief_url[2]);
                pop.dismiss();
            }
        });
        //常用表格
        contentView.findViewById(R.id.tv_table).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[3]);
                pop.dismiss();
            }
        });
        //单位规章
        contentView.findViewById(R.id.tv_unit).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[4]);
                pop.dismiss();
            }
        });
        //标准规范
        contentView.findViewById(R.id.tv_standard).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                TwoLogin(brief_url[5]);
                pop.dismiss();
            }
        });
    }
    /**
     * 获取控件id
     */
    private void initView() {
        tv_index = findViewById(R.id.tv_index);
        tv_map = findViewById(R.id.tv_map);
        tv_sum = findViewById(R.id.tv_sum);
        tv_center = findViewById(R.id.tv_center);
        webView = findViewById(R.id.webView);
        ll = findViewById(R.id.ll);
        iv_login = findViewById(R.id.iv_login);
        layout = findViewById(R.id.layout);
    }

    private void init() {
        //判断
        String isok = SpUtil.getString(HomeActivity.this, "isok", "");
        if (isok.equals("ok")) {
            tv_map.setVisibility(View.GONE);
            ll.setVisibility(View.GONE);
        }
        initWeb(getIntent().getStringExtra(Constant.NEW_URL));
        /**只有执勤的如此*/
        //TwoLogin("capital_index");
        /**只有执勤的如此*/
        initBottom();
        brief_url = getResources().getStringArray(R.array.brief_url);
    }

    /**
     * 底部栏
     */
    private void initBottom() {
        tv_index.setTextColor(getResources().getColor(R.color.txt_2));
        Drawable img = tv_index.getResources().getDrawable(R.drawable.index_on);
        // 调用setCompoundDrawables时，必须调用Drawable.setBounds()方法,否则图片不显示
        img.setBounds(0, 0, img.getMinimumWidth(), img.getMinimumHeight());
        tv_index.setCompoundDrawables(null, img, null, null); //设置左图标
    }

    /**
     * 跳转
     *
     * @param context
     * @param url
     * @return
     */
    public static Intent newIntent(Context context, String url) {
        Intent intent = new Intent(context, HomeActivity.class);
        intent.putExtra(Constant.NEW_URL, url);
        return intent;
    }
    /**
     * webView的设置
     */
    @SuppressLint("JavascriptInterface")
    private void initWeb(String new_url) {
        Log.d(TAG, "initWeb: " + new_url);
        webView.getSettings().setUseWideViewPort(true);
        webView.getSettings().setLoadWithOverviewMode(true);
        webView.getSettings().setJavaScriptEnabled(true);//加载JavaScript
        webView.setWebViewClient(mWebViewClient);//这个一定要设置，要不然不会再本应用中加载
        webView.setWebChromeClient(mWebChromeClient);
        webView.getSettings().setSupportZoom(true);
        //清除缓存
        webView.addJavascriptInterface(this, "appHandler");
        webView.getSettings().setCacheMode(WebSettings.LOAD_NO_CACHE);
        this.deleteDatabase("WebView.db");
        this.deleteDatabase("WebViewCache.db");
        webView.clearCache(true);
        webView.clearFormData();

        getCacheDir().delete();

        webView.loadUrl(new_url);

        webView.setDownloadListener(new MyWebViewDownLoadListener());
        webView.setWebChromeClient(new WebChromeClient() {

            @Override
            public void onProgressChanged(WebView view, int newProgress) {
                super.onProgressChanged(view, newProgress);
            }

            @Override
            public boolean onShowFileChooser(WebView webView, ValueCallback<Uri[]> filePathCallback, FileChooserParams fileChooserParams) {
                mUploadCallbackAboveL = filePathCallback;
                take();
                return true;
            }

            public void openFileChooser(ValueCallback<Uri> uploadMsg) {
                mUploadMessage = uploadMsg;
                take();
            }

            public void openFileChooser(ValueCallback<Uri> uploadMsg, String acceptType) {
                mUploadMessage = uploadMsg;
                take();
            }

            public void openFileChooser(ValueCallback<Uri> uploadMsg, String acceptType, String capture) {
                mUploadMessage = uploadMsg;
                take();
            }
        });
    }

    @JavascriptInterface
    public void goClockIn (String json){
        //考勤打卡
        startActivity(new Intent(getApplicationContext(),RlsbActivity.class));
        Log.d(TAG, "goClockIn: "+json);
        Gson gson = new Gson();
        goClockInBean daka = gson.fromJson(json, goClockInBean.class);
        SpUtil.putObject(this,Constant.goClockIn,daka);
    }

    @JavascriptInterface
    public void goCCTV (String json){
        //视频监控
        //initPLALogin();
        //startActivity(new Intent(getApplicationContext(),NVSPlayer.class));
        Log.d(TAG, "goCCTV: "+json);
        Gson gson = new Gson();
        goCCTVBean goCCTVBean = gson.fromJson(json, goCCTVBean.class);
        LoginEvent(goCCTVBean.getCameraID(),goCCTVBean.getCameraPwd());
        m_bInited = false;
    }

    /**
     * 视频监控的登录
     */
    int m_InSDCard = 0;
    boolean m_bInited = false;
    private void LoginEvent(String m_name, String m_pwd) {
       /* String m_name = "wane";
        String m_pwd = "wanve";*/
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
                        str = "该项目未配置视频监控设备，请联系管理员！";
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

    private void take() {

        if (ContextCompat.checkSelfPermission(HomeActivity.this,
                android.Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(HomeActivity.this,
                    new String[]{android.Manifest.permission.CAMERA}, 0);
        } else {
            File imageStorageDir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "MyApp");
            // Create the storage directory if it does not exist
            if (!imageStorageDir.exists()) {
                imageStorageDir.mkdirs();
            }
            File file = new File(imageStorageDir + File.separator + "IMG_" + String.valueOf(System.currentTimeMillis()) + ".jpg");
            imageUri = Uri.fromFile(file);

            final List<Intent> cameraIntents = new ArrayList<Intent>();
            final Intent captureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
            final PackageManager packageManager = getPackageManager();
            final List<ResolveInfo> listCam = packageManager.queryIntentActivities(captureIntent, 0);
            for (ResolveInfo res : listCam) {
                final String packageName = res.activityInfo.packageName;
                final Intent i = new Intent(captureIntent);
                i.setComponent(new ComponentName(res.activityInfo.packageName, res.activityInfo.name));
                i.setPackage(packageName);
                i.putExtra(MediaStore.EXTRA_OUTPUT, imageUri);
                cameraIntents.add(i);
            }
            Intent i = new Intent(Intent.ACTION_GET_CONTENT);
            i.addCategory(Intent.CATEGORY_OPENABLE);
            i.setType("image/*");
            Intent chooserIntent = Intent.createChooser(i, "Image Chooser");
            chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS, cameraIntents.toArray(new Parcelable[]{}));
            HomeActivity.this.startActivityForResult(chooserIntent, FILECHOOSER_RESULTCODE);
        }
    }
    /**
     * 如果要实现文件下载的功能，需要设置WebView的DownloadListener，通过实现自己的DownloadListener来实现文件的下载
     */
    private class MyWebViewDownLoadListener implements DownloadListener {
        @Override
        public void onDownloadStart(String url, String userAgent, String contentDisposition, String mimetype, long contentLength) {
            Log.i("tag", "url=" + url);
            Log.i("tag", "userAgent=" + userAgent);
            Log.i("tag", "contentDisposition=" + contentDisposition);
            Log.i("tag", "mimetype=" + mimetype);
            Log.i("tag", "contentLength=" + contentLength);
            Uri uri = Uri.parse(url);
            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            startActivity(intent);
        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case 0:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    //这里已经获取到了摄像头的权限，想干嘛干嘛了可以
                    File imageStorageDir = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "MyApp");
                    // Create the storage directory if it does not exist
                    if (!imageStorageDir.exists()) {
                        imageStorageDir.mkdirs();
                    }
                    File file = new File(imageStorageDir + File.separator + "IMG_" + String.valueOf(System.currentTimeMillis()) + ".jpg");
                    imageUri = Uri.fromFile(file);

                    final List<Intent> cameraIntents = new ArrayList<Intent>();
                    final Intent captureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
                    final PackageManager packageManager = getPackageManager();
                    final List<ResolveInfo> listCam = packageManager.queryIntentActivities(captureIntent, 0);
                    for (ResolveInfo res : listCam) {
                        final String packageName = res.activityInfo.packageName;
                        final Intent i = new Intent(captureIntent);
                        i.setComponent(new ComponentName(res.activityInfo.packageName, res.activityInfo.name));
                        i.setPackage(packageName);
                        i.putExtra(MediaStore.EXTRA_OUTPUT, imageUri);
                        cameraIntents.add(i);

                    }
                    Intent i = new Intent(Intent.ACTION_GET_CONTENT);
                    i.addCategory(Intent.CATEGORY_OPENABLE);
                    i.setType("image/*");
                    Intent chooserIntent = Intent.createChooser(i, "Image Chooser");
                    chooserIntent.putExtra(Intent.EXTRA_INITIAL_INTENTS, cameraIntents.toArray(new Parcelable[]{}));
                    HomeActivity.this.startActivityForResult(chooserIntent, FILECHOOSER_RESULTCODE);
                } else {
                    //这里是拒绝给APP摄像头权限，给个提示什么的说明一下都可以。
                    ToastUtil.show(HomeActivity.this,"请手动打开相机权限");
                }
                break;
            default:
                break;
        }

    }

    @SuppressLint("NewApi")
    @TargetApi(Build.VERSION_CODES.KITKAT)
    public static String getPath(final Context context, final Uri uri) {
        final boolean isKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;

        // DocumentProvider
        if (isKitKat && DocumentsContract.isDocumentUri(context, uri)) {
            // ExternalStorageProvider
            if (isExternalStorageDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                if ("primary".equalsIgnoreCase(type)) {
                    return Environment.getExternalStorageDirectory() + "/" + split[1];
                }

                // TODO handle non-primary volumes
            }
            // DownloadsProvider
            else if (isDownloadsDocument(uri)) {

                final String id = DocumentsContract.getDocumentId(uri);
                final Uri contentUri = ContentUris.withAppendedId(Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));

                return getDataColumn(context, contentUri, null, null);
            }
            // MediaStore (and general)
            else if ("content".equalsIgnoreCase(uri.getScheme())) {
                return getDataColumn(context, uri, null, null);
            }
            // File
            else if ("file".equalsIgnoreCase(uri.getScheme())) {
                return uri.getPath();
            }

            // MediaProvider
            else if (isMediaDocument(uri)) {
                final String docId = DocumentsContract.getDocumentId(uri);
                final String[] split = docId.split(":");
                final String type = split[0];

                Uri contentUri = null;
                if ("image".equals(type)) {
                    contentUri = MediaStore.Images.Media.EXTERNAL_CONTENT_URI;
                } else if ("video".equals(type)) {
                    contentUri = MediaStore.Video.Media.EXTERNAL_CONTENT_URI;
                } else if ("audio".equals(type)) {
                    contentUri = MediaStore.Audio.Media.EXTERNAL_CONTENT_URI;
                }

                final String selection = "_id=?";
                final String[] selectionArgs = new String[]{split[1]};
                return getDataColumn(context, contentUri, selection, selectionArgs);
            }
        }

        return null;
    }

    public static String getDataColumn(Context context, Uri uri, String selection, String[] selectionArgs) {
        Cursor cursor = null;
        final String column = "_data";
        final String[] projection = {column};

        try {
            cursor = context.getContentResolver().query(uri, projection, selection, selectionArgs, null);
            if (cursor != null && cursor.moveToFirst()) {
                final int column_index = cursor.getColumnIndexOrThrow(column);
                return cursor.getString(column_index);
            }
        } finally {
            if (cursor != null) cursor.close();
        }
        return null;
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is ExternalStorageProvider.
     */
    public static boolean isExternalStorageDocument(Uri uri) {
        return "com.android.externalstorage.documents".equals(uri.getAuthority());
    }

    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is DownloadsProvider.
     */
    public static boolean isDownloadsDocument(Uri uri) {
        return "com.android.providers.downloads.documents".equals(uri.getAuthority());
    }


    /**
     * @param uri The Uri to check.
     * @return Whether the Uri authority is MediaProvider.
     */
    public static boolean isMediaDocument(Uri uri) {
        return "com.android.providers.media.documents".equals(uri.getAuthority());
    }

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == FILECHOOSER_RESULTCODE) {
            if (null == mUploadMessage && null == mUploadCallbackAboveL) return;
            Uri result = data == null || resultCode != RESULT_OK ? null : data.getData();
            if (mUploadCallbackAboveL != null) {
                onActivityResultAboveL(requestCode, resultCode, data);
            } else if (mUploadMessage != null) {

                if (result != null) {
                    String path = getPath(getApplicationContext(), result);
                    Uri uri = Uri.fromFile(new File(path));
                    Log.d(TAG, "onActivityResult: 11111111111111111111"+path);
                    mUploadMessage.onReceiveValue(uri);
                } else {
                    mUploadMessage.onReceiveValue(imageUri);
                }
                mUploadMessage = null;
            }
        }
    }

    @SuppressWarnings("null")
    @android.support.annotation.RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    private void onActivityResultAboveL(int requestCode, int resultCode, Intent data) {
        if (requestCode != FILECHOOSER_RESULTCODE
                || mUploadCallbackAboveL == null) {
            return;
        }

        Uri[] results = null;

        if (resultCode == Activity.RESULT_OK) {

            if (data == null) {

                results = new Uri[]{imageUri};
            } else {
                String dataString = data.getDataString();
                ClipData clipData = data.getClipData();

                if (clipData != null) {
                    results = new Uri[clipData.getItemCount()];
                    for (int i = 0; i < clipData.getItemCount(); i++) {
                        ClipData.Item item = clipData.getItemAt(i);
                        results[i] = item.getUri();
                    }
                }

                if (dataString != null)
                    results = new Uri[]{Uri.parse(dataString)};
            }
        }
        if (results != null) {
            mUploadCallbackAboveL.onReceiveValue(results);
            mUploadCallbackAboveL = null;
        } else {
            results = new Uri[]{imageUri};
            mUploadCallbackAboveL.onReceiveValue(results);
            mUploadCallbackAboveL = null;
        }

        return;
    }

    //ChromeClient   监听网页加载
    WebChromeClient mWebChromeClient = new WebChromeClient() {
        @Override
        public void onProgressChanged(WebView view, int newProgress) {
            super.onProgressChanged(view, newProgress);
        }

        @Override
        public boolean onJsAlert(WebView view, String url, String message, final JsResult result) {
            result.confirm();
            return true;
        }

        @Override
        public void onReceivedTitle(WebView view, String title) {
            super.onReceivedTitle(view, title);
        }

    };


    //WebViewClient
    WebViewClient mWebViewClient = new WebViewClient() {
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            // 消耗掉这个事件。Android中返回True的即到此为止吧,事件就会不会冒泡传递了，我们称之为消耗掉
            // 使用自己的WebView组件来响应Url加载事件，而不是使用默认浏览器器加载页面

            Log.e("用户单击超连接", url);
            //判断用户单击的是那个超连接
            String tag = "tel";
            if (url.contains(tag)) {
                String mobile = url.substring(url.lastIndexOf("/") + 1);
                Log.e("mobile----------->", mobile);
                Intent mIntent = new Intent(Intent.ACTION_CALL);
                Uri data = Uri.parse(mobile);
                mIntent.setData(data);
                //Android6.0以后的动态获取打电话权限
                if (ActivityCompat.checkSelfPermission(HomeActivity.this, android.Manifest.permission.CALL_PHONE) == PackageManager.PERMISSION_GRANTED) {
                    startActivity(mIntent);
                    //这个超连接,java已经处理了，webview不要处理
                    return true;
                } else {
                    //申请权限
                    ActivityCompat.requestPermissions(HomeActivity.this, new String[]{Manifest.permission.CALL_PHONE}, 1);
                    return true;
                }
            } else {
                webView.loadUrl(url);
            }
            return true;
        }

        @Override
        public void onPageFinished(WebView view, String url) {

        }
    };

    private long firstTime = 0;
    private static Boolean isExit = false;

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK) && webView.canGoBack()) {
            webView.goBack();
        } else if (keyCode == KeyEvent.KEYCODE_BACK) {
            long secondTime = System.currentTimeMillis();
            if (secondTime - firstTime > 2000) {
                ToastUtil.show(getApplicationContext(), "再按一次退出程序");
                firstTime = secondTime;
            } else {
                finish();
            }
        }
        return false;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (webView != null) {
            webView.destroy();
        }
        //清空所有Cookie
        CookieSyncManager.createInstance(this);  //Create a singleton CookieSyncManager within a context
        CookieManager cookieManager = CookieManager.getInstance(); // the singleton CookieManager instance
        cookieManager.removeAllCookie();// Removes all cookies.
        CookieSyncManager.getInstance().sync(); // forces sync manager to sync now

        webView.setWebChromeClient(null);
        webView.setWebViewClient(null);
        webView.getSettings().setJavaScriptEnabled(false);
        webView.clearCache(true);

    }
}
