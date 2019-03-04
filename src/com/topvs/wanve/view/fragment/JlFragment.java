package com.topvs.wanve.view.fragment;

import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.OrientationHelper;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;
import android.widget.TextView;

import com.haibin.calendarview.Calendar;
import com.haibin.calendarview.CalendarView;
import com.topvs.platform.R;
import com.topvs.wanve.adapter.base.CommonAdapter;
import com.topvs.wanve.adapter.base.ViewHolder;
import com.topvs.wanve.base.Constant;
import com.topvs.wanve.bean.GetClockRecordsBean;
import com.topvs.wanve.bean.GetColokAllBean;
import com.topvs.wanve.bean.goClockInBean;
import com.topvs.wanve.contract.JlContract;
import com.topvs.wanve.presenter.JlPresenter;
import com.topvs.wanve.util.DateUtil;
import com.topvs.wanve.util.SpUtil;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Created by zhou
 * on 2019/2/26.
 */

public class JlFragment extends Fragment implements JlContract.View {

    private static final String TAG = "JlFragment";
    private CalendarView calendarView;
    private TextView tvDate;

    private JlPresenter presenter = new JlPresenter(this);
    private RecyclerView recyclerView;
    private CommonAdapter<GetClockRecordsBean.RecordsBean> adapter;
    private TextView tvName;
    private TextView tvSum;
    goClockInBean clockInBean = (goClockInBean) SpUtil.getObject(getContext(), Constant.goClockIn, goClockInBean.class);
    private TextView tvNum;

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        presenter.attachView(this);
        initView(view);

        initJl(view);
    }

    private void initJl(View view) {
        Date date = new Date();
        SimpleDateFormat dateFm = new SimpleDateFormat("EEEE");
        tvDate.setText(DateUtil.lineDate(date)+" "+dateFm.format(date));

        tvName.setText(clockInBean.getUserName());
        tvNum.setText("班次："+clockInBean.getStartWorkTime()+"-"+clockInBean.getEndWorkTime());

        GetAllRecords();//月，获取当月的打卡数量
        getDayRecords();//日，获取当日打卡数量

        calendarView.setOnCalendarSelectListener(new CalendarView.OnCalendarSelectListener() {
            @Override
            public void onCalendarOutOfRange(Calendar calendar) {
                //这是选择控件中某一日的方法
                Log.d(TAG, "onCalendarOutOfRange: " + calendar);

            }

            @Override
            public void onCalendarSelect(Calendar calendar, boolean isClick) {
                Log.d(TAG, isClick+"onCalendarSelect: " + calendar);
                String week = "";
                if (calendar.getWeek()==0){
                    week = " 星期日";
                }
                if (calendar.getWeek()==1){
                    week = " 星期一";
                }
                if (calendar.getWeek()==2){
                    week = " 星期二";
                }
                if (calendar.getWeek()==3){
                    week = " 星期三";
                }
                if (calendar.getWeek()==4){
                    week = " 星期四";
                }
                if (calendar.getWeek()==5){
                    week = " 星期五";
                }
                if (calendar.getWeek()==6){
                    week = " 星期六";
                }
                String crdate = calendar+"";

                tvDate.setText(crdate.substring(0,4)+"-"+crdate.substring(4,6)+"-"+crdate.substring(6,8)+week);
                if (!isClick){

                    presenter.GetAllRecords(crdate.substring(0, 6));
                }
                presenter.GetClockRecords(calendar + "");
            }
        });

    }
    /**
     * 日，获取当日打卡数量
     */
    private void getDayRecords() {
        String today = DateUtil.lineDDate(new Date());
        Log.d(TAG, "getDayRecords: "+today);
        presenter.GetClockRecords(today);
    }

    /**
     * 月，获取当月的打卡数量
     */
    private void GetAllRecords() {
        String YM = DateUtil.lineYDate(new Date());
        Log.d(TAG, "GetAllRecords: "+YM);
        presenter.GetAllRecords(YM);
    }

    private void initView(View view) {
        calendarView = view.findViewById(R.id.calendarView);
        tvDate = view.findViewById(R.id.tvDate);
        recyclerView = view.findViewById(R.id.recyclerView);
        tvName = view.findViewById(R.id.tvName);
        tvSum = view.findViewById(R.id.tvSum);
        tvNum = view.findViewById(R.id.tvNum);

        LinearLayoutManager layoutManager = new LinearLayoutManager(getContext());
        //设置布局管理器
        recyclerView.setLayoutManager(layoutManager);
        //设置为垂直布局，这也是默认的
        layoutManager.setOrientation(OrientationHelper.VERTICAL);
        //设置Adapter
        List<GetClockRecordsBean.RecordsBean> data = new ArrayList<>();
        adapter = new CommonAdapter<GetClockRecordsBean.RecordsBean>(getActivity(),R.layout.daka_recyclerview,data) {
            @Override
            public void convert(ViewHolder holder, GetClockRecordsBean.RecordsBean recordsBean, int position) {
                holder.setText(R.id.tvTime,"打卡时间："+recordsBean.getClockTime());
            }
        };
        recyclerView.setAdapter(adapter);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.jl_fragment, container, false);
        return view;
    }

    @Override
    public void showError() {

    }

    @Override
    public void complete() {

    }


    @Override
    public void GetClockRecordsSuccess(GetClockRecordsBean getClockRecordsBean) {
        if (getClockRecordsBean.getRecords() == null) {
            tvSum.setText("今日打卡次数：");
            return;
        }
        Log.d(TAG, "GetClockRecordsSuccess: " + getClockRecordsBean.toString());
        adapter.clear();
        adapter.add(getClockRecordsBean.getRecords());
        tvSum.setText("今日打卡次数："+(adapter.getItemCount()-1));
        adapter.notifyDataSetChanged();

    }

    @Override
    public void GetAllRecordsSuccess(GetColokAllBean getColokAllBean) {
        if (getColokAllBean==null)return;
        Log.d(TAG, "GetAllRecordsSuccess: "+getColokAllBean.toString());
        addChangeIndex(getColokAllBean);
    }

    /**
     * 增加下标
     * @param bean
     */
    private void addChangeIndex(GetColokAllBean bean) {
        Map<String, Calendar> map = new HashMap<>();
        for (int i=0;i< bean.getRecords().size();i++){
            Log.d(TAG, "addChangeIndex: "+bean.getRecords().get(i).getClockTime());
            String clockTime = bean.getRecords().get(i).getClockTime();
            int year = Integer.valueOf(clockTime.substring(0, 4));
            int month = Integer.valueOf(clockTime.substring(5, 7));
            int day = Integer.valueOf(clockTime.substring(8, 10));
            map.put(getSchemeCalendar(year, month, day, 0xFF40db25, "假").toString(),
                    getSchemeCalendar(year, month, day, 0xFF40db25, "假"));
        }
        //此方法在巨大的数据量上不影响遍历性能，推荐使用
        calendarView.setSchemeDate(map);
    }


    private Calendar getSchemeCalendar(int year, int month, int day, int color, String text) {
        Calendar calendar = new Calendar();
        calendar.setYear(year);
        calendar.setMonth(month);
        calendar.setDay(day);
        calendar.setSchemeColor(color);//如果单独标记颜色、则会使用这个颜色
        calendar.setScheme(text);
        return calendar;
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