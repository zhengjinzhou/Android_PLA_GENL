package com.topvs.wanve.contract;

import com.topvs.wanve.base.BaseContract;
import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.bean.UserSNIDBean;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public interface DkContract {
    interface View extends BaseContract.BaseView{
        void GetDistanceSuccess(DistanceBean distanceBean);
        void GetAttachmentSuccess(UserSNIDBean userSNIDBean);
        String setUserSNID();
        String setPM_BH();
    }

    interface Presenter<T> extends BaseContract.BasePresenter<T>{
        void GetAttachment();
        String getUserSNID();
        String getPN_BH();
        void GetDistance(String lat,String lon,String address);
    }
}
