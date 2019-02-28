package com.topvs.platform;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Timer;
import java.util.TimerTask;
import com.topvs.expandablelist.MyExpandableListView;
import com.topvs.expandablelist.MyExpandableListAdapter;
import com.topvs.expandablelist.MyExpandableListAdapter.ViewHolder;
import com.topvs.player.FileListActivity;
import com.topvs.player.PlayActivity;
import android.annotation.SuppressLint;
import android.app.ProgressDialog;
import android.app.TabActivity;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View.OnClickListener;
import android.view.WindowManager;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ExpandableListView;
import android.widget.ExpandableListView.OnChildClickListener;
import android.widget.ExpandableListView.OnGroupClickListener;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.Switch;
import android.widget.SearchView.OnQueryTextListener;
import android.widget.SimpleAdapter;
import android.widget.TabHost;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TabHost.OnTabChangeListener;

public class DeviceListActivity extends TabActivity
        implements OnTabChangeListener, OnQueryTextListener {
    /**
     * logcat printf tag
     */
    private static final String TAG = "DeviceListActivity";
    public static final int REQUEST_CODE = 1;

    /**
     * handler message type
     */
    private final int RET_OK = 0;
    private final int LIST_OK = 1;
    private final int ALARM_TRUE = 5;
    private final int ALARM_FALSE = 6;
    private final int SEARCHVIEW_APPEAR = 7;
    private final int SEARCHVIEW_DISAPPEAR = 8;
    private final int SELECT_NON = 9;
    private final int MAX_OPEN_COUNT = 16;

    private Thread m_getDevListThread;

    /**
     * timer for getting the ck_AlarmInfo from frent
     * add Linzh107 2014-6-12
     */
    private Timer mAlarmTimer = null;
    private TimerTask mTimerTask = null;
    private final int DELAY = 500; // 0.5s
    private final int PERIOD = 1000; // 1s
    public static ck_AlarmInfo mAlarmInfo;
    private boolean bDisplayAlarm = false;

    public static int MAX_ALARMINFO_COUNT = 32; // 设备套数
    public static int iAlarmIDArray[] = new int[MAX_ALARMINFO_COUNT];    //告警类ID
    public static String AlarmDateArray[] = new String[MAX_ALARMINFO_COUNT * 2];    // 告警发生时间
    public static String AlarmTypeArray[][] = new String[MAX_ALARMINFO_COUNT][6];    //告警类型名
    private int AlarmTimeArray[] = new int[MAX_ALARMINFO_COUNT];
    private int iAlarmIDIndex = 0;                        // 告警信息设备下标
    protected int iAlarmTypeIndex = 0;                    // 告警信息下标，即type

    private final SimpleDateFormat mSDFormat = new SimpleDateFormat("MM/dd");
    private final SimpleDateFormat mSDTime = new SimpleDateFormat("HH:mm:ss");

    private TableLayout mTableLayout;                    // 告警输出表格
    private int MAX_TABLEROW = 0;                        // 实际行数


    private ProgressDialog mProgressDialog = null;
    private Handler mHandler = null;
    private int iCameraIndex = -1;

    private int iInSDCard;

    private LinearLayout mLayoutDevice;
    private LinearLayout mLayoutDeviceSch;
    private LinearLayout mLayoutRecord;
    private LinearLayout mLayoutPictrue;
    private LinearLayout mLayoutAlarmInfo;
    private LinearLayout mLayoutSWOut;
    private LinearLayout mLayoutSWIn;

    private TabHost myTabhost;
    private int iTtabIndex;

    private ListView mListView;
    private MyExpandableListView mDevListView;
    private MyExpandableListAdapter mDevListAdapter;
    private int iExpandFlag;
    private SimpleAdapter mSimpleAdapter;

    private boolean isSeeking;
    private ListView mSeekListView;
    private SearchView mSearchView;
    private ArrayAdapter<String> mSeekAdapter;

    // 2015-4-21 LinZh107
    private ArrayList<HashMap<String, Object>> mGroupItemArray;    // ListView's group adapter members
    private ArrayList<HashMap<String, Object>> mChildrenItemArray;    // ListView's child adapter members
    private ArrayList<Integer> mSelectedCamList;    // store the selected pu_id

    /**
     * Set the max area node number(AREA_GROUP_NUM) here ,
     * while the first index is use for the group count,
     * so it should plus 1.
     */
    final int AREA_GROUP_NUM = 101;
    final int MAX_NODE_NUM = 2000;    //max device number

    public int[] DevNumArray = new int[3];
    public int[] SWiStatusArray = new int[16];    // max input i/o number
    public int[] SWoStatusArray = new int[16];    // max output i/o  number
    public int[] CamStatusArray = new int[MAX_NODE_NUM];// max camera's number
    public int[] AreaChildArray = new int[AREA_GROUP_NUM];
    public String[] ArrayAreaStr = new String[AREA_GROUP_NUM];    // area_name use for listView display
    public String[] ArrayGUName = new String[MAX_NODE_NUM];    // pu_name use for listView display
    public String[] ArrayGUID = new String[MAX_NODE_NUM];    // pu_id use for listView display
    public String[] Arraypuid;                    // pu_name use for seekList

    private Button mBtnStartPreview;
    private ImageButton mBtnHelp;
    // private ImageButton btnShare;
    // private TextView barTitle;

    private Resources mResources;
    private Switch[] mSWinGroup;
    private Switch[] mSWoutGroup;
    private TableLayout mTableLayoutSWO1;
    private TableLayout mTableLayoutSWO2;
    private TableLayout mTableLayoutSWI1;
    private TableLayout mTableLayoutSWI2;
    public Point mPoint = new Point();
    private AbsListView.LayoutParams mListItemLP;


    /**
     * Custom MyExpandableListView 's GroupClickListener
     */
    class GroupClickListener implements OnGroupClickListener {
        @Override
        public boolean onGroupClick(ExpandableListView parent, View v,
                                    int groupPosition, long id) {
            if (iExpandFlag == -1) {
                // 展开被选的group
                mDevListView.expandGroup(groupPosition);
                // 设置被选中的group置于顶端
                mDevListView.setSelectedGroup(groupPosition);
                iExpandFlag = groupPosition;
            } else if (iExpandFlag == groupPosition) {
                mDevListView.collapseGroup(iExpandFlag);
                iExpandFlag = -1;
            } else {
                mDevListView.collapseGroup(iExpandFlag);
                // 展开被选的group
                mDevListView.expandGroup(groupPosition);
                // 设置被选中的group置于顶端
                mDevListView.setSelectedGroup(groupPosition);
                iExpandFlag = groupPosition;
            }
            return true;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        Bundle extras = getIntent().getExtras();
        iInSDCard = extras.getInt("INSDCARD");
        mResources = getResources();

        /************** Create the Message Handler *************/
        CreateHandleMessage();

        /***** Create a new thread to get the devices list *****/
        m_getDevListThread = new Thread(new Runnable() {
            public void run() {
                threadRunGetDevList();
            }
        });
        m_getDevListThread.start();


        myTabhost = this.getTabHost();
        LayoutInflater.from(this).inflate(R.layout.devicelist, myTabhost.getTabContentView(), true);

        iTtabIndex = 1; //choose the first tab by default

        // make a new Tab
        myTabhost.addTab(myTabhost.newTabSpec("One").setIndicator(mResources.getString(R.string.dev_RTVideo),
                mResources.getDrawable(R.drawable.realtime_camera)).setContent(R.id.widget_layout_device));

        // 2014-6-11 LinZh107 add for ck_AlarmInfo
        myTabhost.addTab(myTabhost.newTabSpec("Two").setIndicator(mResources.getString(R.string.dev_RCVideo),
                mResources.getDrawable(R.drawable.record)).setContent(R.id.widget_layout_record));

        myTabhost.addTab(myTabhost.newTabSpec("Three").setIndicator(mResources.getString(R.string.dev_RCPhoto),
                mResources.getDrawable(R.drawable.takephoto)).setContent(R.id.widget_layout_photo));

        myTabhost.addTab(myTabhost.newTabSpec("Four").setIndicator(mResources.getString(R.string.dev_RTAlarmInfo),
                mResources.getDrawable(R.drawable.mesurement)).setContent(R.id.widget_layout_info));

        myTabhost.addTab(myTabhost.newTabSpec("Fine").setIndicator(mResources.getString(R.string.dev_NetSwitchOut),
                mResources.getDrawable(R.drawable.switch_out)).setContent(R.id.layout_switch_out));

        myTabhost.addTab(myTabhost.newTabSpec("Six").setIndicator(mResources.getString(R.string.dev_NetSwitchIn),
                mResources.getDrawable(R.drawable.switch_in)).setContent(R.id.layout_switch_in));

        mLayoutDevice = (LinearLayout) findViewById(R.id.widget_layout_device);
        mLayoutDeviceSch = (LinearLayout) findViewById(R.id.widget_layout_device_sch);
        mLayoutRecord = (LinearLayout) findViewById(R.id.widget_layout_record);
        mLayoutPictrue = (LinearLayout) findViewById(R.id.widget_layout_photo);
        mLayoutAlarmInfo = (LinearLayout) findViewById(R.id.widget_layout_info);
        mLayoutSWOut = (LinearLayout) findViewById(R.id.layout_switch_out);
        mLayoutSWIn = (LinearLayout) findViewById(R.id.layout_switch_in);

        myTabhost.setOnTabChangedListener(this);

        /**
         * get the List item LayoutParams
         * fix for mutiple android devices
         * note: can't move above
         * */
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.devsearchbar);
        WindowManager wm = this.getWindowManager();
        wm.getDefaultDisplay().getSize(mPoint);
        int height = mPoint.y;
        if (height > 900)
            height /= 20;
        else
            height /= 14;
        mListItemLP = new AbsListView.LayoutParams(AbsListView.LayoutParams.MATCH_PARENT, height);

        /**
         * Create the search tool
         * */
        mSearchView = (SearchView) findViewById(R.id.searchViewCam);
        mSearchView.setOnQueryTextListener(this);
        mLayoutDeviceSch.setVisibility(View.GONE);
        isSeeking = false;

        // barTitle = (TextView)findViewById(R.id.textViewTitleBar);
        // btnShare = (ImageButton) findViewById(R.id.btnShare);
        // btnShare.setOnClickListener(onClickListener);
        mBtnHelp = (ImageButton) findViewById(R.id.btnHelp);
        mBtnHelp.setOnClickListener(onClickListener);
        mBtnStartPreview = (Button) findViewById(R.id.startRVP);
        mBtnStartPreview.setOnClickListener(onClickListener);

    }// end onCreate()

    @SuppressLint("HandlerLeak")
    private void CreateHandleMessage() {
        mHandler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case LIST_OK: {
                        CreatePlayListView();        // 创建实时视频列表
                        CreateRecordListView();        // 创建视频录像列表
                        CreatePhotosListView();        // 创建视频快照列表
                        CreateInfoListView(0);        // 创建测控信息输出列表，参数是新增table行数
                        CreateTimerTask();            // 创建定时器用于更新测控信息列表内容
                        CreateNetSwitchLayout();    // 创建网络开关列表
                        break;
                    }
                    case RET_OK: {
                        mProgressDialog.dismiss();
                        Intent intent = new Intent(DeviceListActivity.this, PlayActivity.class);
                        intent.putExtra("INDEX", iCameraIndex);
                        intent.putExtra("INSDCARD", iInSDCard);
                        intent.putExtra("GUID", ArrayGUID[iCameraIndex]);
                        intent.putExtra("name", ArrayGUName[iCameraIndex]);
                        startActivity(intent);
                        // startActivityForResult(intent, REQUEST_CODE); //有bug ==> 第二次运行会崩溃
                        break;
                    }
                    case -3: {
                        mProgressDialog.dismiss();
                        Toast.makeText(DeviceListActivity.this, "RealVideo Preview Start error!", Toast.LENGTH_SHORT)
                                .show();
                        break;
                    }
                    case -11: {
                        mProgressDialog.dismiss();
                        Toast.makeText(DeviceListActivity.this, "Create StartReq CMD error!", Toast.LENGTH_SHORT)
                                .show();
                        break;
                    }
                    case -12: {
                        mProgressDialog.dismiss();
                        Toast.makeText(DeviceListActivity.this, "Wait for response timeout!", Toast.LENGTH_SHORT)
                                .show();
                        break;
                    }
                    case -13:
                    case -14: {
                        mProgressDialog.dismiss();
                        Toast.makeText(DeviceListActivity.this, "Failed to analyze the ipAddr!",
                                Toast.LENGTH_SHORT).show();
                        break;
                    }

                    case ALARM_TRUE: {
                        iAlarmIDIndex = 0;
                        // 识别设备ID iAlarmIDIndex
                        if (iAlarmIDArray != null) {
                            while (iAlarmIDIndex < MAX_ALARMINFO_COUNT) {
                                if (iAlarmIDArray[iAlarmIDIndex] == 0) {
                                    iAlarmIDArray[iAlarmIDIndex] = mAlarmInfo.devid;
                                }
                                if (iAlarmIDArray[iAlarmIDIndex] == mAlarmInfo.devid)
                                    break;
                                else iAlarmIDIndex++;
                            }
                            if (iAlarmIDIndex < MAX_ALARMINFO_COUNT
                                    && AlarmTimeArray[iAlarmIDIndex] != mAlarmInfo.alarmtime) {
                                // 更新列表
                                AlarmTimeArray[iAlarmIDIndex] = mAlarmInfo.alarmtime;
                                if (iAlarmIDArray[iAlarmIDIndex] != 0)
                                    UpdateInfoListView(iAlarmIDIndex);
                            }
                        }
                        break;
                    }

                    case ALARM_FALSE: {
                        Toast.makeText(DeviceListActivity.this, "" + mAlarmInfo.devid, Toast.LENGTH_SHORT).show();
                        break;
                    }
                    case SEARCHVIEW_APPEAR: {
                        if (iTtabIndex != 4) {
                            mLayoutDeviceSch.setVisibility(View.VISIBLE);
                            mLayoutDevice.setVisibility(View.GONE);
                            mLayoutRecord.setVisibility(View.GONE);
                            mLayoutPictrue.setVisibility(View.GONE);
                            mLayoutAlarmInfo.setVisibility(View.GONE);
                            mLayoutSWOut.setVisibility(View.GONE);
                            mLayoutSWIn.setVisibility(View.GONE);
                        }
                        break;
                    }
                    case SEARCHVIEW_DISAPPEAR: {
                        mLayoutDeviceSch.setVisibility(View.GONE);
                        if (iTtabIndex == 1)
                            mLayoutDevice.setVisibility(View.VISIBLE);
                        else if (iTtabIndex == 2)
                            mLayoutRecord.setVisibility(View.VISIBLE);
                        else if (iTtabIndex == 3)
                            mLayoutPictrue.setVisibility(View.VISIBLE);
                        else if (iTtabIndex == 4)
                            mLayoutAlarmInfo.setVisibility(View.VISIBLE);
                        else if (iTtabIndex == 5)
                            mLayoutSWOut.setVisibility(View.VISIBLE);
                        else if (iTtabIndex == 6)
                            mLayoutSWIn.setVisibility(View.VISIBLE);
                        break;
                    }
                    case SELECT_NON: {
                        Toast.makeText(DeviceListActivity.this, mResources.getString(R.string.msg_selectfirst),
                                Toast.LENGTH_SHORT).show();
                        break;
                    }
                    case MAX_OPEN_COUNT: {
                        Toast.makeText(DeviceListActivity.this, mResources.getString(R.string.msg_maxselect),
                                Toast.LENGTH_SHORT).show();
                        break;
                    }

                    default: {
                        mProgressDialog.dismiss();
                        Toast.makeText(DeviceListActivity.this, mResources.getString(R.string.msg_loginfailed) + msg.what,
                                Toast.LENGTH_SHORT).show();
                        break;
                    }
                    // add LinZh107 2014-6-11 begin
                }
            }
        };
    }

    private void threadRunGetDevList() {
        // TODO Auto-generated method stub
        LibAPI.GetDeviceList(DevNumArray, CamStatusArray, SWiStatusArray, SWoStatusArray,
                AreaChildArray, ArrayAreaStr, ArrayGUID, ArrayGUName);

        Arraypuid = new String[DevNumArray[0]];
        mAlarmInfo = new ck_AlarmInfo();
        mHandler.sendEmptyMessage(LIST_OK);

    }

    private void CreatePlayListView() {
        /*
		 * modify the listview to a ExpandableListView
		 * LinZh107 on 2015-8-15
		 * */
        mDevListView = (MyExpandableListView) findViewById(R.id.dev_listview);
        mSeekListView = (ListView) findViewById(R.id.dev_searchview);

        // 生成动态数组，加入数据
        mGroupItemArray = new ArrayList<HashMap<String, Object>>();
        mChildrenItemArray = new ArrayList<HashMap<String, Object>>();
        String tmp = "";
        //crate the group member info
        for (int i = 0; i < AreaChildArray[100]; i++) {
            //if this group didn't have any children , it would be showed
            if (AreaChildArray[i] > 0) {
                HashMap<String, Object> groupItemMap = new HashMap<String, Object>();
                groupItemMap.put("GroupName", ArrayAreaStr[i] + " [" + AreaChildArray[i] + "]");
                groupItemMap.put("ChildCnt", AreaChildArray[i]);
                mGroupItemArray.add(groupItemMap);
            }
        }
        //crate the children's member array, insert to a single list
        for (int i = 0; i < DevNumArray[0]; i++) {
            HashMap<String, Object> childItemMap = new HashMap<String, Object>();
            if (CamStatusArray[i] == 0x1002) {
                childItemMap.put("ItemImage", R.drawable.notonline);// 图像资源的ID
                tmp = "(OFF)";
            } else {
                childItemMap.put("ItemImage", R.drawable.online);// 图像资源的ID
                tmp = "(ON)";
            }
            childItemMap.put("ItemTitle", " " + ArrayGUName[i]); // GUName
            mChildrenItemArray.add(childItemMap);

            if (i < 10)
                Arraypuid[i] = "00" + i + ' ' + ArrayGUName[i] + tmp;
            else if (i < 100)
                Arraypuid[i] = "0" + i + ' ' + ArrayGUName[i] + tmp;
            else Arraypuid[i] = "" + i + ' ' + ArrayGUName[i] + tmp;
        }

        //设置悬浮头部VIEW
        mDevListView.setHeaderView(getLayoutInflater().inflate(R.layout.devicelisthead, mDevListView, false), mListItemLP);
        mDevListAdapter = new MyExpandableListAdapter(this, mDevListView, mGroupItemArray, mChildrenItemArray, mListItemLP);
        mDevListView.setAdapter(mDevListAdapter);    // 添加并且显示
        mDevListView.setOnChildClickListener(onChildItemClickListener);
        //if(mDevListAdapter.getGroupCount() > 0)
        //	mDevListView.expandGroup(0);

        mSelectedCamList = new ArrayList<Integer>();

        // 2014-11-27
        mSeekAdapter = new ArrayAdapter<String>(getApplicationContext(), R.layout.devseeklistiterm, Arraypuid);
        mSeekListView.setAdapter(mSeekAdapter);
        mSeekListView.setTextFilterEnabled(true);
        mSeekListView.setOnItemClickListener(onSerItermClickListener);
    }

    private void CreateRecordListView() {
		/*
		mListView = (ListView) findViewById(R.id.record_listview);
		// 生成动态数组，加入数据
		mChildrenItemArray = new ArrayList<HashMap<String, Object>>();
		for (int i = 0; i < DevNumArray[0]; i ++) {
			HashMap<String, Object> map = new HashMap<String, Object>();
			map.put("ItemImage", R.drawable.videos);// 图像资源的ID
			map.put("ItemTitle", ArrayGUName[i]);
			mChildrenItemArray.add(map);
		}
		// 生成适配器的Item和动态数组对应的元素
		mSimpleAdapter = new SimpleAdapter(this, mChildrenItemArray,// 数据源
		        R.layout.devicerecorditem,// ListItem的XML实现
		        // 动态数组与ImageItem对应的子项
		        new String[]{"ItemImage", "ItemTitle"},
		        // ImageItem的XML文件里面的一个ImageView,两个TextView ID
		        new int[]{R.id.dev_Image, R.id.dev_Title});
		// 添加并且显示
		mListView.setAdapter(mSimpleAdapter);
		// 添加点击
		mListView.setOnItemClickListener(onRecordItemClick);
		*/
		
		
		/*
		 * modify the listview to a ExpandableListView
		 * LinZh107 on 2015-8-15
		 * */

        mDevListView = (MyExpandableListView) findViewById(R.id.record_listview);
        // 生成动态数组，加入数据
        mGroupItemArray = new ArrayList<HashMap<String, Object>>();
        mChildrenItemArray = new ArrayList<HashMap<String, Object>>();
        //crate the group member info
        for (int i = 0; i < AreaChildArray[100]; i++) {
            //if this group didn't have any children , it would be showed
            if (AreaChildArray[i] > 0) {
                HashMap<String, Object> groupItemMap = new HashMap<String, Object>();
                groupItemMap.put("GroupName", ArrayAreaStr[i] + " [" + AreaChildArray[i] + "]");
                groupItemMap.put("ChildCnt", AreaChildArray[i]);
                mGroupItemArray.add(groupItemMap);
            }
        }
        //crate the children's member array, insert to a single list
        for (int i = 0; i < DevNumArray[0]; i++) {
            HashMap<String, Object> childItemMap = new HashMap<String, Object>();
            childItemMap.put("ItemImage", R.drawable.videos);// 图像资源的ID
            childItemMap.put("ItemTitle", ArrayGUName[i]);
            mChildrenItemArray.add(childItemMap);
        }

        //设置悬浮头部VIEW
        mDevListView.setHeaderView(getLayoutInflater().inflate(R.layout.devicelisthead, mDevListView, false), mListItemLP);
        mDevListAdapter = new MyExpandableListAdapter(this, mDevListView, mGroupItemArray, mChildrenItemArray, mListItemLP);
        mDevListAdapter.checkBoxVisible = false;
        mDevListView.setAdapter(mDevListAdapter);    // 添加并且显示
        mDevListView.setOnChildClickListener(new OnChildClickListener() {
            @Override
            public boolean onChildClick(ExpandableListView parent, View v,
                                        int groupPosition, int childPosition, long id) {
                iCameraIndex = (int) mDevListAdapter.getChildId(groupPosition, childPosition);
                Intent in = new Intent(DeviceListActivity.this, FileListActivity.class);
                in.putExtra("IsPhotos", false);
                in.putExtra("GUID", ArrayGUID[iCameraIndex]);
                in.putExtra("GUName", ArrayGUName[iCameraIndex]);
                startActivity(in);
                return true;
            }
        });
		/**/
    }

    private void CreatePhotosListView() {
		/*
		mListView = (ListView) findViewById(R.id.photo_listview);
		// 生成动态数组，加入数据
		mChildrenItemArray = new ArrayList<HashMap<String, Object>>();
		for (int i = 0; i < DevNumArray[0]; i ++) {
			HashMap<String, Object> map = new HashMap<String, Object>();
			map.put("ItemImage", R.drawable.photos);// 图像资源的ID
			map.put("ItemTitle", ArrayGUName[i]);
			mChildrenItemArray.add(map);
		}
		
		// 生成适配器的Item和动态数组对应的元素
		mSimpleAdapter = new SimpleAdapter(this, mChildrenItemArray,// 数据源
		        R.layout.devicerecorditem,// ListItem的XML实现
		        // 动态数组与ImageItem对应的子项
		        new String[]{"ItemImage", "ItemTitle"},
		        // ImageItem的XML文件里面的一个ImageView,两个TextView ID
		        new int[]{R.id.dev_Image, R.id.dev_Title});

		// 添加并且显示
		mListView.setAdapter(mSimpleAdapter);

		// 添加点击
		mListView.setOnItemClickListener(onRecordItemClick);
		*/
		
		/*
		 * modify the listview to a ExpandableListView
		 * LinZh107 on 2015-8-15
		 * */

        mDevListView = (MyExpandableListView) findViewById(R.id.photo_listview);
        // 生成动态数组，加入数据
        mGroupItemArray = new ArrayList<HashMap<String, Object>>();
        mChildrenItemArray = new ArrayList<HashMap<String, Object>>();
        //crate the group member info
        for (int i = 0; i < AreaChildArray[100]; i++) {
            //if this group didn't have any children , it would be showed
            if (AreaChildArray[i] > 0) {
                HashMap<String, Object> groupItemMap = new HashMap<String, Object>();
                groupItemMap.put("GroupName", ArrayAreaStr[i] + " [" + AreaChildArray[i] + "]");
                groupItemMap.put("ChildCnt", AreaChildArray[i]);
                mGroupItemArray.add(groupItemMap);
            }
        }
        //crate the children's member array, insert to a single list
        for (int i = 0; i < DevNumArray[0]; i++) {
            HashMap<String, Object> childItemMap = new HashMap<String, Object>();
            childItemMap.put("ItemImage", R.drawable.photos);// 图像资源的ID
            childItemMap.put("ItemTitle", ArrayGUName[i]);
            mChildrenItemArray.add(childItemMap);
        }

        //设置悬浮头部VIEW
        mDevListView.setHeaderView(getLayoutInflater().inflate(R.layout.devicelisthead, mDevListView, false), mListItemLP);
        mDevListAdapter = new MyExpandableListAdapter(this, mDevListView, mGroupItemArray, mChildrenItemArray, mListItemLP);
        mDevListAdapter.checkBoxVisible = false;
        mDevListView.setAdapter(mDevListAdapter);    // 添加并且显示
        mDevListView.setOnChildClickListener(new OnChildClickListener() {
            @Override
            public boolean onChildClick(ExpandableListView parent, View v,
                                        int groupPosition, int childPosition, long id) {
                iCameraIndex = (int) mDevListAdapter.getChildId(groupPosition, childPosition);
                Intent in = new Intent(DeviceListActivity.this, FileListActivity.class);
                in.putExtra("IsPhotos", true);
                in.putExtra("GUID", ArrayGUID[iCameraIndex]);
                in.putExtra("GUName", ArrayGUName[iCameraIndex]);
                startActivity(in);
                return true;
            }
        });
		/**/
    }

    private void CreateInfoListView(int count) {
        mTableLayout = (TableLayout) findViewById(R.id.widget_layout_info);
        mTableLayout.setStretchAllColumns(true);
        for (int row = count; row < MAX_TABLEROW; row++) {
            //when the list's row is not enought, should add a new row
            TableRow tablerow = new TableRow(this);
            if (row % 2 == 0)
                tablerow.setBackgroundColor(Color.rgb(240, 250, 250));
            for (int list = 0; list < 4; list++) {
                TextView tv = new TextView(this);
                tv.setTextSize(14);
                if (list == 0) {
                    if (row < 9)
                        tv.setText(" 0" + (row + 1));
                    else tv.setText(" " + (row + 1));
                } else tv.setText("value");
                tablerow.addView(tv);
            }
            mTableLayout.addView(tablerow, new TableLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.MATCH_PARENT));
        }
    }

    @SuppressLint("NewApi")
    private void CreateNetSwitchLayout() {
        // TODO Auto-generated method stub
        mSWinGroup = new Switch[DevNumArray[1]];
        mSWoutGroup = new Switch[DevNumArray[2]];

        int height = mPoint.y;
        if (height > 900)
            height /= 16;
        else
            height /= 12;
        TableRow.LayoutParams swRowLayoutParam = new TableRow.LayoutParams(
                TableRow.LayoutParams.MATCH_PARENT, height);

        int i = 0;
        int list = 2;
        int row = DevNumArray[1] / list;
        mTableLayoutSWI1 = (TableLayout) findViewById(R.id.table_switch_in1);
        mTableLayoutSWI2 = (TableLayout) findViewById(R.id.table_switch_in2);
        for (i = 0; i < row; i++) {
            TableRow tablerow1 = new TableRow(this);
            Switch sw1 = new Switch(this);
            sw1.setId(i);
            sw1.setText(String.valueOf(i + 1));
            sw1.setTextColor(Color.BLACK);
            sw1.setTextAlignment(View.TEXT_ALIGNMENT_TEXT_END);
            sw1.setChecked(SWiStatusArray[i] == 1);
            sw1.setEnabled(false);
            sw1.setLayoutParams(swRowLayoutParam);
            tablerow1.addView(sw1);
            mSWinGroup[i] = sw1;
            mTableLayoutSWI1.addView(tablerow1);

            TableRow tablerow2 = new TableRow(this);
            Switch sw2 = new Switch(this);
            sw2.setId(i + row);
            sw2.setText(String.valueOf(i + row + 1));
            sw2.setTextColor(Color.BLACK);
            sw2.setTextAlignment(View.TEXT_ALIGNMENT_TEXT_END);
            sw2.setChecked(SWiStatusArray[i + row] == 1);
            sw2.setEnabled(false);
            sw2.setLayoutParams(swRowLayoutParam);
            tablerow2.addView(sw2);
            mSWinGroup[i + row] = sw2;
            mTableLayoutSWI2.addView(tablerow2);
        }

        row = DevNumArray[2] / list;
        mTableLayoutSWO1 = (TableLayout) findViewById(R.id.table_switch_out1);
        mTableLayoutSWO2 = (TableLayout) findViewById(R.id.table_switch_out2);
        for (i = 0; i < row; i++) {
            TableRow tablerow1 = new TableRow(this);
            Switch sw1 = new Switch(this);
            sw1.setId(i);
            sw1.setText(String.valueOf(i + 1));
            sw1.setTextColor(Color.BLACK);
            sw1.setTextAlignment(View.TEXT_ALIGNMENT_TEXT_END);
            sw1.setChecked(SWoStatusArray[i] == 1);
            sw1.setOnClickListener(onSwClickListener);
            sw1.setOnCheckedChangeListener(onSwChangeListener);
            sw1.setLayoutParams(swRowLayoutParam);
            tablerow1.addView(sw1);
            mSWoutGroup[i] = sw1;
            mTableLayoutSWO1.addView(tablerow1);

            TableRow tablerow2 = new TableRow(this);
            Switch sw2 = new Switch(this);
            sw2.setId(i + row);
            sw2.setText(String.valueOf(i + row + 1));
            sw2.setTextColor(Color.BLACK);
            sw2.setTextAlignment(View.TEXT_ALIGNMENT_TEXT_END);
            sw2.setChecked(SWoStatusArray[i + row] == 1);
            sw2.setOnClickListener(onSwClickListener);
            sw2.setOnCheckedChangeListener(onSwChangeListener);
            sw2.setLayoutParams(swRowLayoutParam);
            tablerow2.addView(sw2);
            mSWoutGroup[i + row] = sw2;
            mTableLayoutSWO2.addView(tablerow2);
        }
    }

    private void CreateTimerTask() {
        // 2014-6-10 定时接收报警信息
        if (mTimerTask == null) {
            mTimerTask = new TimerTask() {    // TimerTask 是个抽象类,实现的是Runable类
                @Override
                public void run() {
                    // 定义一个消息传过去
                    if (0 == LibAPI.GetdevAlarmInfo(mAlarmInfo)) {
                        mHandler.sendEmptyMessage(ALARM_TRUE);
                    }
                }
            };

            if (mAlarmTimer == null) {
                mAlarmTimer = new Timer();
                mAlarmTimer.schedule(mTimerTask, DELAY, PERIOD);
            }
        }
    }

    private void CancelTimerTask() {
        if (mAlarmTimer != null) {
            mAlarmTimer.cancel();
            mAlarmTimer = null;
        }
        if (mTimerTask != null) {
            mTimerTask.cancel();
            mTimerTask = null;
        }
    }

    private void UpdateInfoListView(int nindex) {
        iAlarmTypeIndex = nindex;
        int curRow = iAlarmTypeIndex * 6;
        if (curRow + 6 > MAX_TABLEROW) {
            int SumTable = MAX_TABLEROW;
            MAX_TABLEROW = curRow + 6;
            CreateInfoListView(SumTable);
        }
        if (AlarmTypeArray != null) {
            AlarmTypeArray[iAlarmTypeIndex][0] = "空气温度:" + mAlarmInfo.airtemp + " ℃";
            AlarmTypeArray[iAlarmTypeIndex][1] = "空气湿度:" + mAlarmInfo.airhumi + " %";
            AlarmTypeArray[iAlarmTypeIndex][2] = "土壤温度:" + mAlarmInfo.soiltemp + " ℃";
            AlarmTypeArray[iAlarmTypeIndex][3] = "土壤湿度:" + mAlarmInfo.soilhumi + " %";
            AlarmTypeArray[iAlarmTypeIndex][4] = "光照强度:" + mAlarmInfo.illuminance + " lux";
            AlarmTypeArray[iAlarmTypeIndex][5] = "CO2浓度:" + mAlarmInfo.CO2density + " ppm";

            Date date = new Date((long) mAlarmInfo.alarmtime * 1000);
            AlarmDateArray[iAlarmTypeIndex * 2] = mSDFormat.format(date);
            AlarmDateArray[iAlarmTypeIndex * 2 + 1] = mSDTime.format(date);
            TableRow tmpRow = null;
            TextView tv = null;
            // 更新告警列值
            if (bDisplayAlarm) {
                for (int row = curRow; row < curRow + 6; row++) {
                    tmpRow = (TableRow) mTableLayout.getChildAt(row + 1);
                    tv = (TextView) tmpRow.getChildAt(3);// 更新告警列值
                    tv.setText(AlarmTypeArray[iAlarmTypeIndex][row % 6]);

                    tv = (TextView) tmpRow.getChildAt(1);// 更新时间列值
                    tv.setText("...");
                    tv = (TextView) tmpRow.getChildAt(2);// 更新设备ID列值
                    tv.setText(String.format("%1$#10x", iAlarmIDArray[iAlarmTypeIndex]));
                }
                tmpRow = (TableRow) mTableLayout.getChildAt(curRow + 1);
                tv = (TextView) tmpRow.getChildAt(1);// 更新时间列 年月日值
                tv.setText(AlarmDateArray[iAlarmTypeIndex * 2]);

                tmpRow = (TableRow) mTableLayout.getChildAt(curRow + 2);
                tv = (TextView) tmpRow.getChildAt(1);// 更新时间列 分秒值
                tv.setText(AlarmDateArray[iAlarmTypeIndex * 2 + 1]);
            }
        }
    }

    public OnItemClickListener onRecordItemClick = new OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
            Intent in = new Intent(DeviceListActivity.this, FileListActivity.class);
            in.putExtra("IsPhotos", true);
            in.putExtra("GUID", ArrayGUID[arg2]);
            in.putExtra("GUName", ArrayGUName[arg2]);
            startActivity(in);
        }
    };


    //child click listener for expandableList
    public OnChildClickListener onChildItemClickListener = new OnChildClickListener() {
        @Override
        public boolean onChildClick(ExpandableListView parent, View v,
                                    int groupPosition, int childPosition, long id) {
            iCameraIndex = (int) mDevListAdapter.getChildId(groupPosition, childPosition);
            Log.v(TAG, "Clicked the position is = " + iCameraIndex);
            if (CamStatusArray[iCameraIndex] != 0x1002) // 设备在线的话就拉视频，否则不执行
            {
                // 2015-4-20 14:56:39 LinZh107 add multiple play view support
                ViewHolder holder = (ViewHolder) v.getTag();
                holder.cBox.toggle();
                Integer obj = Integer.valueOf(iCameraIndex);
                // 同时修改map的值保存状态
                mDevListAdapter.selectedArray.put(iCameraIndex, holder.cBox.isChecked());
                if (holder.cBox.isChecked() == true)
                    mSelectedCamList.add(obj);
                else
                    mSelectedCamList.remove(obj);

                if (mSelectedCamList.size() > MAX_OPEN_COUNT)
                    mHandler.sendEmptyMessage(MAX_OPEN_COUNT);
                mBtnStartPreview.setText(mResources.getString(R.string.btn_play_cam) + "(" + mSelectedCamList.size() + ")");
            }
            return true;
        }
    };

    // devices list item click listener
    public OnItemClickListener onDevItemClickListener = new OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> arg0, View view, int position, long arg3) {
            iCameraIndex = position;
            Log.v(TAG, "Clicked the position is = " + position);

            if (CamStatusArray[position] != 0x1002) // 设备在线的话就拉视频，否则不执行
            {
                // 2015-4-20 14:56:39 LinZh107 add multiple play view support
                ViewHolder holder = (ViewHolder) view.getTag();
                holder.cBox.toggle();
                Integer obj = Integer.valueOf(position);
                // 同时修改map的值保存状态
                mDevListAdapter.selectedArray.put(position, holder.cBox.isChecked());
                if (holder.cBox.isChecked() == true)
                    mSelectedCamList.add(obj);
                else
                    mSelectedCamList.remove(obj);

                if (mSelectedCamList.size() > MAX_OPEN_COUNT)
                    mHandler.sendEmptyMessage(MAX_OPEN_COUNT);
                mBtnStartPreview.setText(mResources.getString(R.string.btn_play_cam) + "(" + mSelectedCamList.size() + ")");
            }
        }
    };

    // search list item click Listener
    public OnItemClickListener onSerItermClickListener = new OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> arg0, View view, int position, long arg3) {
            Log.v(TAG, "Have click the searchlist iterm!");
            String str = arg0.getItemAtPosition(position).toString();
            int indexNil = str.indexOf(' ');
            String subStr = str.substring(0, indexNil);
            iCameraIndex = Integer.valueOf(subStr);

            // play view
            if (iTtabIndex == 1) {
                if (CamStatusArray[iCameraIndex] != 0x1002) // 设备在线的话就拉视频，否则不执行
                {
                    // 2015-4-20 14:56:39 LinZh107 add multiple play view support
                    Integer obj = Integer.valueOf(iCameraIndex);
                    TextView tv = (TextView) view;
                    if (mSelectedCamList.contains(obj)) {
                        tv.setTextColor(Color.BLACK);
                        mDevListAdapter.selectedArray.put(iCameraIndex, false);
                        mSelectedCamList.remove(obj);
                    } else {
                        tv.setTextColor(Color.GREEN);
                        mDevListAdapter.selectedArray.put(iCameraIndex, true);
                        mSelectedCamList.add(obj);
                    }

                    if (mSelectedCamList.size() > MAX_OPEN_COUNT)
                        mHandler.sendEmptyMessage(MAX_OPEN_COUNT);
                    mBtnStartPreview.setText(mResources.getString(R.string.btn_play_cam) + "(" + mSelectedCamList.size() + ")");
                }
            }

            // record view
            else if (iTtabIndex == 2) {
                Intent in = new Intent(DeviceListActivity.this, FileListActivity.class);
                in.putExtra("IsPhotos", false);
                in.putExtra("GUID", ArrayGUID[iCameraIndex]);
                startActivity(in);
            }

            // photos view
            else if (iTtabIndex == 3) {
                Intent in = new Intent(DeviceListActivity.this, FileListActivity.class);
                in.putExtra("IsPhotos", true);
                in.putExtra("GUID", ArrayGUID[iCameraIndex]);
                startActivity(in);
            }

            // alarm info view
            else {
                mLayoutDeviceSch.setVisibility(View.GONE);
            }
        }
    };

    // Switch click listener
    public OnClickListener onSwClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            Log.v(TAG, "your have clicked the " + v.getId() + " switch!");
            mSWoutGroup[v.getId()].setChecked(mSWoutGroup[v.getId()].isChecked());
        }
    };

    // Switch checked change listener
    public OnCheckedChangeListener onSwChangeListener = new OnCheckedChangeListener() {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            // TODO Auto-generated method stub
            Log.v(TAG, "your have checked the " + buttonView.getId() + " switch!");
            if (mSWoutGroup[buttonView.getId()].isChecked())
                LibAPI.IoControl(SWoStatusArray[buttonView.getId()], 1, 1);
                //m_netswitch.output[buttonView.getId()] = (byte)(0x01);
            else
                LibAPI.IoControl(SWoStatusArray[buttonView.getId()], 0, 1);
            //m_netswitch.output[buttonView.getId()] = (byte)(0x00);
        }
    };

    // button clickListener
    public OnClickListener onClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            if (v.getId() == R.id.btnHelp) {
                Intent intent = new Intent(DeviceListActivity.this, ShowHelpActivity.class);
                DeviceListActivity.this.startActivity(intent);
            } else if (v.getId() == R.id.btnShare) {
                Intent it = new Intent(Intent.ACTION_VIEW);
                it.putExtra("sms_body", "下载地址：http://www.sztopvs.com/");
                it.setType("vnd.android-dir/mms-sms");
                startActivity(it);
            } else if (v.getId() == R.id.startRVP) {
                if (mSelectedCamList.isEmpty())
                    mHandler.sendEmptyMessage(SELECT_NON);
                else if (mSelectedCamList.size() > MAX_OPEN_COUNT)
                    mHandler.sendEmptyMessage(MAX_OPEN_COUNT);
                else {
                    //2015-4-28 Login Multiple Camera。
                    Intent intent = new Intent(DeviceListActivity.this, PlayActivity.class);
                    intent.putIntegerArrayListExtra("SELECTINDEX", mSelectedCamList);
                    intent.putExtra("GUSTATUSARRAY", CamStatusArray);
                    intent.putExtra("GUIDARRAY", ArrayGUID);
                    intent.putExtra("GUNAMEARRAY", ArrayGUName);
                    intent.putExtra("INSDCARD", iInSDCard);
                    startActivity(intent);
                }
            }
        }
    };

    public boolean onQueryTextChange(String newText) {
        // TODO Auto-generated method stub
        // TextUtils.isEmpty(string)判断（）里面string是否为空，或者为0字节
        // 若为空或0字节则返回false,否则为true
        if (TextUtils.isEmpty(newText)) {
            // Clear the text filter.
            mSeekListView.clearTextFilter();
            isSeeking = false;
            mHandler.sendEmptyMessage(SEARCHVIEW_DISAPPEAR);
        } else {
            // Sets the initial value for the text filter.
            mSeekListView.setFilterText(newText.toString());
            isSeeking = true;
            mHandler.sendEmptyMessage(SEARCHVIEW_APPEAR);
        }
        return true;
    }

    public boolean onQueryTextSubmit(String query) {
        // TODO Auto-generated method stub
        return true;
    }

    @Override
    public void onTabChanged(String tagString) {
        // TODO Auto-generated method stub
        mLayoutDeviceSch.setVisibility(View.GONE);
        mBtnStartPreview.setVisibility(View.INVISIBLE);
        bDisplayAlarm = false;
        CancelTimerTask();
        if (tagString.equals("One")) {
            iTtabIndex = 1;
            mBtnStartPreview.setVisibility(View.VISIBLE);
        } else if (tagString.equals("Two")) {
            iTtabIndex = 2;
        } else if (tagString.equals("Three")) {
            iTtabIndex = 3;
        } else if (tagString.equals("Four")) {
            //set the alarminfo listview param
            bDisplayAlarm = true;
            CreateTimerTask();
            iTtabIndex = 4;
        } else if (tagString.equals("Fine")) {
            iTtabIndex = 5;
        } else if (tagString.equals("Six")) {
            iTtabIndex = 6;
        }
        if (isSeeking && iTtabIndex != 4) {
            mLayoutDeviceSch.setVisibility(View.VISIBLE);
            mLayoutDevice.setVisibility(View.GONE);
            mLayoutRecord.setVisibility(View.GONE);
            mLayoutPictrue.setVisibility(View.GONE);
            mLayoutAlarmInfo.setVisibility(View.GONE);
            mLayoutSWOut.setVisibility(View.GONE);
            mLayoutSWIn.setVisibility(View.GONE);
        }
    }

    protected void onDestroy() {
        super.onDestroy();

        CancelTimerTask();
        if (iAlarmIDArray != null)
            for (int i = 0; i < MAX_ALARMINFO_COUNT; i++)
                iAlarmIDArray[i] = 0;

        int ret = LibAPI.RequestLogout();
        Log.v(TAG, "RequestLogout return " + ret);

        ret = LibAPI.DeleteLibInstance();
        Log.v(TAG, "DeleteLibInstance return " + ret);

        System.gc();
        this.finish();
    }
}
