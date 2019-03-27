package com.topvs.wanve.view.fragment;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.FragmentManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.location.Location;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.tencent.faceid.FaceIdClient;
import com.tencent.faceid.auth.CredentialProvider;
import com.tencent.faceid.exception.ClientException;
import com.tencent.faceid.exception.ServerException;
import com.tencent.faceid.model.GetLipLanguageRequest;
import com.tencent.faceid.model.GetLipLanguageResult;
import com.tencent.faceid.model.VideoImageIdentityRequest;
import com.tencent.faceid.model.VideoImageIdentityResult;
import com.tencent.map.geolocation.TencentGeofence;
import com.tencent.map.geolocation.TencentLocation;
import com.tencent.map.geolocation.TencentLocationListener;
import com.tencent.map.geolocation.TencentLocationManager;
import com.tencent.map.geolocation.TencentLocationRequest;
import com.tencent.mapsdk.raster.model.BitmapDescriptorFactory;
import com.tencent.mapsdk.raster.model.Circle;
import com.tencent.mapsdk.raster.model.CircleOptions;
import com.tencent.mapsdk.raster.model.GeoPoint;
import com.tencent.mapsdk.raster.model.LatLng;
import com.tencent.mapsdk.raster.model.Marker;
import com.tencent.mapsdk.raster.model.MarkerOptions;
import com.tencent.tencentmap.mapsdk.map.MapView;
import com.tencent.tencentmap.mapsdk.map.TencentMap;
import com.tencent.tencentmap.mapsdk.map.UiSettings;
import com.topvs.platform.MyApp;
import com.topvs.platform.R;
import com.topvs.wanve.base.Constant;
import com.topvs.wanve.base.DemoGeofenceApp;
import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.bean.UserInfo;
import com.topvs.wanve.bean.UserSNIDBean;
import com.topvs.wanve.bean.goClockInBean;
import com.topvs.wanve.contract.DkContract;
import com.topvs.wanve.location.DemoGeofenceService;
import com.topvs.wanve.location.LocationHelper;
import com.topvs.wanve.location.Utils;
import com.topvs.wanve.presenter.DaKaPresenter;
import com.topvs.wanve.presenter.DkPresenter;
import com.topvs.wanve.util.DistanceUtil;
import com.topvs.wanve.util.DownloadUtil;
import com.topvs.wanve.util.SpUtil;
import com.topvs.wanve.util.ToastUtil;
import com.topvs.wanve.view.RecorderActivity;
import com.topvs.wanve.view.RlsbActivity;
import com.topvs.wanve.widget.LoadDialog;

import java.io.File;
import java.util.List;

/**
 * Created by zhou
 * on 2019/3/14.
 */

public class DaKaFragment extends Fragment implements
        View.OnTouchListener, TencentLocationListener, DkContract.View {
    private String sdImagePath;
    private LocationHelper mLocationHelper;
    protected LoadDialog dialog;
    private MapView mMapView;
    private TencentMap mTencentMap;
    private FaceIdClient mFaceIdClient;
    private ArrayAdapter<TencentGeofence> mFenceListAdapter;
    private TextView tvChun;
    private List<Marker> mFenceItems;
    TencentLocation location;
    goClockInBean clockInBean = (goClockInBean) SpUtil.getObject(getContext(), Constant.goClockIn, goClockInBean.class);
    private final Location mCenter = new Location("");
    private final String TAG = "打卡";
    private TextView tvPlace;
    private DaKaPresenter presenter = new DaKaPresenter(this);
    private double latitude;//当前经纬度
    private double longitude;//当前经纬度
    private String address;
    private Bitmap bitmap;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.activity_my_location, container, false);
        return view;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        mFaceIdClient = new FaceIdClient(getContext(), UserInfo.APP_ID);
        sendRequest();

        presenter.attachView(this);

        dialog = new LoadDialog(getContext(), false, "加载中...");

        presenter.GetAttachment();

        mLocationHelper = new LocationHelper(getContext());

        initUi(view);
        initLocation();

        doAdd();

    }

    private void doAdd() {

        for (int i = 0; i < clockInBean.getPoints().size(); i++) {
            double lat = Double.parseDouble(clockInBean.getPoints().get(i).getLat());
            double lng = Double.parseDouble(clockInBean.getPoints().get(i).getLon());

            double distance = Double.parseDouble(clockInBean.getDistance());
            String name = clockInBean.getPoints().get(i).getName();
            TencentGeofence.Builder builder = new TencentGeofence.Builder();
            TencentGeofence geofence = builder.setTag(name) // 设置 Tag
                    .setCircularRegion(lat, lng, 500) // 设置中心点和半径
                    .build();
            mFenceItems.add(createPoiItem(geofence));

            LatLng latLng = new LatLng(lat, lng);
            mTencentMap.addCircle(new CircleOptions().
                    center(latLng).
                    radius(distance).
                    fillColor(0x0f2fc39c).
                    strokeColor(0xff2fc39c).
                    strokeWidth(8));
        }
    }

    @Override
    public void onLocationChanged(TencentLocation tencentLocation, int i, String s) {
        latitude = tencentLocation.getLatitude();
        longitude = tencentLocation.getLongitude();
        address = tencentLocation.getAddress();
        Log.d(TAG, "onLocationChanged: 维度=" + tencentLocation.getLatitude());
        Log.d(TAG, "onLocationChanged: 精度=" + tencentLocation.getLongitude());
        Log.d(TAG, "onLocationChanged: 地址=" + tencentLocation.getAddress());

        mTencentMap.setCenter(new LatLng(tencentLocation.getLatitude(), tencentLocation.getLongitude()));
        tvPlace.setText(tencentLocation.getAddress());

        TencentGeofence.Builder builder = new TencentGeofence.Builder();
        TencentGeofence geofence = builder.setTag(address) // 设置 Tag
                .setCircularRegion(latitude, longitude, 500) // 设置中心点和半径
                .setExpirationDuration(0) // 设置有效期
                .build();
        mFenceItems.add(createNewPoiItem(geofence));

    }

    @Override
    public void onStatusUpdate(String s, int i, String s1) {

    }

    private Marker createNewPoiItem(TencentGeofence geofence) {
        Bitmap bitmap = BitmapFactory.decodeResource(getActivity().getResources(), R.drawable.me);

        Marker marker = mTencentMap.addMarker(new MarkerOptions().
                position(new LatLng(geofence.getLatitude(), geofence.getLongitude())).
                title(geofence.getTag()).icon(BitmapDescriptorFactory.fromBitmap(bitmap)).
                snippet(Utils.fmt(geofence.getLatitude()) + ","
                        + Utils.fmt(geofence.getLongitude()) + ","
                        + geofence.getRadius()));
        return marker;
    }

    /**
     * 腾讯地图打标
     *
     * @param geofence
     * @return
     */
    private Marker createPoiItem(TencentGeofence geofence) {

        Marker marker = mTencentMap.addMarker(new MarkerOptions().
                position(new LatLng(geofence.getLatitude(), geofence.getLongitude())).
                title(geofence.getTag()).icon(BitmapDescriptorFactory.fromBitmap(bitmap)).
                snippet(Utils.fmt(geofence.getLatitude()) + ","
                        + Utils.fmt(geofence.getLongitude()) + ","
                        + geofence.getRadius()));
        return marker;
    }

    private void doMyLoc() {
        mTencentMap.setZoom(15);
        if (mLocationHelper.getLastLocation() != null) {
            animateTo(mLocationHelper.getLastLocation()); // 已有最新位置
        } else if (mLocationHelper.isStarted()) {
        } else {
            mLocationHelper.start(new Runnable() {
                public void run() {
                    animateTo(mLocationHelper.getLastLocation());
                }
            });
        }
    }


    private void animateTo(TencentLocation location) {
        if (location == null) {
            return;
        }
        mMapView.getController().animateTo(Utils.of(location));
        // 修改 mapview 中心点
        mMapView.getController().setCenter(Utils.of(location));
        // 注意一定要更新当前位置 mCenter
        updatePosition();
    }

    /**
     * 更新当前的位置
     */
    private void updatePosition() {
        double lat = mTencentMap.getMapCenter().getLatitude();
        double lng = mTencentMap.getMapCenter().getLongitude();
        Log.d(TAG, "updatePosition: " + lat + "," + lng);
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

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_UP) {
            updatePosition();
        }
        return false;
    }

    /**
     * 对id控件
     *
     * @param view
     */
    private void initUi(View view) {
        mFenceItems = MyApp.getFenceItems();// poi item & poi overlay

        tvChun = view.findViewById(R.id.tvChun);
        tvPlace = view.findViewById(R.id.tvPlace);
        mMapView = view.findViewById(R.id.map);
        mMapView.computeScroll();
        mTencentMap = mMapView.getMap();
        mTencentMap.setZoom(15);
        mMapView.setOnTouchListener(this);

        //更换唇语
        view.findViewById(R.id.tvChance).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendRequest();
            }
        });

        view.findViewById(R.id.btDaKa).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //录制视频并使用
                // recordVideo();
                if (clockInBean.getPoints().size() == 0) {
                    ToastErrot("该项目未设置打卡点！");
                    return;
                }
                boolean flat = false;
                //项目点与设备坐标距离计算
                for (int i = 0; i < clockInBean.getPoints().size(); i++) {
                    double lat = Double.parseDouble(clockInBean.getPoints().get(i).getLat());
                    double lng = Double.parseDouble(clockInBean.getPoints().get(i).getLon());
                    double distanceProject = Double.parseDouble(clockInBean.getDistance());
                    double distance = DistanceUtil.getDistance(lat, lng, latitude, longitude);
                    Log.d(TAG, "onClick: 距离"+distance);
                    if (distance <= distanceProject) {
                        flat = true;
                        recordVideo();
                        break;
                    }
                }
                if (!flat) {
                    ToastErrot("您不在打卡范围内");
                }
            }
        });

        view.findViewById(R.id.my_loc).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                doMyLoc();
            }
        });
    }

    private static final int IMAGE_SELECT_REQUEST_CODE = 1;
    private static final int VIDEO_SELECT_REQUEST_CODE = 2;
    private static final int RECORD_VIDEO_REQUEST_CODE = 3;

    private void recordVideo() {
        Intent intent = new Intent(getContext(), RecorderActivity.class);
        intent.putExtra("lip", tvChun.getText().toString());
        intent.putExtra("sdImagePath", sdImagePath);
        startActivityForResult(intent, RECORD_VIDEO_REQUEST_CODE);
    }

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
     *
     * @param data
     */
    private String mVideoPath;

    public void onVideoRecordActivityResult(Intent data) {
        try {
            mVideoPath = data.getStringExtra(RecorderActivity.INTENT_KEY_VIDEO_PATH);
            long fileSize = getFileSize(new File(mVideoPath));
            String msg = String.format("文件路径 %s 大小%s", mVideoPath, getFileSizeString(fileSize));
            Log.d(TAG, "onVideoRecordActivityResult: " + msg);
            DaKa();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 打卡
     */
    private void DaKa() {
        String lip = tvChun.getText().toString();
        String imagePath = Environment.getExternalStorageDirectory().getPath().toString() + "/" + sdImagePath;
        Log.d(TAG, "图片地址: " + sdImagePath);
        Log.d(TAG, "唇语: " + lip);

        if (TextUtils.isEmpty(lip)) {
            ToastUtil.show(getContext(), "请输入唇语");
            return;
        }
        if (TextUtils.isEmpty(mVideoPath)) {
            ToastUtil.show(getContext(), "请选择视频");
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
    private int currentRequestId;

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
                        dialog.dismiss();
                        feedback(result);
                    } else {
                        Log.d(TAG, "result == null ");
                    }
                } catch (ClientException e) {
                    e.printStackTrace();
                    Log.d(TAG, "run: " + e.toString());
                } catch (ServerException e) {
                    e.printStackTrace();
                    Log.d(TAG, "run: " + e.toString());
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
                ToastErrot("Card 照片与视频本人不匹配");
                return;
            }
            presenter.GetDistance(latitude + "", longitude + "", address);
        } else {
            ToastErrot("打卡失败：" + result.getMessage());
        }
    }

    private void ToastErrot(final String errorMsg) {
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                final AlertDialog.Builder builder = new AlertDialog.Builder(getContext());
                View view = LayoutInflater.from(getContext()).inflate(R.layout.show_dialog, null);
                builder.setView(view);
                final AlertDialog dialogA = builder.create();
                TextView tvMes = view.findViewById(R.id.tvMes);
                tvMes.setText(errorMsg);
                view.findViewById(R.id.tvSure).setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        dialogA.dismiss();
                    }
                });
                dialogA.show();
            }
        });
    }

    /**
     * 计算视频文件的大小
     *
     * @param fileSize
     * @return
     */
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
     * 获取唇语验证码，用于活体核身
     * bucketName bucket 名称
     * sign 鉴权签名, 测试时可以调用 {@link CredentialProvider#getMultipleSign(String, long)} 来生成
     * seqString 请求标识，用于日志查询
     */
    private void sendRequest() {
        String bucketName = UserInfo.BUCKET_NAME;
        String seqString = "";
        CredentialProvider credentialProvider = new CredentialProvider(UserInfo.APP_ID, UserInfo.SECRET_ID, UserInfo.SECRET_KEY);
        String sign = credentialProvider.getMultipleSign(bucketName, UserInfo.EFFECTIVE_DURATION);

        final GetLipLanguageRequest request = new GetLipLanguageRequest(bucketName, seqString);
        request.setSign(sign);
        new Thread() {
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
                    Log.d(TAG, "run: " + e.toString());
                }
                super.run();
            }
        }.start();
    }


    @Override
    public void showError() {

    }

    @Override
    public void complete() {

    }

    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.qiye);
        callBackValue = (CallBackValue) getActivity();
    }

    CallBackValue callBackValue;

    //定义一个回调接口
    public interface CallBackValue {
        public void SendMessageValue(String strValue);
    }

    @Override
    public void GetDistanceSuccess(DistanceBean distanceBean) {
        Log.d(TAG, "GetDistanceSuccess: " + distanceBean.toString());
        if (distanceBean.isSuccess()) {
            ToastErrot("打卡成功");
            callBackValue.SendMessageValue("打卡成功");
        }
    }

    @Override
    public void GetAttachmentSuccess(UserSNIDBean userSNIDBean) {
        if (userSNIDBean == null)
            return;
        Log.d(TAG, "GetAttachmentSuccess: " + userSNIDBean.toString());
        sdImagePath = DownloadUtil.get().download(userSNIDBean.getVirtualPath(), "");
    }

    @Override
    public String setUserSNID() {
        return clockInBean.getUserSNID();
    }

    @Override
    public String setPM_BH() {
        return clockInBean.getProNo();
    }
}