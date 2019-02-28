package com.topvs.wanve.base;

/**
 * Created by zhou
 * on 2019/2/27.
 */
public interface BaseContract {
    interface BasePresenter<T>{
        void attachView(T view);

        void detachView();
    }

    interface BaseView{
        void showError();

        void complete();
    }
}
