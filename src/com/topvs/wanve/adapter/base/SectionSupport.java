package com.topvs.wanve.adapter.base;

/**
 * Created by Administrator on 2017/9/12.
 */

public interface SectionSupport<T> {
    public int sectionHeaderLayoutId();

    public int sectionTitleTextViewId();

    public String getTitle(T t);
}
