package com.topvs.wanve.adapter.base;

import android.graphics.Rect;
import android.support.v7.widget.RecyclerView;
import android.view.View;

import java.util.ArrayList;
import java.util.List;

/**
 *
 *  画主页的菜单的线
 *
 * Created by Administrator on 2017/11/1.
 */
public class MainMuenSpacingItemDecoration  extends RecyclerView.ItemDecoration {

    private int space;

    private int rows;

    private int column;

    private List<List<Rect>> array = new ArrayList<>();

    public MainMuenSpacingItemDecoration(int space, int rows, int column) {
        this.space = space;
        this.rows = rows;
        this.column = column;
        initArray(rows,column);
    }

    @Override
    public void getItemOffsets(Rect outRect, View view, RecyclerView parent, RecyclerView.State state) {

        outRect.left = space;
        outRect.top = space;
        outRect.right = space;
        outRect.bottom = space;

        if(parent.getChildLayoutPosition(view)  < 3 ){
            outRect.bottom = space;
            outRect.top = 0;
            outRect.right = space;
            outRect.left =0;
        }else if(parent.getChildLayoutPosition(view)  ==  3){
            outRect.bottom = space;
            outRect.top = 0;
            outRect.right = 0;
            outRect.left =0;
        }else if(parent.getChildLayoutPosition(view)  <  7 && parent.getChildLayoutPosition(view) > 3){
            outRect.bottom = 0;
            outRect.top = 0;
            outRect.right = space;
            outRect.left =0;
        }else{
            outRect.bottom = 0;
            outRect.top = 0;
            outRect.right = 0;
            outRect.left =0;
        }


    }

    private void initArray(int rows,int column){

        for(int i = 0; i < rows; i++){
            List<Rect> entity = new ArrayList<>();
            for(int j = 0;j < column;j++){
                Rect rect = new Rect();
                // 尾行处理
                if(i == rows - 1){
                    if(j == column - 1){
                        rect.bottom = 0;
                        rect.top = 0;
                        rect.right = 0;
                        rect.left =0;
                    }else{
                        rect.bottom = 0;
                        rect.top = 0;
                        rect.right = space;
                        rect.left =0;
                    }
                }else{
                    if(j == column - 1){
                        rect.bottom = space;
                        rect.top = 0;
                        rect.right = 0;
                        rect.left =0;
                    }else{
                        rect.bottom = space;
                        rect.top = 0;
                        rect.right = space;
                        rect.left =0;
                    }
                }
                entity.add(rect);
            }
            array.add(entity);
        }


    }

}
