package com.topvs.wanve.view;

import android.Manifest;
import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.MotionEvent;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.TextView;
import android.widget.Toast;

import com.tencent.cloud.cameralib.ICamcorder;
import com.tencent.cloud.cameralib.impl.CamcorderImpl;
import com.topvs.platform.R;
import com.topvs.wanve.util.ToastUtil;

public class RecorderActivity extends Activity {

    public static final String INTENT_KEY_VIDEO_PATH = "INTENT_KEY_VIDEO_PATH";

    private static final String TAG = "Recorder";

    private static final int REQUEST_PERMISSION_CAMERA_CODE = 1;
    private static final int REQUEST_PERMISSION_MIC_CODE = 2;

    private ICamcorder mCamcorder;
    private View mButton;
    private TextureView mTextureView;
    private TextView tvTime;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_recorder);

        initView();
    }

    private void initView() {
        mButton = findViewById(R.id.button_capture);
        mTextureView =  findViewById(R.id.surface_view);
        tvTime = findViewById(R.id.tvTime);

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
                    mCamcorder.startRecord();
                }
            }

            @Override
            public void onLog(String tag, String msg) {
                Log.d(tag, msg);
            }

            @Override
            public void onAnythingFailed(Exception e) {
                ToastUtil.show(RecorderActivity.this, e.toString());
                e.printStackTrace();
            }
        });

        mButton.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();
                if (action == MotionEvent.ACTION_DOWN) {
                    v.setPressed(true);
                    mCamcorder.prepareAsync();
                } else if (action == MotionEvent.ACTION_UP) {
                    mCamcorder.stopRecord();
                    v.setPressed(false);
                } else if (action == MotionEvent.ACTION_OUTSIDE || action == MotionEvent.ACTION_CANCEL) {
                    v.setPressed(false);
                }
                return true;
            }
        });

        findViewById(R.id.use_video).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent=getIntent();
                Bundle b=new Bundle();
                b.putString(INTENT_KEY_VIDEO_PATH, mCamcorder.getVideoFile().getAbsolutePath());
                intent.putExtras(b);
                setResult(RESULT_OK, intent);
                finish();
            }
        });
        mCamcorder.prepareAsync();
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

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy() called");
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
}
