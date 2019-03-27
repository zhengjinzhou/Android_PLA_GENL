package com.topvs.wanve.api;

import com.topvs.wanve.bean.DistanceBean;
import com.topvs.wanve.bean.GetClockRecordsBean;
import com.topvs.wanve.bean.GetColokAllBean;
import com.topvs.wanve.bean.UserSNIDBean;

import retrofit2.http.GET;
import retrofit2.http.Query;
import rx.Observable;

/**
 * Created by zhou
 * on 2019/2/27.
 */

public interface ApiService {

    @GET("/Wan_mpda_pic/Handlers/FaceAttendanceHandler.ashx")
    Observable<UserSNIDBean>
    GetAttachment(@Query("Action") String action, @Query("UserSNID") String UserSNID);

    @GET("/Wan_mpda_pic/Handlers/FaceAttendanceHandler.ashx")
    Observable<DistanceBean>
    GetDistance(@Query("Action") String action, @Query("Lat") String Lat, @Query("Lon") String Lon, @Query("PM_BH") String PM_BH, @Query("UserSNID") String UserSNID,@Query("Address") String Address);

    @GET("/Wan_mpda_pic/Handlers/FaceAttendanceHandler.ashx")
    Observable<GetClockRecordsBean>
    GetClockRecords(@Query("Action") String action, @Query("PM_BH") String PM_BH,@Query("UserSNID") String UserSNID,@Query("CRDate") String CRDate);

    @GET("/Wan_mpda_pic/Handlers/FaceAttendanceHandler.ashx")
    Observable<GetColokAllBean>
    GetAllRecords(@Query("Action") String action, @Query("PM_BH") String PM_BH,@Query("UserSNID") String UserSNID,@Query("CRDate") String CRDate);

}
