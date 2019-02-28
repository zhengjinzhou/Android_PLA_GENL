package com.topvs.wanve.adapter.base;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.util.SparseArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

/**
 * 通用的ViewHolder，提供基本的getView 、setText、setImager
 * <p>
 * Created by Administrator on 2017/9/12.
 */
public class ViewHolder extends RecyclerView.ViewHolder {

    private SparseArray<View> views;
    private View convertView;
    private Context context;
    private int position;


    public ViewHolder(Context context, View itemView, ViewGroup parent) {
        super(itemView);
        this.context = context;
        this.convertView = itemView;
        this.views = new SparseArray<>();
    }

    public static ViewHolder get(Context context, ViewGroup parent, int layoutId) {
        View itemView = LayoutInflater.from(context).inflate(layoutId, parent, false);
        ViewHolder holder = new ViewHolder(context, itemView, parent);
        return holder;
    }

    /**
     * 通过viewId获取控件
     *
     * @param viewId
     * @return
     */
    public <T extends View> T getView(int viewId) {
        View view = views.get(viewId);
        if (view == null) {
            view = convertView.findViewById(viewId);
            views.put(viewId, view);
        }
        return (T) view;
    }

    public void updatePosition(int position) {
        this.position = position;
    }

    public ViewHolder setText(int viewId, String text) {
        TextView tv = getView(viewId);
        tv.setText(text);
        return this;
    }

    public ViewHolder setText(int viewId, String text, int color) {
        TextView tv = getView(viewId);
        tv.setText(text);
        tv.setTextColor(color);
        return this;
    }

    public ViewHolder setText(int viewId, String text, int color, int flags) {
        TextView tv = getView(viewId);
        tv.setText(text);
        tv.setTextColor(color);
        tv.getPaint().setFlags(flags);
        return this;
    }

    public ViewHolder setImageResource(int viewId, int resId) {
        ImageView view = getView(viewId);
        view.setImageResource(resId);
        return this;
    }

    public ViewHolder setOnClickListener(int viewId,
                                         View.OnClickListener listener) {
        View view = getView(viewId);
        view.setOnClickListener(listener);
        return this;
    }

}
