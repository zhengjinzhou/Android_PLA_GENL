package com.topvs.wanve.presenter;

import android.util.Log;

import com.topvs.wanve.api.VpnApi;
import com.topvs.wanve.base.RxPresenter;
import com.topvs.wanve.bean.GetClockRecordsBean;
import com.topvs.wanve.bean.GetColokAllBean;
import com.topvs.wanve.contract.DkContract;
import com.topvs.wanve.contract.JlContract;
import com.topvs.wanve.view.fragment.DkFragment;
import com.topvs.wanve.view.fragment.JlFragment;

import okhttp3.OkHttpClient;
import rx.Observer;
import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public class JlPresenter extends RxPresenter<JlContract.View> implements JlContract.Presenter<JlContract.View> {

    VpnApi vpnApi;
    JlFragment jlFragment;

    public JlPresenter(JlFragment jlFragment){
        this.jlFragment = jlFragment;
        this.vpnApi = new VpnApi(new OkHttpClient());
    }

    @Override
    public void GetClockRecords(String CRDate) {
        Subscription getClockRecords = vpnApi.GetClockRecords("GetClockRecords", getPN_BH(), getUserSNID(), CRDate)
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Observer<GetClockRecordsBean>() {
                    @Override
                    public void onCompleted() {
                        Log.d("", "onCompleted: ");
                    }

                    @Override
                    public void onError(Throwable e) {
                        Log.d("", "onError: " + e.getMessage());
                    }

                    @Override
                    public void onNext(GetClockRecordsBean getClockRecordsBean) {
                        mView.GetClockRecordsSuccess(getClockRecordsBean);
                    }
                });
        addSubscrebe(getClockRecords);

    }

    @Override
    public void GetAllRecords(String CRDate) {
        Subscription getAllRecords = vpnApi.GetAllRecords("GetAllRecords", getPN_BH(), getUserSNID(), CRDate)
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Observer<GetColokAllBean>() {
                    @Override
                    public void onCompleted() {
                        Log.d("", "onCompleted: ");
                    }

                    @Override
                    public void onError(Throwable e) {
                        Log.d("", "onError: " + e.getMessage());
                    }

                    @Override
                    public void onNext(GetColokAllBean getColokAllBean) {
                        mView.GetAllRecordsSuccess(getColokAllBean);
                    }
                });
        addSubscrebe(getAllRecords);
    }

    @Override
    public String getUserSNID() {
        return mView.setUserSNID();
    }

    @Override
    public String getPN_BH() {
        return mView.setPM_BH();
    }

}
