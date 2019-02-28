package com.topvs.wanve.view.fragment;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.DownloadManager;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.tencent.faceid.FaceIdClient;
import com.tencent.faceid.auth.CredentialProvider;
import com.tencent.faceid.exception.ClientException;
import com.tencent.faceid.exception.ServerException;
import com.tencent.faceid.model.GetLipLanguageRequest;
import com.tencent.faceid.model.GetLipLanguageResult;
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
import com.topvs.wanve.bean.UserSNIDBean;
import com.topvs.wanve.bean.goClockInBean;
import com.topvs.wanve.contract.DkContract;
import com.topvs.wanve.presenter.DkPresenter;
import com.topvs.wanve.util.DownloadUtil;
import com.topvs.wanve.util.SpUtil;
import com.topvs.wanve.util.ToastUtil;
import com.topvs.wanve.view.RecorderActivity;

import java.io.File;

import static android.os.Looper.getMainLooper;

/**
 * Created by zhou
 * on 2019/2/26.
 */

public class DkFragment extends Fragment implements DkContract.View,
        TencentLocationListener {
    private FaceIdClient mFaceIdClient;
    private TextView tvChun;
    private final String TAG = "DkFragment";
    private DkPresenter presenter = new DkPresenter(this);
    private int currentRequestId;
    private String mVideoPath;
    private String sdImagePath;
    private TencentLocationManager manager;
    private double lat;
    private double lon;

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        presenter.attachView(this);

        presenter.GetAttachment();

        initLocation();

        mFaceIdClient = new FaceIdClient(getContext(), UserInfo.APP_ID);
        sendRequest();
        tvChun = view.findViewById(R.id.tvChun);

        //更换唇语
        view.findViewById(R.id.tvChance).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendRequest();
            }
        });

        view.findViewById(R.id.rlSP).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //录制视频并使用
                recordVideo();
            }
        });

        view.findViewById(R.id.btDaKa).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                DaKa();
            }
        });
    }


    /**
     * 获取经纬度
     */
    private TencentLocationManager mLocationManager;
    private void initLocation() {
        mLocationManager = TencentLocationManager.getInstance(getContext());
        // 设置坐标系为 gcj-02, 缺省坐标为 gcj-02, 所以通常不必进行如下调用
        mLocationManager.setCoordinateType(TencentLocationManager.COORDINATE_TYPE_GCJ02);
        // 创建定位请求
        TencentLocationRequest request = TencentLocationRequest.create();
        // 修改定位请求参数, 周期为 5000 ms
        request.setInterval(15000);
        // 开始定位
        mLocationManager.requestLocationUpdates(request, this);
    }

    public void onLocationChanged(TencentLocation tencentLocation, int i, String s) {
        Log.d(TAG, "onLocationChanged: 维度="+tencentLocation.getLatitude());
        Log.d(TAG, "onLocationChanged: 精度="+tencentLocation.getLongitude());
        lat = tencentLocation.getLatitude();
        lon = tencentLocation.getLongitude();
    }

    @Override
    public void onStatusUpdate(String s, int i, String s1) {

    }
    /**
     * 打卡
     */
    private void DaKa() {
        String lip = tvChun.getText().toString();
        String imagePath = Environment.getExternalStorageDirectory().getPath().toString() + "/"+sdImagePath;
        Log.d(TAG, "图片地址: "+sdImagePath);
        Log.d(TAG, "唇语: "+lip);

        if (TextUtils.isEmpty(lip)) {
            Toast.makeText(getContext(), "请输入唇语", Toast.LENGTH_SHORT).show();
            return;
        }
        if (TextUtils.isEmpty(mVideoPath)) {
            Toast.makeText(getContext(), "请选择视频", Toast.LENGTH_SHORT).show();
            return;
        }

        String bucketName = UserInfo.BUCKET_NAME;
        CredentialProvider credentialProvider = new CredentialProvider(UserInfo.APP_ID, UserInfo.SECRET_ID, UserInfo.SECRET_KEY);
        Log.d(TAG, "DaKa: "+UserInfo.APP_ID+"-----"+UserInfo.SECRET_KEY);

        String sign = credentialProvider.getMultipleSign(bucketName, UserInfo.EFFECTIVE_DURATION);
        sendRequest(lip, mVideoPath, imagePath, "", sign, bucketName);//打卡
    }

    private void recordVideo() {
        Intent intent = new Intent(getContext(), RecorderActivity.class);
        startActivityForResult(intent, RECORD_VIDEO_REQUEST_CODE);
    }

    private static final int IMAGE_SELECT_REQUEST_CODE = 1;
    private static final int VIDEO_SELECT_REQUEST_CODE = 2;
    private static final int RECORD_VIDEO_REQUEST_CODE = 3;
    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != Activity.RESULT_OK || data == null) {
            return;
        }

        switch (requestCode) {
            case IMAGE_SELECT_REQUEST_CODE:
               // onImageSelectActivityResult(data);
                break;
            case VIDEO_SELECT_REQUEST_CODE:
               // onVideoSelectActivityResult(data);
                break;
            case RECORD_VIDEO_REQUEST_CODE:
                onVideoRecordActivityResult(data);
                break;
            default:
                break;
        }
    }

    /**
     * 回调，想不到在Fragment里也可以回调activity
     * @param data
     */
    public void onVideoRecordActivityResult(Intent data) {
        try {
            mVideoPath = data.getStringExtra(RecorderActivity.INTENT_KEY_VIDEO_PATH);
            Log.d(TAG, "video path = " + mVideoPath);

            long fileSize = getFileSize(new File(mVideoPath));
            String msg = String.format("文件路径 %s 大小%s", mVideoPath, getFileSizeString(fileSize));
            Log.d(TAG, "onVideoRecordActivityResult: "+msg);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static String getFileSizeString(long fileSize) {
        if (fileSize < 0) {
            return "无法获取文件大小";
        }

        long KB = 1024L;
        long MB = 1024 * KB;
        if (fileSize < MB) {
            return (fileSize * 1f / KB) + "KB";
        } else {
            return (fileSize * 1f / MB) + "MB";
        }
    }

    public static long getFileSize(File file) {
        long size = -1;
        if (file.exists()) {
            size = file.length();
        }
        return size;
    }

    /**
     * 活体核身（通过视频和照片）
     * <ol>
     *     <li>视频活体检测</li>
     *     <li>视频与照片相似度检测</li>
     * </ol>
     * @param lip 唇语验证码，4个数字，例如 "0426"
     * @param videoPath 视频文件路径
     * @param imagePath 人脸图片文件路径
     * @param seq 请求标识，用于日志查询
     * @param sign 鉴权签名, 测试时可以调用 {@link CredentialProvider#getMultipleSign(String, long)} 来生成
     * @param bucketName bucket 名称
     */
    private void sendRequest(String lip, String videoPath, String imagePath, String seq, String sign, String bucketName) {
        Log.d(TAG, "活体核身（通过视频和照片）唇语验证码:"+lip+"视频文件路径:"+videoPath+"人脸图片文件路径:"+imagePath+"请求标识:"+seq+"鉴权签名:"+bucketName);
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
                        Log.d(TAG, "打卡反馈信息: "+request.toString());
                        //上传到服务器
                        presenter.GetDistance(lat+"",lon+"");
                    } else {
                        Log.d(TAG, "result == null ");
                    }
                } catch (ClientException e) {
                    e.printStackTrace();
                    Log.d(TAG, "run: "+e.toString());
                } catch (ServerException e) {
                    e.printStackTrace();
                    Log.d(TAG, "run: "+e.toString());
                }
            }
        }).start();
    }

    /**
     * 获取唇语验证码，用于活体核身
     *  bucketName bucket 名称
     *  sign 鉴权签名, 测试时可以调用 {@link CredentialProvider#getMultipleSign(String, long)} 来生成
     *  seqString 请求标识，用于日志查询
     */
    private void sendRequest() {
        String bucketName = UserInfo.BUCKET_NAME;
        String seqString = "";
        CredentialProvider credentialProvider = new CredentialProvider(UserInfo.APP_ID, UserInfo.SECRET_ID, UserInfo.SECRET_KEY);
        String sign = credentialProvider.getMultipleSign(bucketName, UserInfo.EFFECTIVE_DURATION);

        final GetLipLanguageRequest request = new GetLipLanguageRequest(bucketName, seqString);
        request.setSign(sign);
        new Thread(){
            @Override
            public void run() {
                try {
                    final GetLipLanguageResult result = mFaceIdClient.getLipLanguage(request);
                    if (result != null) {
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                tvChun.setText(result.getValidateData());
                            }
                        });
                    } else {
                        getActivity().runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                tvChun.setText("null");
                            }
                        });

                    }
                } catch (ClientException e) {
                    e.printStackTrace();
                } catch (ServerException e) {
                    e.printStackTrace();
                    Log.d(TAG, "run: "+e.toString());
                }
                super.run();
            }
        }.start();
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.dk_fragment,container, false);
        return view;
    }

    @Override
    public void showError() {

    }

    @Override
    public void complete() {

    }

    @Override
    public void GetDistanceSuccess(DistanceBean distanceBean) {
        if (distanceBean==null)return;
        Log.d(TAG, "GetDistanceSuccess: "+distanceBean);
        if (distanceBean.isSuccess()){
            ToastUtil.show(getContext(),"打卡成功");
        }else {
            ToastUtil.show(getContext(),distanceBean.getMessage());
        }
    }

    @Override
    public void GetAttachmentSuccess(UserSNIDBean userSNIDBean) {
        if (userSNIDBean==null)
            return;
        Log.d(TAG, "GetAttachmentSuccess: "+userSNIDBean.toString());
        sdImagePath = DownloadUtil.get().download(userSNIDBean.getVirtualPath(), "");
        Log.d(TAG, "GetAttachmentSuccess: "+ sdImagePath);
    }

    @Override
    public String setUserSNID() {
        goClockInBean clockInBean = (goClockInBean) SpUtil.getObject(getContext(), Constant.goClockIn, goClockInBean.class);
        return clockInBean.getUserSNID();
    }

    @Override
    public String setPM_BH() {
        goClockInBean clockInBean = (goClockInBean) SpUtil.getObject(getContext(), Constant.goClockIn, goClockInBean.class);
        return clockInBean.getProNo();
    }


    @Override
    public void onDestroy() {
        super.onDestroy();
        mLocationManager.removeUpdates(this);
    }
}
