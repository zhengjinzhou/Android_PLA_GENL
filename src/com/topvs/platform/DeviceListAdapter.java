package com.topvs.platform;

/*
 * Create By LinZh107 2015-5-02
 * adapter for devicelist with a multiple checked box.
 * Com.sztopvs
 * 
 * */

import java.util.ArrayList;
import java.util.HashMap;

import android.content.Context;
import android.graphics.Point;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

public class DeviceListAdapter extends BaseAdapter
{
	private static final String TAG = "DeviceListAdapter";
    private Context mContext = null;
    private LayoutInflater mInflater;
    
    private ArrayList<HashMap<String, Object>> mDataArray;	
	public static HashMap<Integer, Boolean> selectedArray;	//item's checkbox selectedArray
	
	AbsListView.LayoutParams itemLP;
	private int height;//window's width and height
	
    public DeviceListAdapter(Context context, ArrayList<HashMap<String, Object>> mDataArray, Point point)
    {
    	this.mContext = context;
    	this.mDataArray = mDataArray;
    	mInflater = LayoutInflater.from(mContext);
    	initSelectedArray();
    	height = point.y;
    	if(height > 900)
    		height /= 20;
    	else
    		height /= 14;
    	itemLP = new AbsListView.LayoutParams(AbsListView.LayoutParams.WRAP_CONTENT, height);
    }
    
    // 初始化 设置所有checkbox都为未选择
    public void initSelectedArray() {
	    selectedArray = new HashMap<Integer, Boolean>();
	    for (int i = 0; i < mDataArray.size(); i++){
	            selectedArray.put(i, false);
	    }
    }
    
	@Override
	public int getCount()
	{
		// TODO Auto-generated method stub
		return mDataArray.size();
	}

	@Override
	public Object getItem(int position)
	{
		// TODO Auto-generated method stub
		return mDataArray.get(position);
	}

	@Override
	public long getItemId(int position)
	{
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		// TODO Auto-generated method stub	
		/*
		ViewHolder holder = null;  
		//convertView为null的时候初始化convertView。    
        if (convertView == null) {
            holder = new ViewHolder();
            //Log.d(TAG, "new a item to display.");         
			convertView = mInflater.inflate(R.layout.devicelistitem, null);
			holder.Image = (ImageView) convertView.findViewById(R.id.dev_Image);
            holder.title = (TextView) convertView.findViewById(R.id.dev_Title);    
            holder.cBox = (CheckBox) convertView.findViewById(R.id.dev_checkBox);
            holder.cBox.setVisibility(View.VISIBLE);
            convertView.setTag(holder);            		
        } else {
            holder = (ViewHolder) convertView.getTag();
        }
        
        if(holder != null)
        {
	        HashMap<String, Object> itemMap = mDataArray.get(position);
	        if (itemMap != null) {
	        	int itemIameg = (Integer) itemMap.get("ItemImage");
	        	String itemTitle = (String)itemMap.get("ItemTitle");        	
	        	holder.Image.setBackgroundResource(itemIameg); 
	            holder.title.setText(itemTitle);
	        }
	        holder.cBox.setChecked(selectedArray.get(position));
        }
        convertView.setLayoutParams(itemLP);
        return convertView;
        */
		
		View view = null;
		//convertView为null的时候初始化convertView。    
        if (convertView == null){
			view = mInflater.inflate(R.layout.devicelistitem, null); 
        	Log.d(TAG, "children's converview is NULL");
        }    		
        else{
        	view = convertView;
        	Log.i(TAG, "children's converview not NULL");
        }
        ViewHolder holder = null;  
        holder = (ViewHolder) view.getTag();
        if(holder == null){
        	holder = new ViewHolder();
            view.setTag(holder);
        }
		holder.Image = (ImageView) view.findViewById(R.id.dev_Image);
		holder.title = (TextView) view.findViewById(R.id.dev_Title);    
		holder.cBox = (CheckBox) view.findViewById(R.id.dev_checkBox);
		//holder.cBox.setVisibility(View.VISIBLE);
		
		HashMap<String, Object> childItemMap = null;
    	if(position < mDataArray.size())
    	{	
    		childItemMap = mDataArray.get(position);
	        if (childItemMap != null) {
	        	Integer itemIameg = (Integer) childItemMap.get("ItemImage");
	        	String itemTitle = (String)childItemMap.get("ItemTitle");        	
	        	holder.Image.setBackgroundResource(itemIameg.intValue()); 
	            holder.title.setText(itemTitle);
	        }
	        holder.cBox.setChecked(selectedArray.get(position));
    	}

        view.setLayoutParams(itemLP);
        return view;
    }
    
    public class ViewHolder {
    	public ImageView Image;
        public TextView title;    
        public CheckBox cBox;    
    }
}
