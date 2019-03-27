package com.topvs.wanve.api;

import com.topvs.wanve.base.Constant;
import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.bean.GetClockRecordsBean;
import com.topvs.wanve.bean.GetColokAllBean;
import com.topvs.wanve.bean.UserSNIDBean;

import okhttp3.OkHttpClient;
import retrofit2.Retrofit;
import retrofit2.adapter.rxjava.RxJavaCallAdapterFactory;
import retrofit2.converter.gson.GsonConverterFactory;
import rx.Observable;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public class VpnApi {

    public static VpnApi bookApi;
    private ApiService service;

    public VpnApi(OkHttpClient okHttpClient) {
        Retrofit retrofit = new Retrofit.Builder()
                .baseUrl(Constant.BASE_URL)
                .addCallAdapterFactory(RxJavaCallAdapterFactory.create()) // 添加Rx适配器
                .addConverterFactory(GsonConverterFactory.create()) // 添加Gson转换器
                .client(okHttpClient)
                .build();
        service = retrofit.create(ApiService.class);
    }


    public Observable<UserSNIDBean> GetAttachment(String action, String UserSNID){
        return service.GetAttachment(action,UserSNID);
    }

    public Observable<DistanceBean> GetDistance(String action,String lat,String lon,String PM_BH, String UserSNID,String Address){
        return  service.GetDistance(action,lat,lon,PM_BH,UserSNID,Address);
    }

    public Observable<GetClockRecordsBean> GetClockRecords(String action,String pm_bh,String userSNID,String CRDate){
        return service.GetClockRecords(action,pm_bh,userSNID,CRDate);
    }

    public Observable<GetColokAllBean> GetAllRecords(String action, String pm_bh, String userSNID, String CRDate){
        return service.GetAllRecords(action,pm_bh,userSNID,CRDate);
    }
}
