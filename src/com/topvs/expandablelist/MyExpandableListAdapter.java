/**
 * @author LinZh107
 *
 */
package com.topvs.expandablelist;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.zip.CheckedInputStream;

import com.topvs.expandablelist.MyExpandableListView.HeaderAdapter;
import com.topvs.platform.R;

import android.content.Context;
import android.util.SparseBooleanArray;
import android.util.SparseIntArray;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseExpandableListAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

public class MyExpandableListAdapter extends BaseExpandableListAdapter
	implements HeaderAdapter
{
	private static final String TAG = "ExpandableListAdapter";
	
    private Context mContext;
    private LayoutInflater mInflater;
    
	private ArrayList<HashMap<String, Object>> mGroupArrayData;	// ListView's group adapter members
	private ArrayList<HashMap<String, Object>> mChildArrayData;	// ListView's child adapter members
	
	private MyExpandableListView mListView;
	
	/**
	 * item's checkbox selectedArray
	 * */
	//public HashMap<Integer, Boolean> selectedArray;
	public SparseBooleanArray selectedArray;
	public boolean checkBoxVisible;
	AbsListView.LayoutParams itemLP;
	private int height;	//window's width and height
	
    public class ViewHolder {
    	public ImageView Image;
        public TextView title;    
        public CheckBox cBox;    
    }
	
    // 初始化 设置所有checkbox都为未选择
    public void initSelectedArray() {
	    //selectedArray = new HashMap<Integer, Boolean>();
    	selectedArray = new SparseBooleanArray();
	    for (int i = 0; i < mChildArrayData.size(); i++){
	            selectedArray.put(i, false);
	    }
    }
    
	public MyExpandableListAdapter()
	{
		// TODO Auto-generated constructor stub
	}
	
	public MyExpandableListAdapter(Context context, MyExpandableListView listview, 
			ArrayList<HashMap<String, Object>> m_groupArray, 
			ArrayList<HashMap<String, Object>> m_childArray,
								   AbsListView.LayoutParams lp)
	{
	  	this.mContext = context;
    	this.mGroupArrayData = m_groupArray;
    	this.mChildArrayData = m_childArray;
    	this.mListView = listview;
    	this.checkBoxVisible = true;
    	mInflater = LayoutInflater.from(mContext);
    	initSelectedArray();
    	itemLP = lp;
	}
    
	@Override
	public int getGroupCount()
	{
		return mGroupArrayData.size();
	}

	@Override
	public int getChildrenCount(int groupPosition)
	{
		if(groupPosition >= mGroupArrayData.size())
			return 0;
		HashMap<String, Object> groupItemMap = (HashMap<String, Object>) getGroup(groupPosition);
		Integer cout = (Integer)groupItemMap.get("ChildCnt");
		return cout.intValue();
	}

	@Override
	public Object getGroup(int groupPosition)
	{
		return mGroupArrayData.get(groupPosition);
	}

	@Override
	public Object getChild(int groupPosition, int childPosition)
	{
		int index = (int) getChildId(groupPosition, childPosition);
	    return mChildArrayData.get(index);
	}

	@Override
	public long getGroupId(int groupPosition)
	{
		return groupPosition;
	}

	@Override
	public long getChildId(int groupPosition, int childPosition)
	{
		long offset = 0;
		for(int i = 0; i < groupPosition; i++){
			HashMap<String, Object> groupItemMap = mGroupArrayData.get(i);			
			if (groupItemMap != null) {
				Integer tmp = (Integer)groupItemMap.get("ChildCnt");
				offset += tmp.intValue();
	        }
		}
		return offset + childPosition;
	}

	@Override
	public boolean hasStableIds()
	{
		return true;
	}
	
	@Override
	public View getGroupView(int groupPosition, boolean isExpanded, View convertView, ViewGroup parent)
	{
		View view = null;
		if(convertView != null && convertView.getTag() == null)
			view = convertView;
		else
			view = mInflater.inflate(R.layout.devicelisthead, null);

		view.setLayoutParams(itemLP);
		ImageView imgv = (ImageView)view.findViewById(R.id.group_icon);
		TextView textview = (TextView) view.findViewById(R.id.group_title);
		//CheckBox ckb = (CheckBox)view.findViewById(R.id.group_checkBox);

		if(isExpanded)
			imgv.setImageResource(R.drawable.expand);
		else
			imgv.setImageResource(R.drawable.expand2);

	    @SuppressWarnings("unchecked")
		HashMap<String, Object> groupItem = (HashMap<String, Object>) getGroup(groupPosition);
		String itemTitle = String.valueOf(groupPosition);
	    if (groupItem != null) {
        	itemTitle = (String)groupItem.get("GroupName");
        }
		textview.setText(itemTitle);
        return view;
	}

	@Override
	public View getChildView(int groupPosition, int childPosition, boolean isLastChild, View convertView,
	        ViewGroup parent)
	{		
		View view = null;
		//convertView为null的时候初始化convertView。    
        if (convertView != null && convertView.getTag() != null)
			view = convertView;
        else
        	view = mInflater.inflate(R.layout.devicelistitem, null);
		view.setLayoutParams(itemLP);

        ViewHolder holder = null;  
        holder = (ViewHolder) view.getTag();
        if(holder == null){
        	holder = new ViewHolder();
            view.setTag(holder);
        }
		holder.Image = (ImageView) view.findViewById(R.id.dev_Image);
		holder.title = (TextView) view.findViewById(R.id.dev_Title);    
		holder.cBox = (CheckBox) view.findViewById(R.id.dev_checkBox);
		HashMap<String, Object> childItemMap = null;
    	int index = (int)getChildId(groupPosition, childPosition); 
    	if(index < mChildArrayData.size())
    	{	
    		childItemMap = mChildArrayData.get(index);
	        if (childItemMap != null) {
	        	Integer itemIameg = (Integer) childItemMap.get("ItemImage");
	        	String itemTitle = (String)childItemMap.get("ItemTitle");        	
	        	holder.Image.setBackgroundResource(itemIameg.intValue()); 
	            holder.title.setText(itemTitle);
	        }
	        holder.cBox.setChecked(selectedArray.get(index));
    	}
    	if(!checkBoxVisible)
    		holder.cBox.setVisibility(View.INVISIBLE);
        return view;
	}

	@Override
	public boolean isChildSelectable(int groupPosition, int childPosition)
	{
		// TODO Auto-generated method stub
		return true;
	}

	/**
	 * PinnedHeaderExpandableListView's interface
	 * */
	@Override
	public int getHeaderState(int groupPosition, int childPosition) {
		final int childCount = getChildrenCount(groupPosition);
		if (childPosition == childCount - 1) {
			return PINNED_HEADER_PUSHED_UP;
		} else if (childPosition == -1
				&& !mListView.isGroupExpanded(groupPosition)) {
			return PINNED_HEADER_GONE;
		} else {
			return PINNED_HEADER_VISIBLE;
		}
	}

	@Override
	public void configureHeader(View view, int groupPosition,
			int childPosition, int alpha) {
		view.setLayoutParams(itemLP);
		ImageView imgv = (ImageView)view.findViewById(R.id.group_icon);
		TextView textview = (TextView) view.findViewById(R.id.group_title);

		if(getGroupClickStatus(groupPosition) == 0)
			imgv.setImageResource(R.drawable.expand2);
		else
			imgv.setImageResource(R.drawable.expand);

		@SuppressWarnings("unchecked")
		HashMap<String, Object> groupItem = (HashMap<String, Object>) getGroup(groupPosition);
		String itemTitle = String.valueOf(groupPosition);
		if (groupItem != null) {
			itemTitle = (String)groupItem.get("GroupName");
		}
		textview.setText(itemTitle);
	}
	
	private SparseIntArray groupStatusMap = new SparseIntArray(); 
	
	@Override
	public void setGroupClickStatus(int groupPosition, int status) {
		groupStatusMap.put(groupPosition, status);
	}

	@Override
	public int getGroupClickStatus(int groupPosition) {
		if (groupStatusMap.keyAt(groupPosition)>=0) {
			return groupStatusMap.get(groupPosition);
		} else {
			return 0;
		}
	}
}
