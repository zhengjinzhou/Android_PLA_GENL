package com.topvs.wanve.presenter;

import android.util.Log;

import com.topvs.wanve.api.VpnApi;
import com.topvs.wanve.base.RxPresenter;
import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.contract.DkContract;
import com.topvs.wanve.contract.RecordContract;
import com.topvs.wanve.view.RecorderActivity;
import com.topvs.wanve.view.fragment.DkFragment;

import okhttp3.OkHttpClient;
import rx.Observer;
import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;

/**
 * Created by zhou
 * on 2019/3/5.
 */

public class RecordPresenter extends RxPresenter<RecordContract.View> implements RecordContract.Presenter<RecordContract.View> {

    VpnApi vpnApi;
    RecorderActivity recorderActivity;

    public RecordPresenter(RecorderActivity recorderActivity){
        this.recorderActivity = recorderActivity;
        this.vpnApi = new VpnApi(new OkHttpClient());
    }

    @Override
    public String getUserSNID() {
        return mView.setUserSNID();
    }

    @Override
    public String getPN_BH() {
        return mView.setPM_BH();
    }

    @Override
    public void GetDistance(String lat, String lon,String address) {
        Log.d("", "GetDistance: "+lat+"----"+lon+"---"+getPN_BH()+"---"+getUserSNID());
        Subscription getDistance = vpnApi.GetDistance("GetDistance", lat, lon, getPN_BH(), getUserSNID(),address)
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Observer<DistanceBean>() {
                    @Override
                    public void onCompleted() {
                        Log.d("", "onCompleted: ");
                    }

                    @Override
                    public void onError(Throwable e) {
                        Log.d("", "onError: " + e.getMessage());
                    }

                    @Override
                    public void onNext(DistanceBean distanceBean) {
                        mView.GetDistanceSuccess(distanceBean);
                    }
                });
        addSubscrebe(getDistance);
    }
}
