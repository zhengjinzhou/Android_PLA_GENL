
package com.topvs.player;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.topvs.platform.LibAPI;
import com.topvs.platform.R;
import com.topvs.player.PlayViewAdapter.ViewHolder;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.ProgressDialog;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.GridView;
import android.widget.LinearLayout;
import android.widget.TableLayout;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

public class PlayActivity extends Activity {
    private static final String TAG = "PlayActivity";
    protected static final int RESULT_CODE = 1;
    private volatile boolean isActivityExit;
    private ProgressDialog progressDialog = null;
    public Resources rs;

    // 硬件屏幕长宽
    private int i_deviceWidth;
    private int i_deviceHeight;

    // 2014-5-13 LinZh107 增加视频缩放比例,以区别横、竖屏时的视频长宽比
    private float i_Vscale = 1.25f;        // 默认的长宽比例
    private int i_scaleWidth;                // 视频显示的宽
    private int i_scaleHeight;                // 视频显示的高

    Button btn1;

    // 2015-5-12 LinZh107 add multiple view switch
    private Button m_btn4views;
    private Button m_btn9views;
    private Button m_btn16views;
    private Button m_btnRefresh;

    // 2015-5-19 LinZh107 add multiple video views
    private Object m_startPlayLock;
    private Thread m_playStartCtrlThread;

    private class StartVideoIndex {
        int camIndex;
        int viindex;
        int retryCount;
        boolean isStarted;

    }

    /*
     * 2015-4-25 LinZh107 add multiple video views
     * */
    private int i_sfvNums;
    private int i_sfvNumColumns;
    private GridView m_surfacePane;
    private PlayViewAdapter m_playAdapter;
    private ArrayList<Integer> m_selectList;    // store the selected pu_id
    private int[] CamStatusArray;                // pu_index for guinfo list
    private String[] ArrayGUID;                    // pu_id use for listView display and save
    private String[] ArrayGUName;                // pu_name use for listView display
    private ArrayList<HashMap<String, Object>> m_sfpItemArray;    // sfv item data list.
    private ArrayList<StartVideoIndex> m_videoArray;    // cameraList that to be started.
    private int i_listIndex;                    // current selected camera index, index .
    private long l_lastClickTime;                // save last click time for judge doubleClicked
    private boolean isFullSreen;                // double clicked will set fullSreen

    private Button mBtnRecord;
    private Button mBtnRecord_f;
    private boolean isRecording = false;
    private int i_inSDCard = 0;                    // current store in sdCard flag.

    // 2014-5-13 LinZh107 增加登录信息提示
    private Handler mHandler;
    private final int MSG_SHOWOPEN = 1;
    private final int MSG_SHOWCLOSE = 2;
    private final int MSG_DISMISSDIALOG = 3;
    private final int MSG_ONESFV = 4;
    private final int MSG_ALLSFV = 5;
    private final int MSG_REFRESH = 6;

    // 音频模块变量
    private Button mBtnChart;
    private Thread m_chartThread;
    private boolean isTalking = false;

    private Thread m_audioThread;
    private AudioTrack mTrack;
    public short[] s_audioDataArray;
    public int i_audioParams[] = {0, 0};    // 音频帧速和帧长

    /**
     * 云台控制class封装
     */
    PZLControlThread ctlThread;


    /*** 2014-5-13 LinZh107 增加消息提示 ******/
    class MyMessageHandler extends Handler {
        public MyMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            switch (msg.what) {
                case MSG_SHOWOPEN:
                    if (progressDialog == null || !progressDialog.isShowing())
                        progressDialog = ProgressDialog.show(PlayActivity.this,
                                rs.getString(R.string.msg_opening), rs.getString(R.string.msg_wait));
                    break;
                case MSG_SHOWCLOSE:
                    if (progressDialog == null || !progressDialog.isShowing())
                        progressDialog = ProgressDialog.show(PlayActivity.this,
                                rs.getString(R.string.msg_closing), rs.getString(R.string.msg_wait));
                    break;
                case MSG_DISMISSDIALOG:
                    if (progressDialog != null && progressDialog.isShowing())
                        progressDialog.dismiss();
                    break;
                /*
				 * 6.增加刷新功能，即restart，问题:在restart之后sfhcallback不能正常运行，
				 * fix：在restart之前必须stop，而在stop之前必须stopcallback
				 */
                case MSG_REFRESH: {
                    if (progressDialog == null || !progressDialog.isShowing())
                        progressDialog = ProgressDialog.show(PlayActivity.this,
                                rs.getString(R.string.msg_refreshing), rs.getString(R.string.msg_wait));
                    synchronized (m_startPlayLock) {
                        for (int i = 0; i < m_videoArray.size(); i++) {
                            PlaySFHCallBack sfhcb = (PlaySFHCallBack) m_playAdapter.getItem(i);
                            if (sfhcb != null)
                                sfhcb.isStoped = true;
                            if (m_videoArray.get(i).isStarted) {
                                LibAPI.StopPlay(m_videoArray.get(i).viindex);
                                m_videoArray.get(i).isStarted = false;
                                m_videoArray.get(i).retryCount = 0;
                            }
                        }
                        if (progressDialog != null && progressDialog.isShowing())
                            progressDialog.dismiss();
                        m_startPlayLock.notify();
                    }
                    break;
                }
                case MSG_ONESFV: {
                    i_sfvNums = i_sfvNumColumns = 1;
                    m_playAdapter.setFullScreen(true, i_listIndex);
                    setSurfacePaneItemNums(i_sfvNums, i_sfvNumColumns);
                    synchronized (m_startPlayLock) {
                        for (int i = 0; i < m_videoArray.size(); i++) {
                            if (i != i_listIndex) {
                                if (m_videoArray.get(i).isStarted) {
                                    LibAPI.StopPlay(m_videoArray.get(i).viindex);
                                    m_videoArray.get(i).isStarted = false;
                                }
                            }
                        }
                    }
                    // m_playAdapter.notifyDataSetChanged();
                    break;
                }
                case MSG_ALLSFV: {
                    if (m_videoArray.size() < 5) {
                        i_sfvNums = 4;
                        i_sfvNumColumns = 2;
                    } else if (m_videoArray.size() < 10) {
                        i_sfvNums = 9;
                        i_sfvNumColumns = 3;
                    } else {
                        i_sfvNums = 16;
                        i_sfvNumColumns = 4;
                    }
                    m_playAdapter.setFullScreen(false, i_listIndex);
                    setSurfacePaneItemNums(i_sfvNums, i_sfvNumColumns);
                    synchronized (m_startPlayLock) {
                        m_startPlayLock.notify();
                    }
                    // m_playAdapter.notifyDataSetChanged();
                    break;
                }
                default:
                    break;
            }
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.playview);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        Bundle extras = getIntent().getExtras();
        rs = getResources();

        // 得到当前线程的Looper实例，由于当前线程是UI线程也可以通过Looper.getMainLooper()得
        Looper looper = Looper.myLooper();
        // 此处甚至可以不需要设置Looper，因为 Handler默认就使用当前线程的Loope
        mHandler = new MyMessageHandler(looper);

        i_inSDCard = extras.getInt("INSDCARD");
        CamStatusArray = extras.getIntArray("GUSTATUSARRAY");
        ArrayGUID = extras.getStringArray("GUIDARRAY");
        ArrayGUName = extras.getStringArray("GUNAMEARRAY");
        m_selectList = extras.getIntegerArrayList("SELECTINDEX");
        i_listIndex = 0;
        m_videoArray = new ArrayList<StartVideoIndex>();
        for (int i = 0; i < m_selectList.size(); i++) {
            StartVideoIndex svi = new StartVideoIndex();
            svi.camIndex = CamStatusArray[m_selectList.get(i)];
            svi.viindex = i;
            svi.retryCount = 0;
            svi.isStarted = false;
            m_videoArray.add(svi);

            Log.d(TAG, "这些是什么文件"+m_videoArray.toString());
        }

        Log.d(TAG, "onCreate:就是这里了 "+m_selectList);
        // 初始化云台控制栏
        initYuntaiControl();

        initVideoPlayView();

        /************* 视频播放控制线程 **************/
        m_startPlayLock = new Object();
        isActivityExit = false;
        m_playStartCtrlThread = new Thread(new Runnable() {
            public void run() {
                createThreadPool();
            }
        });
        m_playStartCtrlThread.start();


        /** 2015-5-06 Add by LinZh107 support multiple video **/
        createSurfacePane();

        m_btn4views = (Button) this.findViewById(R.id.fourCamView);
        m_btn9views = (Button) this.findViewById(R.id.nineCamView);
        m_btn16views = (Button) this.findViewById(R.id.sixteenCamView);
        m_btnRefresh = (Button) this.findViewById(R.id.refreshCamView);
        mBtnChart = (Button) this.findViewById(R.id.chart_btn);

        mBtnChart.setOnClickListener(onListenerCVBtn);
        m_btn4views.setOnClickListener(onListenerCVBtn);
        m_btn9views.setOnClickListener(onListenerCVBtn);
        m_btn16views.setOnClickListener(onListenerCVBtn);
        m_btnRefresh.setOnClickListener(onListenerCVBtn);

        /************* 音频解码线程 ***************/
        m_audioThread = new Thread(new Runnable() {
            public void run() {
                threadRunDecodeAudio();
            }
        });
        m_audioThread.start();

        /************* 音频对讲线程 ***************/
        m_chartThread = new Thread(new Runnable() {
            public void run() {
                threadRunRecordAduio();
            }
        });
        m_chartThread.start();


        ctlThread = new PZLControlThread();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // 获取屏幕参数并保存
        WindowManager.LayoutParams attrs = getWindow().getAttributes();

        // 横屏
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            attrs.flags |= WindowManager.LayoutParams.FLAG_FULLSCREEN;

            // Measure the layout's width and height
            LinearLayout layout = (LinearLayout) findViewById(R.id.ytline);
            layout.setVisibility(View.GONE);

            LinearLayout layout2 = (LinearLayout) findViewById(R.id.TopPanelLine);
            layout2.setVisibility(View.GONE);

            LinearLayout layout3 = (LinearLayout) findViewById(R.id.widget_layout_switchbar);
            layout3.setVisibility(View.GONE);

            LinearLayout layout5 = (LinearLayout) findViewById(R.id.fullscreenbar);
            layout5.setVisibility(View.VISIBLE);
            layout5.bringToFront();

            // 横屏时隐藏信息窗
            TableLayout layout6 = (TableLayout) findViewById(R.id.widget_layout_info);
            layout6.setVisibility(View.GONE);
        } else
        // 竖屏
        {
            attrs.flags &= ~WindowManager.LayoutParams.FLAG_FULLSCREEN;

            // Measure the layout's width and height
            LinearLayout layout = (LinearLayout) findViewById(R.id.ytline);
            layout.setVisibility(View.VISIBLE);

            LinearLayout layout2 = (LinearLayout) findViewById(R.id.TopPanelLine);
            layout2.setVisibility(View.VISIBLE);

            LinearLayout layout3 = (LinearLayout) findViewById(R.id.widget_layout_switchbar);
            layout3.setVisibility(View.VISIBLE);

            LinearLayout layout5 = (LinearLayout) findViewById(R.id.fullscreenbar);
            layout5.setVisibility(View.GONE);

            TableLayout layout6 = (TableLayout) findViewById(R.id.widget_layout_info);
            layout6.setVisibility(View.VISIBLE);
        }

        setSurfacePaneItemNums(i_sfvNums, i_sfvNumColumns);

        // 恢复屏幕参数
        getWindow().setAttributes(attrs);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        if (isRecording) {
            if (0 == LibAPI.StopRecord(i_listIndex))
                Toast.makeText(PlayActivity.this, "Finish recording", Toast.LENGTH_SHORT).show();
            isRecording = false;
        }
        return;
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "PlayActivity call destroy.");
        isActivityExit = true;
        ctlThread.destroy();

        synchronized (m_startPlayLock) {
            m_startPlayLock.notifyAll();
        }
        if (m_playStartCtrlThread != null && m_playStartCtrlThread.isAlive())
            try {
                m_playStartCtrlThread.join();
            } catch (InterruptedException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

        for (int i = 0; i < m_videoArray.size(); i++) {
            if (m_videoArray.get(i).isStarted) {
                LibAPI.StopPlay(m_videoArray.get(i).viindex);
                m_videoArray.get(i).isStarted = false;
            }
        }
        super.onDestroy();
        this.finish();
    }

    /*
     * 2015-5-19 LinZh107 add for start multiple video instance
     * 4.如果不将 start 和 stop 放在callback 怎退出时要等解码完成的问题。
     *  fix：将解码数据拷贝到解码函数的内置独立buffer，不必等待
     * 5.建立两个线程 一个维护SatrtStream，一个维护StopStream
     *  fix1：已初步建立
     *  引申问题：startPool 建了很多start线程之后，拉到视频再退出能正常工作；
     * 	但是当，还没拉到视频就要退出，则stop线程将不能有效stop，因为start是在stop之后
     *	fix2：在start线程之后加上退出判断，当退出标志置位，就则在后面进行stop操作。
     */
    private void createThreadPool() {
        // TODO Auto-generated method stub
        ExecutorService cachedThreadPool = Executors.newCachedThreadPool();
        int index = 0;
        while (!isActivityExit) {
            synchronized (m_startPlayLock) {
                while (index < m_videoArray.size()) {
                    if (m_videoArray.get(index).retryCount < 2
                            && !m_videoArray.get(index).isStarted) {
                        mHandler.sendEmptyMessage(MSG_SHOWOPEN);
                        final int vindex = m_videoArray.get(index).viindex;
                        final int cindex = m_videoArray.get(index).camIndex;
                        cachedThreadPool.execute(new Runnable() {
                            @Override
                            public void run() {
                                threadRunOpenVideo(cindex, vindex);
                            }
                        });

                        try {
                            Thread.sleep(100);
                        } catch (InterruptedException e1) {
                            // TODO Auto-generated catch block
                            e1.printStackTrace();
                        }
                    }
                    index++;
                }//end while()
                try {
                    Log.d(TAG, "Wait the startLock notify");
                    m_startPlayLock.wait();
                    Log.d(TAG, "Recv the startLock notify");
                    index = 0;
                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    Log.e(TAG, "CreateThreadPool() recv interrupted " + e.toString());
                }
            }
        }
        // end while()
    }

    // 2015-5-6 LinZh107 add a surface pane for multiple views
    private void createSurfacePane() {
        isFullSreen = false;
        i_sfvNums = i_sfvNumColumns * i_sfvNumColumns;

        for (int i = 0; i < m_selectList.size(); i++) {
            HashMap<String, Object> itemMap = new HashMap<String, Object>();
            itemMap.put("VIstance", i);
            itemMap.put("CameraID", CamStatusArray[m_selectList.get(i)]);
            itemMap.put("ItemTitle", " " + ArrayGUName[m_selectList.get(i)]);
            m_sfpItemArray.add(itemMap);
        }

        // set adapter to create the surfaceViewGroup
        m_playAdapter = new PlayViewAdapter(this, m_sfpItemArray);
        setSurfacePaneItemNums(i_sfvNums, i_sfvNumColumns);
        m_surfacePane.setAdapter(m_playAdapter);
        m_surfacePane.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                // TODO Auto-generated method stub
                // make every item be checked
                ((ViewHolder) view.getTag()).playitem.setBackgroundColor(Color.rgb(205, 115, 54));

                if (System.currentTimeMillis() - l_lastClickTime < 400 && position < m_sfpItemArray.size()) {
                    isFullSreen = !isFullSreen;
                    if (isFullSreen) {
                        i_listIndex = position;
                        mHandler.sendEmptyMessage(MSG_ONESFV);
                    } else mHandler.sendEmptyMessage(MSG_ALLSFV);
                } else if (!isFullSreen && i_listIndex != position && i_listIndex < parent.getCount()) {
                    // unchecked last selected item
                    ViewHolder holder = (ViewHolder) parent.getChildAt(i_listIndex).getTag();
                    if (null != holder)
                        holder.playitem.setBackgroundColor(Color.rgb(102, 102, 102));
                    i_listIndex = position;
                }

                l_lastClickTime = System.currentTimeMillis();
            }
        });
    }

    private void initYuntaiControl() {
        // TODO Auto-generated method stub
        btn1 = (Button) findViewById(R.id.leftbtn);
        btn1.setOnTouchListener(onTouchListener);

        Button btn2 = (Button) findViewById(R.id.rightbtn);
        btn2.setOnTouchListener(onTouchListener);

        Button btn3 = (Button) findViewById(R.id.upbtn);
        btn3.setOnTouchListener(onTouchListener);

        Button btn4 = (Button) findViewById(R.id.downbtn);
        btn4.setOnTouchListener(onTouchListener);

        Button btn5 = (Button) findViewById(R.id.zoominbtn);
        btn5.setOnTouchListener(onTouchListener);

        Button btn6 = (Button) findViewById(R.id.zoomoutbtn);
        btn6.setOnTouchListener(onTouchListener);

        Button takephotobtn = (Button) findViewById(R.id.takephotobtn);
        takephotobtn.setOnClickListener(onListener);

        mBtnRecord = (Button) findViewById(R.id.recordbtn);
        mBtnRecord.setOnClickListener(onListener);

        /**************** 以上为竖屏时的按钮布局 *************************************************/
        /**************** 以下为全屏时的按钮布局 *************************************************/

        Button btn1_f = (Button) findViewById(R.id.leftbtn_f);
        btn1_f.setOnTouchListener(onTouchListener);

        Button btn2_f = (Button) findViewById(R.id.rightbtn_f);
        btn2_f.setOnTouchListener(onTouchListener);

        Button btn3_f = (Button) findViewById(R.id.upbtn_f);
        btn3_f.setOnTouchListener(onTouchListener);

        Button btn4_f = (Button) findViewById(R.id.downbtn_f);
        btn4_f.setOnTouchListener(onTouchListener);

        Button btn5_f = (Button) findViewById(R.id.zoominbtn_f);
        btn5_f.setOnTouchListener(onTouchListener);

        Button btn6_f = (Button) findViewById(R.id.zoomoutbtn_f);
        btn6_f.setOnTouchListener(onTouchListener);

        Button takephotobtn_f = (Button) findViewById(R.id.takephotobtn_f);
        takephotobtn_f.setOnClickListener(onListener);

        mBtnRecord_f = (Button) findViewById(R.id.recordbtn_f);
        mBtnRecord_f.setOnClickListener(onListener);
    }

    private void initVideoPlayView() {
        // TODO Auto-generated method stub
        // 获取屏幕密度（方法3）
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        i_deviceHeight = dm.heightPixels;
        i_deviceWidth = dm.widthPixels; // 屏幕宽（px，如：480px）
        Log.i(TAG, "i_deviceWidth x i_deviceHeight = " + i_deviceWidth + "x" + i_deviceHeight);

        m_surfacePane = (GridView) findViewById(R.id.surfacePane);
        m_sfpItemArray = new ArrayList<HashMap<String, Object>>();

        /************* 初始化视频显示栏 **************/
        // sfpNum is a number that 1<= i_sfvNum <=4
        if (m_selectList.size() < 5)
            i_sfvNumColumns = 2;
        else if (m_selectList.size() < 10)
            i_sfvNumColumns = 3;
        else if (m_selectList.size() < 17)
            i_sfvNumColumns = 4;
    }

    // 2015-5-14 LinZh107 add for multiple videos
    private OnClickListener onListenerCVBtn = new OnClickListener() {
        @Override
        public void onClick(View v) {
            // TODO Auto-generated method stub
            switch (v.getId()) {
                // 2015-5-12 add multipel video surpost button
                case R.id.fourCamView:
                    i_sfvNums = 4;
                    i_sfvNumColumns = 2;
                    break;
                case R.id.nineCamView:
                    i_sfvNums = 9;
                    i_sfvNumColumns = 3;
                    Log.i(TAG, "Click the button nineCamView");
                    break;
                case R.id.sixteenCamView:
                    i_sfvNums = 16;
                    i_sfvNumColumns = 4;
                    break;
                case R.id.refreshCamView:
                    mHandler.sendEmptyMessage(MSG_REFRESH);
                    return;
                case R.id.chart_btn:
                    if (i_listIndex < m_videoArray.size()) {
                        int ret = 0;
                        if (isTalking) {
                            LibAPI.StopChart();
                            isTalking = false;
                        } else {
                            ret = LibAPI.StartChart(m_videoArray.get(i_listIndex).camIndex, m_videoArray.get(i_listIndex).viindex);
                            if (ret == 0) {
                                isTalking = true;
                            }
                        }
                        m_playAdapter.setChartBtnBackgroud(isTalking, i_listIndex);
                    }
                    return;
            }

            isFullSreen = false;
            m_playAdapter.setFullScreen(false, i_listIndex);
            setSurfacePaneItemNums(i_sfvNums, i_sfvNumColumns);

            if (m_videoArray.size() < i_sfvNums)
                // reopen all video
                synchronized (m_startPlayLock) {
                    m_startPlayLock.notify();
                }
            else if (m_videoArray.size() > i_sfvNums) {
                // close some video
                mHandler.sendEmptyMessage(MSG_SHOWCLOSE);
                synchronized (m_startPlayLock) {
                    for (int i = 0; i < m_videoArray.size(); i++) {
                        if (i >= i_sfvNums) {
                            if (m_videoArray.get(i).isStarted) {
                                LibAPI.StopPlay(m_videoArray.get(i).viindex);
                                m_videoArray.get(i).isStarted = false;
                            }
                        }
                    }
                    mHandler.sendEmptyMessage(MSG_DISMISSDIALOG);
                }
                progressDialog.dismiss();
            }
        }
    };
    // 2015-5-10 LinZh107 modify for multiple videos
    private OnClickListener onListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            int ret = -1;
            if ((i_listIndex + 1) > m_videoArray.size()) {
                Toast.makeText(PlayActivity.this, "选择有效摄像机", Toast.LENGTH_SHORT).show();
                return;
            }

            int camIndex = 0;
            PlaySFHCallBack sfhcb = (PlaySFHCallBack) m_playAdapter.getItem(i_listIndex);
            if (sfhcb != null)
                camIndex = sfhcb.mVIIndex;
            else return;

            switch (v.getId()) {
                // 2014-6-5 屏蔽单击式控制云台

                case R.id.takephotobtn:
                case R.id.takephotobtn_f: {
                    takePhoto(sfhcb);
                    break;
                }

                case R.id.recordbtn:
                case R.id.recordbtn_f: {
                    if (!isRecording) {
                        ret = startRecordVideo();
                        if (ret >= 0) {
                            isRecording = true;
                            mBtnRecord.setBackgroundResource(R.drawable.recording);
                            mBtnRecord_f.setBackgroundResource(R.drawable.recording_f);
                            Toast.makeText(PlayActivity.this, "Recoding...", Toast.LENGTH_SHORT).show();
                        } else {
                            isRecording = false;
                            Toast.makeText(PlayActivity.this, "Recod error.", Toast.LENGTH_SHORT).show();
                        }
                    } else {
                        mBtnRecord.setBackgroundResource(R.drawable.a2_7);
                        mBtnRecord_f.setBackgroundResource(R.drawable.record_yt);
                        ret = LibAPI.StopRecord(i_listIndex);
                        if (ret == 0)
                            Toast.makeText(PlayActivity.this, "Finish recording", Toast.LENGTH_SHORT).show();
                        isRecording = false;
                    }
                    break;
                }
            }
            // end switch (v.getId())
        }
        // end onClick()
    };

    // 2014-6-5 LinZh107 chance click to touch
    private OnTouchListener onTouchListener = new OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if ((i_listIndex + 1) > m_videoArray.size()) {
                Toast.makeText(PlayActivity.this, "选择有效摄像机", Toast.LENGTH_SHORT).show();
                return true;
            }
            if (!isFullSreen) {
                Toast.makeText(PlayActivity.this, "双击摄像机,单画面预览模式才可操作", Toast.LENGTH_SHORT).show();
                return true;
            }

            int camIndex = m_videoArray.get(i_listIndex).camIndex;
            Toast.makeText(PlayActivity.this, "云台控制", Toast.LENGTH_SHORT).show();
			/*
			 * camera turn left
			 * */
            if ((v.getId() == R.id.leftbtn) || (v.getId() == R.id.leftbtn_f)) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ctlThread.startCTL(camIndex, 5, "PL");
                    v.setBackgroundResource(R.drawable.a2_2);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ctlThread.stopCTL();
                    v.setBackgroundResource(R.drawable.a2_1);
                }
                return true;
            }
			/*
			 * camera turn right
			 * */
            else if ((v.getId() == R.id.rightbtn) || (v.getId() == R.id.rightbtn_f)) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ctlThread.startCTL(camIndex, 5, "PR");
                    v.setBackgroundResource(R.drawable.a2_2);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ctlThread.stopCTL();
                    v.setBackgroundResource(R.drawable.a2_3);
                }
                return true;
            }
			/*
			 * camera turn up
			 * */
            else if ((v.getId() == R.id.upbtn) || (v.getId() == R.id.upbtn_f)) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ctlThread.startCTL(camIndex, 5, "TU");
                    v.setBackgroundResource(R.drawable.a2_2);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ctlThread.stopCTL();
                    v.setBackgroundResource(R.drawable.a1_2);
                }
                return true;
            }
			/*
			 * camera turn down
			 * */
            else if ((v.getId() == R.id.downbtn) || (v.getId() == R.id.downbtn_f)) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ctlThread.startCTL(camIndex, 5, "TD");
                    v.setBackgroundResource(R.drawable.a2_2);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ctlThread.stopCTL();
                    v.setBackgroundResource(R.drawable.a3_2);
                }
                return true;
            }
			/*
			 * camera zoom in
			 * */
            else if ((v.getId() == R.id.zoominbtn) || (v.getId() == R.id.zoominbtn_f)) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ctlThread.startCTL(camIndex, 2, "ZIN");
                    v.setBackgroundResource(R.drawable.a2_2);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ctlThread.stopCTL();
                    v.setBackgroundResource(R.drawable.a2_4);
                }
                return true;
            }
			/*
			 * camera zoom out
			 * */
            else if ((v.getId() == R.id.zoomoutbtn) || (v.getId() == R.id.zoomoutbtn_f)) {
                if (event.getAction() == MotionEvent.ACTION_DOWN) {
                    ctlThread.startCTL(camIndex, 2, "ZOUT");
                    v.setBackgroundResource(R.drawable.a2_2);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    ctlThread.stopCTL();
                    v.setBackgroundResource(R.drawable.a2_5);
                }
                return true;
            }
            return false;
        }
    };

    // 2015-5-12 LinZh107 change SurfacePane items
    private void setSurfacePaneItemNums(int sfvNums, int sfvColumns) {
        if (this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE) {
            // because of the spacing, the item's width and height must be minus
            // 2 pix
            i_scaleWidth = (i_deviceHeight / sfvColumns) - 2;
            i_scaleHeight = (i_deviceWidth / sfvColumns) - 2;
        } else if (this.getResources().getConfiguration().orientation == Configuration.ORIENTATION_PORTRAIT) {
            // because of the spacing, the item's width and height must be minus
            // 2 pix
            i_scaleWidth = (i_deviceWidth / sfvColumns) - 2;
            i_scaleHeight = ((int) (i_deviceWidth * 1.0 / i_Vscale) / sfvColumns) - 2;
        }

        m_surfacePane.setNumColumns(sfvColumns);
        m_playAdapter.setCount(sfvNums, i_scaleWidth, i_scaleHeight);
        // m_playAdapter.clearSfhcb();
        // m_playAdapter.notifyDataSetChanged();
    }

    private int startRecordVideo() {
        SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyyMMdd-HHmmss");
        String date = sDateFormat.format(new java.util.Date());
        // 如果没有SD卡，就自动保存在手机内存中
        String sDir = getApplicationContext().getFilesDir().getAbsolutePath();
        if (1 == i_inSDCard) {
            String sDStateString = android.os.Environment.getExternalStorageState();
            if (sDStateString.equals(android.os.Environment.MEDIA_MOUNTED)) {
                sDir = "/sdcard";
            } else {
                Toast.makeText(PlayActivity.this, "Not found SD card, will save data to phone's memory。",
                        Toast.LENGTH_LONG).show();
            }
        }
        sDir += "/tuopu/record/";
        sDir += ArrayGUName[m_selectList.get(i_listIndex)];
        File destDir = new File(sDir);
        if (!destDir.exists()) {
            destDir.mkdirs();
            Log.i(TAG, sDir);
        }

        String path = sDir + "/" + date + ".mp4";
        return LibAPI.StartRecord(path, i_listIndex);
    }

    @SuppressLint("SimpleDateFormat")
    private void takePhoto(PlaySFHCallBack sfhcb)/* throws IOException */ {
        Log.i(TAG, "takePhoto for camera " + sfhcb.mVIIndex);
        sfhcb.isStoped = true;
        // wait for decode finish
        try {
            Thread.sleep(20);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        if (sfhcb.mBitmap != null && sfhcb.i_frameRate > 0) {
            SimpleDateFormat sDateFormat = new SimpleDateFormat("yyyyMMdd-HHmmss");
            String date = sDateFormat.format(new java.util.Date());

            // 如果没有SD卡，就自动保存在手机内存中
            String sDir = getApplicationContext().getFilesDir().getAbsolutePath();
            if (1 == i_inSDCard) {
                String sDStateString = android.os.Environment.getExternalStorageState();
                if (sDStateString.equals(android.os.Environment.MEDIA_MOUNTED)) {
                    sDir = "/sdcard";
                } else {
                    Toast.makeText(PlayActivity.this, "Not found SD card, will save data to phone's memory。",
                            Toast.LENGTH_LONG).show();
                }
            }

            sDir += "/tuopu/picture/";
            String path = sDir + ArrayGUName[m_selectList.get(i_listIndex)];
            File destDir = new File(path);
            if (!destDir.exists()) {
                destDir.mkdirs();
            }

            File file = new File(path + "/" + date + ".png");
            Log.i(TAG, sDir);
            try {
                file.createNewFile();
            } catch (IOException e1) {
                // TODO Auto-generated catch block
                e1.printStackTrace();
            }
            FileOutputStream fileOut = null;
            try {
                fileOut = new FileOutputStream(file);
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }

            // 2014-5-21 因更换新的显示方案，所以要加上翻转图像
            sfhcb.mBitmap.compress(Bitmap.CompressFormat.PNG, 100, fileOut);
            try {
                fileOut.flush();
                fileOut.close();
                Log.d(TAG, "takePhoto for camera " + sfhcb.mVIIndex);
                sfhcb.isStoped = false;
                Toast.makeText(PlayActivity.this, "Take photo surcceed.", Toast.LENGTH_SHORT).show();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        sfhcb.isStoped = false;
    }

    private void threadRunRecordAduio() {
        int recBufSize = 0;
        AudioRecord audioRecord = null;

        /********** LinZh107 以下在创建通用于所有机型的录音和播放代码时可用 ***********/
        for (int sampleRateInHz : new int[]{8000, 11025, 16000, 22050, 32000, 47250, 44100, 48000}) {
            for (short channelConfig : new short[]{AudioFormat.CHANNEL_IN_MONO, AudioFormat.CHANNEL_IN_STEREO}) {
                for (short audioFormat : new short[]{AudioFormat.ENCODING_PCM_16BIT, AudioFormat.ENCODING_PCM_8BIT}) {    // Try to initialize
                    try {    // 用于录音
                        recBufSize = AudioRecord.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
                        if (recBufSize <= 0 /* || sendBufSize <= 0 */)
                            continue;
                        Log.i(TAG, "录音: sampleRate=" + sampleRateInHz + " audioFormat=" + audioFormat
                                + " channelConfig=" + channelConfig);
                        Log.i(TAG, "Record PCMData Lenght = " + recBufSize);

                        /********** LinZh107 以上在创建通用于所有机型的录音和播放代码时可用 ***********/
                        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRateInHz, channelConfig,
                                audioFormat, recBufSize * 10);
                        if (audioRecord.getState() == AudioRecord.STATE_INITIALIZED) {
                            // 开始录音
                            audioRecord.startRecording();

                            if (AudioRecord.RECORDSTATE_RECORDING == audioRecord.getRecordingState())
                                Log.d(TAG, "RECORDSTATE_RECORDING is true");

                            // 发送缓冲区
                            int sendlen = 640;
                            short[] sendBuffer = new short[sendlen];
                            while (!isActivityExit) {
                                // 从mic读取音频数据
                                if (isTalking) {
                                    audioRecord.read(sendBuffer, 0, sendlen);
                                    calc1(sendBuffer, 0, sendlen);
                                    LibAPI.SendVoiceData(sendBuffer, sendlen);
                                } else
                                    Thread.sleep(50);
                            }// end while(!isExit)
                            audioRecord.stop();
                            Log.i(TAG, "stop ---> audioRecord Thread");
                        }// end if INITIALIZED

                        audioRecord.release();
                        audioRecord = null;
                        if (isTalking) {
                            isTalking = !isTalking;
                            int ret = LibAPI.StopChart();
                            Log.i(TAG, "stopDualTalk return " + ret);
                        }
                        return;
                    } catch (Exception e4) {    // Do nothing
                        e4.printStackTrace();
                        Log.e(TAG, "StartRecord_Aduio " + e4.toString());
                    }
                }
            }
        }
    }

    // 2014-9-01 LinZh107 add Audio decoder
    private void threadRunDecodeAudio() {
        try {
            int ret = 0;
            int sampleRateInHz = 8000;
            final int frame_cnt = 8;
            int bufferSize = 0;

            while (true && !isActivityExit) {
                ret = LibAPI.GetAudioParam(i_audioParams, i_listIndex);
                if (ret != 0)
                    Thread.sleep(20);
                else break;
            }

            bufferSize = frame_cnt * i_audioParams[0] * i_audioParams[1];

            mTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRateInHz, AudioFormat.CHANNEL_OUT_MONO,
                    AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
            if (mTrack.getState() == AudioTrack.STATE_INITIALIZED) {
                mTrack.play();
                while (!isActivityExit) {
                    if (isTalking)
                        s_audioDataArray = LibAPI.GetAudioFrame(frame_cnt, i_listIndex);
                    else
                        s_audioDataArray = null;
                    if (s_audioDataArray != null && s_audioDataArray.length == bufferSize) {
                        //Log.i(TAG,"s_audioDataArray.length=" +	s_audioDataArray.length);
                        calc1(s_audioDataArray, 0, bufferSize);
                        mTrack.write(s_audioDataArray, 0, s_audioDataArray.length);
                    } else {
                        Thread.sleep(10);
                    }
                }
                mTrack.stop();// 停止播放
            }
            mTrack.release();// 释放底层资源。
            mTrack = null;
        } catch (Exception e1) {
            e1.printStackTrace();
        }
    }

    //音频降噪处理
    public void calc1(short[] lin, int off, int len) {
        int i, j;
        for (i = 0; i < len; i++) {
            j = lin[i + off];
            lin[i + off] = (short) (j >> 2);
        }
    }

    // 2015-5-19 LinZh107 add for open multiple video in sametime
    private void threadRunOpenVideo(int camindex, int viindex) {
        if (LibAPI.StartPlay(camindex, viindex) == 0) {
            // start succeed. remove this one from the toStartArray
            m_videoArray.get(viindex).isStarted = true;
            PlaySFHCallBack sfhcb = (PlaySFHCallBack) m_playAdapter.getItem(viindex);
            if (sfhcb != null) {
                Log.d(TAG, "Runnable open video index OK = " + viindex);
                sfhcb.isStoped = false;
            }
        } else {
            // start failed. then stop and clean the cache for next start
            LibAPI.StopPlay(viindex);
            // notify the ctrlThread to start a new open thread.
            synchronized (m_startPlayLock) {
                m_videoArray.get(viindex).retryCount++;
                Log.i(TAG, "Thread run m_startPlayLock.notify " + viindex);
                m_startPlayLock.notify();
            }
        }
        if (isActivityExit)
            LibAPI.StopPlay(viindex);
        mHandler.sendEmptyMessage(MSG_DISMISSDIALOG);
    }

}// end PlayActivity class
