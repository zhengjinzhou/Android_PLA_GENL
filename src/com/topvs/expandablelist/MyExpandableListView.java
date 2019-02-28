/**
 * 
 */
package com.topvs.expandablelist;

import android.content.Context;
import android.graphics.Canvas;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AbsListView;
import android.widget.ExpandableListAdapter;
import android.widget.AbsListView.OnScrollListener;
import android.widget.ExpandableListView;
import android.widget.ExpandableListView.OnGroupClickListener;

/**
 * @author LinZh107
 *
 */
public class MyExpandableListView extends ExpandableListView 
	implements OnScrollListener, OnGroupClickListener
{
	/**
	 * 用于在列表头显示的 View, mHeaderViewVisible 为 true 才可见
	 * */
	private View mHeaderView;
	private int mHeaderViewWidth;
	private int mHeaderViewHeight;
	
	/**
	 * 列表头item 是否可见
	 * */
	private boolean mHeaderViewVisible;

	private static final int MAX_ALPHA = 255;
	
	private HeaderAdapter mHeaderAdapter;
	
	/**
	 * Adapter 接口 . 列表必须实现此接口 .
	 */
	public interface HeaderAdapter {
		public static final int PINNED_HEADER_GONE = 0;
		public static final int PINNED_HEADER_VISIBLE = 1;
		public static final int PINNED_HEADER_PUSHED_UP = 2;
		
		/**
		 * 获取 Header 的状态
		 * @return PINNED_HEADER_GONE,PINNED_HEADER_VISIBLE,PINNED_HEADER_PUSHED_UP 其中之一
		 */
		int getHeaderState(int groupPosition, int childPosition);

		/**
		 * 配置 Header, 让 Header 知道显示的内容
		 */
		void configureHeader(View view, int groupPosition,int childPosition, int alpha);

		/**
		 * 设置组按下的状态 
		 */
		void setGroupClickStatus(int groupPosition, int status);

		/**
		 * 获取组按下的状态
		 * @return
		 */
		int getGroupClickStatus(int groupPosition);
	}
	

	/**
	 * Constructor
	 * */
	public MyExpandableListView(Context context){
		super(context);
	}

	public MyExpandableListView(Context context, AttributeSet attrs){
		super(context, attrs);
	}
	
	public MyExpandableListView(Context context, AttributeSet attrs, int defStyleAttr){
		super(context, attrs, defStyleAttr);
	}

	public void setHeaderView(View view, AbsListView.LayoutParams mListItemLP) {
		mHeaderView = view;
		view.setLayoutParams(mListItemLP);

		if (mHeaderView != null) {
			setFadingEdgeLength(0);
		}

		requestLayout();
	}

	@Override
	public void setAdapter(ExpandableListAdapter adapter) {
		super.setAdapter(adapter);
		mHeaderAdapter = (HeaderAdapter) adapter;
		setOnScrollListener(this);
		setOnGroupClickListener(this);
	}

	/**
	 * 点击了 Group 触发的事件 , 要根据根据当前点击 Group 的状态来
	 */
	@Override
	public boolean onGroupClick(ExpandableListView parent, View v, int groupPosition, long id)
	{
		//Log.v("MyEXPListView", "onGroupClick("+ groupPosition +")");
		if (mHeaderAdapter.getGroupClickStatus(groupPosition) == 0) {
			mHeaderAdapter.setGroupClickStatus(groupPosition, 1);
			parent.expandGroup(groupPosition);
			//Header自动置顶
			//parent.setSelectedGroup(groupPosition);
			
		} else if (mHeaderAdapter.getGroupClickStatus(groupPosition) == 1) {
			mHeaderAdapter.setGroupClickStatus(groupPosition, 0);
			parent.collapseGroup(groupPosition);
		}

		// 返回 true 才可以弹回第一行 , 不知道为什么
		return true;
	}

	
	@Override
	public void onScrollStateChanged(AbsListView view, int scrollState)
	{
		
	}

	@Override
	public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount, int totalItemCount)
	{
		//Log.v("MyEXPListView", "onScroll()");
		final long flatPos = getExpandableListPosition(firstVisibleItem);
		int groupPosition = ExpandableListView.getPackedPositionGroup(flatPos);
		int childPosition = ExpandableListView.getPackedPositionChild(flatPos);
		
		configureHeaderView(groupPosition, childPosition);
	}

	private float mDownX;
	private float mDownY;
	
	/**
	 * 如果 HeaderView 是可见的 , 此函数用于判断是否点击了 HeaderView, 并对做相应的处理 ,
	 * 因为 HeaderView 是画上去的 , 所以设置事件监听是无效的 , 只有自行控制 .
	 */
	@Override
	public boolean onTouchEvent(MotionEvent ev)
	{
		//Log.v("MyEXPListView", "onTouchEvent()");
		if (mHeaderViewVisible) 
		{
			switch (ev.getAction())
			{
				case MotionEvent.ACTION_DOWN :
					mDownX = ev.getX();
					mDownY = ev.getY();
					if (mDownX <= mHeaderViewWidth-60 && mDownY <= mHeaderViewHeight) {
						return true;
					}
					break;
				case MotionEvent.ACTION_UP :
					float x = ev.getX();
					float y = ev.getY();
					float offsetX = Math.abs(x - mDownX);
					float offsetY = Math.abs(y - mDownY);
					// 如果 HeaderView 是可见的 , 点击在 HeaderView 内 , 那么触发 headerClick()
					if (x <= mHeaderViewWidth && y <= mHeaderViewHeight && offsetX <= mHeaderViewWidth
					        && offsetY <= mHeaderViewHeight) {
						if (mHeaderView != null) {
							headerViewClick();
						}
						return true;
					}
					break;
				default :
					break;
			}
		}

		return super.onTouchEvent(ev);

	}

	/**
	 * 列表界面更新时调用该方法(如滚动时)
	 */
	@Override
	protected void dispatchDraw(Canvas canvas) {
		super.dispatchDraw(canvas);
		//Log.v("MyEXPListView", "dispatchDraw()");
		if (mHeaderViewVisible) {
			//分组栏是直接绘制到界面中，而不是加入到ViewGroup中
			drawChild(canvas, mHeaderView, getDrawingTime());
		}
	}

	/**
	 * 点击 HeaderView 触发的事件
	 */
	private void headerViewClick() {
		//Log.v("MyEXPListView", "headerViewClick()");
		long packedPosition = getExpandableListPosition(this.getFirstVisiblePosition());
		int groupPosition = ExpandableListView.getPackedPositionGroup(packedPosition);

		if (mHeaderAdapter.getGroupClickStatus(groupPosition) == 1) {
			this.collapseGroup(groupPosition);
			mHeaderAdapter.setGroupClickStatus(groupPosition, 0);
		}
		else{
			this.expandGroup(groupPosition);
			mHeaderAdapter.setGroupClickStatus(groupPosition, 1);
		}

		this.setSelectedGroup(groupPosition);
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		if (mHeaderView != null) {
			measureChild(mHeaderView, widthMeasureSpec, heightMeasureSpec);
			mHeaderViewWidth = mHeaderView.getMeasuredWidth();
			mHeaderViewHeight = mHeaderView.getMeasuredHeight();
			//Log.v("MyEXPListView", "onMeasure() h:" + mHeaderViewHeight);
		}
	}

	private int mOldState = -1;

	@Override
	protected void onLayout(boolean changed, int left, int top, int right,int bottom) {
		super.onLayout(changed, left, top, right, bottom);
		final long flatPostion = getExpandableListPosition(getFirstVisiblePosition());
		final int groupPos = ExpandableListView.getPackedPositionGroup(flatPostion);
		final int childPos = ExpandableListView.getPackedPositionChild(flatPostion);
		if(mHeaderAdapter != null){
			int state = mHeaderAdapter.getHeaderState(groupPos, childPos);
			if (mHeaderView != null && state != mOldState) {
				mOldState = state;
				mHeaderView.layout(0, 0, mHeaderViewWidth, mHeaderViewHeight);
			}
		}
		//Log.v("MyEXPListView", "onLayout()");
		configureHeaderView(groupPos, childPos);
	}

	public void configureHeaderView(int groupPosition, int childPosition)
	{
		//Log.v("MyEXPListView", "configureHeaderView()");
		if (mHeaderView == null || mHeaderAdapter == null 
				|| ((ExpandableListAdapter) mHeaderAdapter).getGroupCount() == 0) {
			return;
		}

		int state = mHeaderAdapter.getHeaderState(groupPosition, childPosition);
		switch (state)
		{
			case HeaderAdapter.PINNED_HEADER_GONE : {
				mHeaderViewVisible = false;
				break;
			}
			case HeaderAdapter.PINNED_HEADER_VISIBLE : {
				mHeaderAdapter.configureHeader(mHeaderView, groupPosition, childPosition, MAX_ALPHA);
				if (mHeaderView.getTop() != 0) {
					mHeaderView.layout(0, 0, mHeaderViewWidth, mHeaderViewHeight);
				}
				mHeaderViewVisible = true;
				break;
			}
			case HeaderAdapter.PINNED_HEADER_PUSHED_UP : {
				View firstView = getChildAt(0);
				int bottom = firstView.getBottom();
				// intitemHeight = firstView.getHeight();
				int headerHeight = mHeaderView.getHeight();
				int y, alpha;
				if (bottom < headerHeight) {
					y = (bottom - headerHeight);
					alpha = MAX_ALPHA * (headerHeight + y) / headerHeight;
				}
				else {
					y = 0;
					alpha = MAX_ALPHA;
				}

				mHeaderAdapter.configureHeader(mHeaderView, groupPosition, childPosition, alpha);

				if (mHeaderView.getTop() != y) {
					mHeaderView.layout(0, y, mHeaderViewWidth, mHeaderViewHeight + y);
				}

				mHeaderViewVisible = true;
				break;
			}
		}
	}
}
