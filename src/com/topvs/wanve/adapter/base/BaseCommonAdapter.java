package com.topvs.wanve.adapter.base;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.ViewGroup;

import com.topvs.platform.R;

import java.util.ArrayList;
import java.util.List;


/**

 * 单类型通用的Adapter

 * Created by junjianliu

 * time: 2017/6/13.

 */

public abstract class BaseCommonAdapter<T> extends RecyclerView.Adapter<ViewHolder> {

    protected Context mContext;
    protected int mLayoutId;
    protected List<T> mDatas;
    protected LayoutInflater mInflater;
    protected List<ViewHolder> viewHolders;

    public void add(T t) {
        this.mDatas.add(t);
    }

    public void put(int i,T t) {
        this.mDatas.set(i,t);
    }

    public void set(int i,T t) {
        this.mDatas.set(i,t);
    }

    public void addDatas(List<T> t){
        this.mDatas = t;
        notifyDataSetChanged();
    }

    public void clear() {
        this.mDatas.clear();
        notifyDataSetChanged();
    }

    public void add(List<T> t){
        int count  = t.size();
        for(int i = 0; i < count; i++){
            mDatas.add(t.get(i));
        }

        notifyDataSetChanged();
    }

    public T get(int pos){
        return mDatas.get(pos);
    }

    public List<T> getData(){
        return mDatas;
    }

    public void remove(int pos) {
        this.mDatas.remove(pos);
    }

    public BaseCommonAdapter(Context mContext, int mLayoutId, List<T> mDatas) {
        this.mContext = mContext;
        this.mLayoutId = mLayoutId;
        this.mDatas = mDatas;
        this.mInflater = LayoutInflater.from(mContext);
        this.viewHolders = new ArrayList<>();
    }

    @Override
    public ViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        if (viewType == 0) {
            ViewHolder viewHolder = ViewHolder.get(mContext, parent, mLayoutId);
            viewHolders.add(viewHolder);
            return viewHolder;
        } else {
            ViewHolder viewHolder = ViewHolder.get(mContext, parent, R.layout.view_foot);
            return viewHolder;
        }
    }


    @Override
    public void onBindViewHolder(ViewHolder holder, int position) {
        holder.updatePosition(position);
        convert(holder, mDatas.get(position), position);
    }


    public abstract void convert(ViewHolder holder, T t, int position);

    @Override
    public int getItemCount() {
        return mDatas.size();
    }
}
