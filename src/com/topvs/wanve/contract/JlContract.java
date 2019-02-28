package com.topvs.wanve.contract;

import com.topvs.wanve.base.BaseContract;
import com.topvs.wanve.bean.GetClockRecordsBean;
import com.topvs.wanve.bean.GetColokAllBean;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public interface JlContract {
    interface View extends BaseContract.BaseView{
        void GetClockRecordsSuccess(GetClockRecordsBean getClockRecordsBean);
        void GetAllRecordsSuccess(GetColokAllBean getColokAllBean);
        String setUserSNID();
        String setPM_BH();
    }

    interface Presenter<T> extends BaseContract.BasePresenter<T>{
        void GetClockRecords(String CRDate);
        void GetAllRecords(String CRDate);
        String getUserSNID();
        String getPN_BH();
    }
}
