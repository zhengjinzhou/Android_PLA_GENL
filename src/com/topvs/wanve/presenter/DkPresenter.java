package com.topvs.wanve.presenter;

import android.util.Log;

import com.topvs.wanve.api.VpnApi;
import com.topvs.wanve.base.RxPresenter;
import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.bean.UserSNIDBean;
import com.topvs.wanve.contract.DkContract;
import com.topvs.wanve.view.LoginActivity;
import com.topvs.wanve.view.fragment.DkFragment;

import okhttp3.OkHttpClient;
import rx.Observer;
import rx.Subscription;
import rx.android.schedulers.AndroidSchedulers;
import rx.schedulers.Schedulers;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public class DkPresenter extends RxPresenter<DkContract.View> implements DkContract.Presenter<DkContract.View>{

    VpnApi vpnApi;
    DkFragment dkFragment;

    public DkPresenter(DkFragment dkFragment){
        this.dkFragment = dkFragment;
        this.vpnApi = new VpnApi(new OkHttpClient());
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

    @Override
    public void GetAttachment() {
        Subscription subscription = vpnApi.GetAttachment("GetAttachment", getUserSNID())
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Observer<UserSNIDBean>() {
                    @Override
                    public void onCompleted() {
                        mView.complete();
                        Log.d("", "onCompleted: ");
                    }

                    @Override
                    public void onError(Throwable e) {
                        mView.showError();
                        Log.d("", "onError: " + e.getMessage());
                    }

                    @Override
                    public void onNext(UserSNIDBean userSNIDBean) {
                        mView.GetAttachmentSuccess(userSNIDBean);
                    }
                });
        addSubscrebe(subscription);
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
