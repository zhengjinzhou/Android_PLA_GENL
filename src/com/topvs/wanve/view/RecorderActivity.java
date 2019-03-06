package com.topvs.wanve.view;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.text.TextUtils;
import android.util.Log;
import android.view.MotionEvent;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.VideoView;

import com.tencent.cloud.cameralib.ICamcorder;
import com.tencent.cloud.cameralib.impl.CamcorderImpl;
import com.tencent.faceid.FaceIdClient;
import com.tencent.faceid.auth.CredentialProvider;
import com.tencent.faceid.exception.ClientException;
import com.tencent.faceid.exception.ServerException;
import com.tencent.faceid.model.VideoImageIdentityRequest;
import com.tencent.faceid.model.VideoImageIdentityResult;
import com.tencent.map.geolocation.TencentLocation;
import com.tencent.map.geolocation.TencentLocationListener;
import com.tencent.map.geolocation.TencentLocationManager;
import com.tencent.map.geolocation.TencentLocationRequest;
import com.topvs.platform.R;
import com.topvs.wanve.base.Constant;
import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.bean.UserInfo;
import com.topvs.wanve.bean.goClockInBean;
import com.topvs.wanve.contract.RecordContract;
import com.topvs.wanve.presenter.DkPresenter;
import com.topvs.wanve.presenter.RecordPresenter;
import com.topvs.wanve.util.SpUtil;
import com.topvs.wanve.util.ToastUtil;
import com.topvs.wanve.widget.LoadDialog;

import java.io.File;

public class RecorderActivity extends Activity implements RecordContract.View,TencentLocationListener {

    public static final String INTENT_KEY_VIDEO_PATH = "INTENT_KEY_VIDEO_PATH";

    private static final String TAG = "Recorder";
    private int currentRequestId;
    private static final int REQUEST_PERMISSION_CAMERA_CODE = 1;
    private static final int REQUEST_PERMISSION_MIC_CODE = 2;
    private FaceIdClient mFaceIdClient;
    private ICamcorder mCamcorder;
    private View mButton;
    private TextureView mTextureView;
    private TextView tvTime;
    private double lat;
    private double lon;
    protected LoadDialog dialog;
    private RecordPresenter presenter = new RecordPresenter(this);
    goClockInBean clockInBean = (goClockInBean) SpUtil.getObject(this, Constant.goClockIn, goClockInBean.class);
    private TextView tvDes;
    private VideoView vvPlay;
    private TextView tvStop;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recorder);

        initView();

        dialog = new LoadDialog(this,false,"加载中...");

        mFaceIdClient = new FaceIdClient(this, UserInfo.APP_ID);

        presenter.attachView(this);

        initLocation();
    }

    private void initView() {
        mButton = findViewById(R.id.button_capture);
        mTextureView =  findViewById(R.id.surface_view);
        tvTime = findViewById(R.id.tvTime);
        tvDes = findViewById(R.id.tvDes);
        vvPlay = findViewById(R.id.vvPlay);
        tvStop = findViewById(R.id.tvStop);

        findViewById(R.id.btPlay).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                PlayVideo();
            }
        });
        tvStop.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                vvPlay.setVisibility(View.GONE);
                tvStop.setVisibility(View.GONE);
            }
        });

        tvDes.setText("请长按下方按钮，并对着摄像头郎读 "+getIntent().getStringExtra("lip")+" 松开结束");

        //修正宽高比
        mTextureView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
                    mTextureView.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                } else {
                    mTextureView.getViewTreeObserver().removeGlobalOnLayoutListener(this);
                }

                int w = mTextureView.getWidth();
                float whRatio = (float) 3 / 4; // 宽:高 = 3:4
                int newH = Math.round(w / whRatio);
                ViewGroup.LayoutParams lp = mTextureView.getLayoutParams();
                lp.height = newH;
                mTextureView.setLayoutParams(lp);
                Log.d(TAG, "TextureView,onGlobalLayout(): w=" + w + ", h=" + newH);
            }
        });

        tryInit();
    }


    private void initC() {

        mCamcorder = new CamcorderImpl();
        mCamcorder.init(mTextureView);
        mCamcorder.setOnStateListener(new ICamcorder.OnStateListener() {
            @Override
            public void onCamcorderPrepared() {
                if (mButton.isPressed()) {
                    vvPlay.setVisibility(View.GONE);
                    tvStop.setVisibility(View.GONE);
                    mCamcorder.startRecord();
                }
            }

            @Override
            public void onLog(String tag, String msg) {
                Log.d(tag, msg);
            }

            @Override
            public void onAnythingFailed(Exception e) {
                vvPlay.setVisibility(View.GONE);
                tvStop.setVisibility(View.GONE);
                mCamcorder.startRecord();
                ToastUtil.show(RecorderActivity.this, "拍摄时间请不要少于1秒");
                e.printStackTrace();
            }
        });


        mButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();
                if (action == MotionEvent.ACTION_DOWN) {
                    currentSecond =0;
                    mhandle.post(timeRunable);
                    v.setPressed(true);
                    mCamcorder.prepareAsync();
                } else if (action == MotionEvent.ACTION_UP) {
                    mhandle.removeCallbacks(timeRunable);
                    mCamcorder.stopRecord();
                    v.setPressed(false);
                } else if (action == MotionEvent.ACTION_OUTSIDE || action == MotionEvent.ACTION_CANCEL) {
                    v.setPressed(false);
                }
                return true;
            }
        });

        /**
         * 使用视频
         */
        findViewById(R.id.use_video).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent=getIntent();
                Bundle b=new Bundle();
                b.putString(INTENT_KEY_VIDEO_PATH, mCamcorder.getVideoFile().getAbsolutePath());
                intent.putExtras(b);
                setResult(RESULT_OK, intent);
                DaKa(mCamcorder.getVideoFile().getAbsolutePath());
                //finish();
            }
        });

        mCamcorder.prepareAsync();
    }

    /**
     * 播放视频
     */
    private void PlayVideo() {

        String pathVideo = mCamcorder.getVideoFile().getAbsolutePath();
        long fileSize = getFileSize(new File(pathVideo));
        Log.d(TAG, "PlayVideo: "+fileSize);
        if (fileSize==0){
            ToastUtil.show(getApplicationContext(),"您尚未拍摄视频");
            return;
        }
        vvPlay.setVisibility(View.VISIBLE);
        tvStop.setVisibility(View.VISIBLE);
        Uri uri = Uri.parse(pathVideo);
        vvPlay.setVideoURI(uri);
        //开始播放视频
        vvPlay.start();
    }

    public static long getFileSize(File file) {
        long size = -1;
        if (file.exists()) {
            size = file.length();
        }
        return size;
    }
    /**
     * 打卡
     */
    private void DaKa(String mVideoPath) {

        String lip = getIntent().getStringExtra("lip");
        Log.d(TAG, "DaKa: "+lip);
        String sdImagePath = getIntent().getStringExtra("sdImagePath");
        String imagePath = Environment.getExternalStorageDirectory().getPath().toString() + "/" + sdImagePath;
        Log.d(TAG, "图片地址: " + sdImagePath);
        Log.d(TAG, "唇语: " + lip);

        if (TextUtils.isEmpty(lip)) {
            ToastUtil.show(getApplicationContext(),"请输入唇语");
            return;
        }
        if (TextUtils.isEmpty(mVideoPath)) {
            ToastUtil.show(getApplicationContext(),"请选择视频");
            return;
        }
        dialog.show();
        String bucketName = UserInfo.BUCKET_NAME;
        CredentialProvider credentialProvider = new CredentialProvider(UserInfo.APP_ID, UserInfo.SECRET_ID, UserInfo.SECRET_KEY);
        Log.d(TAG, "DaKa: " + UserInfo.APP_ID + "-----" + UserInfo.SECRET_KEY);

        String sign = credentialProvider.getMultipleSign(bucketName, UserInfo.EFFECTIVE_DURATION);
        sendRequest(lip, mVideoPath, imagePath, "", sign, bucketName);//打卡
    }

    /**
     * 活体核身（通过视频和照片）
     * <ol>
     * <li>视频活体检测</li>
     * <li>视频与照片相似度检测</li>
     * </ol>
     *
     * @param lip        唇语验证码，4个数字，例如 "0426"
     * @param videoPath  视频文件路径
     * @param imagePath  人脸图片文件路径
     * @param seq        请求标识，用于日志查询
     * @param sign       鉴权签名, 测试时可以调用 {@link CredentialProvider#getMultipleSign(String, long)} 来生成
     * @param bucketName bucket 名称
     */
    private void sendRequest(String lip, String videoPath, String imagePath, String seq, String sign, String bucketName) {
        Log.d(TAG, "活体核身（通过视频和照片）唇语验证码:" + lip + "视频文件路径:" + videoPath + "人脸图片文件路径:" + imagePath + "请求标识:" + seq + "鉴权签名:" + bucketName);
        final VideoImageIdentityRequest request;
        if (TextUtils.isEmpty(imagePath)) {
            request = new VideoImageIdentityRequest(bucketName, lip, videoPath, seq);
        } else {
            request = new VideoImageIdentityRequest(bucketName, lip, videoPath, imagePath, true, seq);
        }
        request.setSign(sign);
        currentRequestId = request.getRequestId();

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    VideoImageIdentityResult result = mFaceIdClient.videoImageIdentity(request);
                    if (result != null) {
                        Log.d(TAG, "打卡反馈信息: " + result.toString());

                        feedback(result);
                        //上传到服务器
                    } else {
                        Log.d(TAG, "result == null ");
                    }
                } catch (ClientException e) {
                    e.printStackTrace();
                    dialog.dismiss();
                    ToastErrot("您尚未录取视频");
                    Log.d(TAG, "run: 1" + e.toString());
                } catch (ServerException e) {
                    e.printStackTrace();
                    dialog.dismiss();
                    ToastErrot("您尚未录取视频");
                    Log.d(TAG, "run: 2 " + e.toString());
                }
            }
        }).start();
    }

    /**
     * 对反馈回来的信息进行对比
     *
     * @param result
     */
    private void feedback(VideoImageIdentityResult result) {
        Log.d(TAG, "feedback: " + result.getLiveStatus());
        if (result.getCode() == 0) {
            dialog.dismiss();
            if (result.getLiveStatus() == -4006) {
                ToastErrot("视频中自拍照特征提取失败");
                return;
            }
            if (result.getLiveStatus() == -4007) {
                ToastErrot("视频中自拍照之间对比失败");
                return;
            }
            if (result.getLiveStatus() == -4009) {
                ToastErrot("Card 照片提取特征失败");
                return;
            }
            if (result.getLiveStatus() == -4010) {
                ToastErrot("自拍照与Card照片相似度计算失败");
                return;
            }
            if (result.getLiveStatus() == -4011) {
                ToastErrot("照片解码失败");
                return;
            }
            if (result.getLiveStatus() == -4012) {
                ToastErrot("照片人脸检测失败");
                return;
            }
            if (result.getLiveStatus() == -4015) {
                ToastErrot("自拍照人脸检测失败");
                return;
            }
            if (result.getLiveStatus() == -4016) {
                ToastErrot("自拍照解码失败");
                return;
            }
            if (result.getLiveStatus() == -4017) {
                ToastErrot("Card 照片人脸检测失败");
                return;
            }

            if (result.getLiveStatus() == -4018) {
                ToastErrot("Card 照片解码失败");
                return;
            }
            if (result.getLiveStatus() == -5001) {
                ToastErrot("视频无效，上传文件不符合视频要求");
                return;
            }
            if (result.getLiveStatus() == -5002) {
                ToastErrot("唇语失败");
                return;
            }
            if (result.getLiveStatus() == -5005) {
                ToastErrot("自拍照解析照片不足，视频里检测到的人脸较少");
                return;
            }
            if (result.getLiveStatus() == -5007) {
                ToastErrot("视频没有声音");
                return;
            }
            if (result.getLiveStatus() == -5008) {
                ToastErrot("语音识别失效，视频里的人读错数字");
                return;
            }
            if (result.getLiveStatus() == -5009) {
                ToastErrot("视频人脸检测失败，没有嘴或者脸");
                return;
            }

            if (result.getLiveStatus() == -5011) {
                ToastErrot("活体检测失败(活体其他错误都归类到里面)");
                return;
            }
            if (result.getLiveStatus() == -5012) {
                ToastErrot("视频中噪声太大");
                return;
            }
            if (result.getLiveStatus() == -5013) {
                ToastErrot("视频里的声音太小");
                return;
            }
            if (result.getLiveStatus() == -5014) {
                ToastErrot("活体检测 level 参数无效");
                return;
            }
            if (result.getLiveStatus() == -5015) {
                ToastErrot("视频像素太低，最小 270*480");
                return;
            }

            if (result.getLiveStatus() == -5016) {
                ToastErrot("视频里的人不是活体（翻拍等攻击)");
                return;
            }
            if (result.getLiveStatus() == -5801) {
                ToastErrot("请求缺少身份证号码或身份证姓名");
                return;
            }
            if (result.getLiveStatus() == -5802) {
                ToastErrot("服务器内部错误，服务暂时不可用");
                return;
            }
            if (result.getLiveStatus() == -5803) {
                ToastErrot("身份证姓名与身份证号码不一致");
                return;
            }
            if (result.getLiveStatus() == -5805) {
                ToastErrot("Card 用户未输入图像或者 url 下载失败");
                return;
            }
            if (result.getSimilarity() < 70) {
                ToastErrot("照片与视频本人不匹配");
                return;
            }
            presenter.GetDistance(lat + "", lon + "");
        } else {
            ToastUtil.show(this, "打卡失败："+result.getMessage());
        }
    }

    private void ToastErrot(final String errorMsg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                ToastUtil.show(getApplicationContext(), errorMsg);
            }
        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.d(TAG, "onStart() called");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        Log.d(TAG, "onRestart() called");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "onResume() called");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.d(TAG, "onPause() called");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.d(TAG, "onStop() called");
    }


    //##################### 权限 #######################

    private void tryInit() {
        if (hasCameraPermission() && hasMicPermission()) {
            initC();
        }
    }

    private boolean hasCameraPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!(ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED)) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA}, REQUEST_PERMISSION_CAMERA_CODE);
                return false;
            }
        }
        return true;
    }

    private boolean hasMicPermission() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!(ContextCompat.checkSelfPermission(this, Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED)) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.RECORD_AUDIO}, REQUEST_PERMISSION_MIC_CODE);
                return false;
            }
        }
        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == REQUEST_PERMISSION_CAMERA_CODE
                || requestCode == REQUEST_PERMISSION_MIC_CODE) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                tryInit();
            }
        }
    }

    /**
     * 线程
     */
    private Runnable timeRunable = new Runnable() {
        @Override
        public void run() {
            currentSecond = currentSecond + 1000;
            tvTime.setText(getFormatHMS(currentSecond));
            if (!isPause) {
                //递归调用本runable对象，实现每隔一秒一次执行任务
                mhandle.postDelayed(this, 1000);
            }
        }
    };
    //计时器
    private Handler mhandle = new Handler();
    private boolean isPause = false;//是否暂停
    private long currentSecond = 0;//当前毫秒数
    public static String getFormatHMS(long time){
        time=time/1000;//总秒数
        int s= (int) (time%60);//秒
        int m= (int) (time/60);//分
        int h=(int) (time/3600);//秒
        return String.format("%02d:%02d:%02d",h,m,s);
    }

    @Override
    public void showError() {
        dialog.dismiss();
    }

    @Override
    public void complete() {
        dialog.dismiss();
    }

    @Override
    public void GetDistanceSuccess(DistanceBean distanceBean) {
        dialog.dismiss();
        if (distanceBean == null) return;
        Log.d(TAG, "GetDistanceSuccess: " + distanceBean);
        if (distanceBean.isSuccess()) {
            ToastUtil.show(getApplicationContext(), "打卡成功");
        } else {
            ToastUtil.show(getApplicationContext(), distanceBean.getMessage());
        }
    }

    @Override
    public String setUserSNID() {
        return clockInBean.getUserSNID();
    }

    @Override
    public String setPM_BH() {
        return clockInBean.getProNo();
    }

    /**
     * 获取经纬度
     */
    private TencentLocationManager mLocationManager;

    private void initLocation() {
        mLocationManager = TencentLocationManager.getInstance(this);
        // 设置坐标系为 gcj-02, 缺省坐标为 gcj-02, 所以通常不必进行如下调用
        mLocationManager.setCoordinateType(TencentLocationManager.COORDINATE_TYPE_GCJ02);
        // 创建定位请求
        TencentLocationRequest request = TencentLocationRequest.create();
        // 修改定位请求参数, 周期为 5000 ms
        request.setInterval(15000);
        // 开始定位
        mLocationManager.requestLocationUpdates(request, this);
    }

    @Override
    public void onLocationChanged(TencentLocation tencentLocation, int i, String s) {
        Log.d(TAG, "onLocationChanged: 维度=" + tencentLocation.getLatitude());
        Log.d(TAG, "onLocationChanged: 精度=" + tencentLocation.getLongitude());
        lat = tencentLocation.getLatitude();
        lon = tencentLocation.getLongitude();
    }

    @Override
    public void onStatusUpdate(String s, int i, String s1) {

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy() called");
        mLocationManager.removeUpdates(this);
    }
}
