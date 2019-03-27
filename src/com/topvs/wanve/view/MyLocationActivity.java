package com.topvs.wanve.view;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.location.Location;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import com.tencent.map.geolocation.TencentGeofence;
import com.tencent.map.geolocation.TencentLocation;
import com.tencent.map.geolocation.TencentLocationListener;
import com.tencent.map.geolocation.TencentLocationManager;
import com.tencent.map.geolocation.TencentLocationRequest;
import com.tencent.mapsdk.raster.model.LatLng;
import com.tencent.mapsdk.raster.model.Marker;
import com.tencent.mapsdk.raster.model.MarkerOptions;
import com.tencent.tencentmap.mapsdk.map.MapView;
import com.tencent.tencentmap.mapsdk.map.TencentMap;
import com.topvs.platform.R;
import com.topvs.wanve.base.DemoGeofenceApp;
import com.topvs.wanve.location.DemoGeofenceService;
import com.topvs.wanve.location.LocationHelper;
import com.topvs.wanve.location.Utils;
import java.util.List;

public class MyLocationActivity extends Activity  implements
        View.OnTouchListener ,TencentLocationListener {
    private LocationHelper mLocationHelper;

    private MapView mMapView;
    private TencentMap mTencentMap;
    private TextView mPosition;
    private ListView mFenceList;

    private ArrayAdapter<TencentGeofence> mFenceListAdapter;

    private List<Marker> mFenceItems;
    TencentLocation location;

    private final Location mCenter = new Location("");
    private final String TAG = "定位";
    private TextView tvPlace;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_my_location);
        initLocation();
        mLocationHelper = new LocationHelper(this);
        initUi();

        doMyLoc();
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
        Log.d(TAG, "onLocationChanged: 地址="+tencentLocation.getAddress());
        tvPlace.setText(tencentLocation.getAddress());
    }

    @Override
    public void onStatusUpdate(String s, int i, String s1) {

    }

    @Override
    protected void onDestroy() {
        mMapView.onDestroy();
        super.onDestroy();
    }

    @Override
    protected void onPause() {
        mMapView.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        mMapView.onResume();
        super.onResume();
    }

    @Override
    protected void onStop() {
        mMapView.onStop();
        super.onStop();
    }

    // ============== ui handler & listener
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.my_loc: // 获取当前位置
                doMyLoc();
                break;

            case R.id.add: // 添加新的围栏
                doPreAdd();
                break;
            case R.id.del: // 删除选中的围栏
                int selected = mFenceList.getCheckedItemPosition();
                doDel(selected);
                break;

            case R.id.stop:
                DemoGeofenceService.stopMe(this); // 停止测试
                break;

            default:
                break;
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (event.getAction() == MotionEvent.ACTION_UP) {
            updatePosition();
        }
        return false;
    }

    private void doMyLoc() {
        if (mLocationHelper.getLastLocation() != null) {
            Log.d(TAG, "doMyLoc: 1");
            animateTo(mLocationHelper.getLastLocation()); // 已有最新位置
        } else if (mLocationHelper.isStarted()) {
            Log.d(TAG, "doMyLoc: 2");
        } else {
            Log.d(TAG, "doMyLoc: 3");
            mLocationHelper.start(new Runnable() {
                public void run() {
                    animateTo(mLocationHelper.getLastLocation());
                }
            });
        }
    }

    private void doPreAdd() {
        View root = getLayoutInflater().inflate(R.layout.dialog_geofence, null);
        TextView tvLocation = (TextView) root.findViewById(R.id.location);
        tvLocation.setText(Utils.toString(mCenter));

        new AlertDialog.Builder(this).setTitle("保存围栏").setView(root)
                .setPositiveButton("确定", new AddGeofenceOnClickListener(root))
                .setNegativeButton("取消", null).show();
    }

    private void doAdd(String tag) {
        toast(this, tag);

        double lat = mCenter.getLatitude();
        double lng = mCenter.getLongitude();
        // 创建地理围栏
        TencentGeofence.Builder builder = new TencentGeofence.Builder();
        TencentGeofence geofence = builder.setTag(tag) // 设置 Tag
                .setCircularRegion(lat, lng, 500) // 设置中心点和半径
                .setExpirationDuration(3 * 3600 * 1000) // 设置有效期
                .build();
        // 更新 adapter view
        mFenceListAdapter.add(geofence);

        // 更新 overlay
        mFenceItems.add(createPoiItem(geofence));

        // 添加地理围栏
        DemoGeofenceService.startMe(this,
                DemoGeofenceService.ACTION_ADD_GEOFENCE, tag);
    }

    private void doDel(int selected) {
        if (selected == ListView.INVALID_POSITION || selected >= mFenceListAdapter.getCount()) {
            toast(this, "没有选中");
            return;
        }

        // 更新 adapter view
        TencentGeofence item = mFenceListAdapter.getItem(selected);
        mFenceListAdapter.remove(item);
        // 更新 marker lists
        mFenceItems.remove(selected).remove();

        // 移除地理围栏
        DemoGeofenceService.startMe(this,
                DemoGeofenceService.ACTION_DEL_GEOFENCE, item.getTag());
    }

    // ============== do methods

    // ============== util methods
    private void initUi() {
        // poi item & poi overlay
        mFenceItems = DemoGeofenceApp.getFenceItems();

        tvPlace = findViewById(R.id.tvPlace);
        // mapview
        mMapView = (MapView) findViewById(R.id.map);
        mTencentMap = mMapView.getMap();
        mMapView.setOnTouchListener(this);

        // list & adapter

        mFenceListAdapter = new ArrayAdapter<TencentGeofence>(this,
                android.R.layout.simple_list_item_checked,
                DemoGeofenceApp.getFence()) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                TencentGeofence geofence = getItem(position);
                TextView tv = (TextView) super.getView(position, convertView,
                        parent);
                tv.setText(Utils.toString(geofence));
                return tv;
            }
        };

        updatePosition();
    }

    private void updatePosition() {
        double lat = mTencentMap.getMapCenter().getLatitude();
        double lng = mTencentMap.getMapCenter().getLongitude();

        mCenter.setLatitude(lat);
        mCenter.setLongitude(lng);
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

    // 生成 poi item
    private Marker createPoiItem(TencentGeofence geofence) {
        Marker marker = mTencentMap.addMarker(new MarkerOptions().
                position(new LatLng(geofence.getLatitude(), geofence.getLongitude())).
                title(geofence.getTag()).
                snippet(Utils.fmt(geofence.getLatitude()) + ","
                        + Utils.fmt(geofence.getLongitude()) + ","
                        + geofence.getRadius()));
        return marker;
    }

    // ============== util methods

    static void toast(Context context, CharSequence text) {
        Toast.makeText(context, text, Toast.LENGTH_SHORT).show();
    }

    class AddGeofenceOnClickListener implements DialogInterface.OnClickListener {

        private View mView;

        public AddGeofenceOnClickListener(View view) {
            super();
            this.mView = view;
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            if (isFinishing()) {
                return;
            }

            EditText etName = (EditText) mView.findViewById(R.id.name);
            String name = etName.getText().toString();

            if (!TextUtils.isEmpty(name)) {
                doAdd(name);
            } else {
                toast(getApplicationContext(), "围栏名字不能为空");
            }
        }
    }
}
