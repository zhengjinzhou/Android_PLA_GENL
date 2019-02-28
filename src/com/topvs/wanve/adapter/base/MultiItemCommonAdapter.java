package com.topvs.wanve.adapter.base;

import android.content.Context;
import android.view.ViewGroup;

import com.topvs.platform.R;

import java.util.List;


/**
 *   多类型通用的Adapter
 * Created by junjianliu
 * time: 2017/6/13.
 */

public abstract class MultiItemCommonAdapter<T> extends BaseCommonAdapter<T> {

    protected MultiItemTypeSupport<T> mMultiItemTypeSupport;

    public MultiItemCommonAdapter(Context mContext, List<T> mDatas,
                                  MultiItemTypeSupport<T> mMultiItemTypeSupport) {
        super(mContext, -1, mDatas);
        this.mMultiItemTypeSupport = mMultiItemTypeSupport;
    }

    @Override
    public int getItemViewType(int position)
    {
        if(position <= mDatas.size() - 1){
            return mMultiItemTypeSupport.getItemViewType(position, mDatas.get(position));
        }else{
            return 99;
        }
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType)
    {
        if(viewType == 99) {
            ViewHolder viewHolder = ViewHolder.get(mContext, parent, R.layout.view_foot);
            return viewHolder;
        }else{
            int layoutId = mMultiItemTypeSupport.getLayoutId(viewType);
            ViewHolder holder = ViewHolder.get(mContext, parent, layoutId);
            return holder;
        }
    }

    @Override
    public int getItemCount() {
        return mDatas == null ? 0 : mDatas.size() + 1;
    }

    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        if(position <= mDatas.size() - 1){
            holder.updatePosition(position);
            convert(holder, mDatas.get(position),position);
        }else{
            if(listener != null){
                listener.converFoot(holder);
            }
        }
    }

    public void setOnConverFootListener(CommonAdapter.OnConverFootListener listener){
        this.listener = listener;
    }

    private CommonAdapter.OnConverFootListener listener;

    public interface OnConverFootListener{
        void converFoot(ViewHolder holder);
    }
}
