package com.topvs.wanve.contract;

import com.topvs.wanve.base.BaseContract;
import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.bean.UserSNIDBean;

/**
 * Created by zhou
 * on 2019/3/5.
 */

public interface RecordContract {
    interface View extends BaseContract.BaseView{
        void GetDistanceSuccess(DistanceBean distanceBean);
        String setUserSNID();
        String setPM_BH();
    }

    interface Presenter<T> extends BaseContract.BasePresenter<T>{
        String getUserSNID();
        String getPN_BH();
        void GetDistance(String lat,String lon,String address);
    }

}
