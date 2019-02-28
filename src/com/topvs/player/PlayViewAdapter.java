package com.topvs.player;

import java.util.ArrayList;
import java.util.HashMap;

import com.topvs.platform.R;
import com.topvs.platform.R.drawable;
import com.topvs.platform.R.id;
import com.topvs.platform.R.layout;

import android.content.Context;
import android.graphics.Color;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

public class PlayViewAdapter extends BaseAdapter
{
	private final String TAG = "PlayViewAdapter";
	private Context mContext = null;
	private LayoutInflater mInflater;

	private ArrayList<HashMap<String, Object>> m_itemArray; // how many view will be show
	private ArrayList<HashMap<String, Object>> m_dataArray; // how many camera will be open
	
	private int i_selectedIndex; // index indication the view,not camera
	private boolean isFullSreen;

	private ArrayList<Object> m_cbArray; // save sfh's callback , equal to m_dataArray.size()
	private int i_validCount; // how many views will be related to camera.

	private int i_Width;
	private int i_Height;

	//save item's imageButton
	private ImageView btnChart[];
	private int i_lastPos;
	public boolean isTalking = false;
	
	//save item's component
	static class ViewHolder
	{
		public RelativeLayout playitem;
		public SurfaceView sfv;
		public TextView title;
		public ImageView recodebtn;
		public boolean isTalking = false;
	}
		
	//constructor
	public PlayViewAdapter(Context c, ArrayList<HashMap<String, Object>> itemArray)
	{
		mContext = c;
		i_selectedIndex = 0;
		i_validCount = 0;
		isFullSreen = false;
		m_cbArray = new ArrayList<Object>();
		m_dataArray = itemArray;
		m_itemArray = new ArrayList<HashMap<String, Object>>();
		mInflater = LayoutInflater.from(c);
		btnChart = new ImageView[m_dataArray.size()];
		i_lastPos = 0;
	}

	//set item count and size, can be [1,4,9,16...] and so on .
	public void setCount(int count, int w, int h)
	{
		m_cbArray.clear();
		m_itemArray.clear();
		if (isFullSreen){
			m_itemArray.add(m_dataArray.get(i_selectedIndex));
			i_selectedIndex = 0;
		}
		else
			for (int i = 0; i < count; i++)
			{
				HashMap<String, Object> itemMap = null;
				if (i < m_dataArray.size())
					itemMap = m_dataArray.get(i);
				else
					itemMap = new HashMap<String, Object>();
				m_itemArray.add(itemMap);
			}
		i_Width = w;
		i_Height = h;
	}
	
	public void setFullScreen(boolean flags, int index)
	{
		i_validCount = 0;
		isFullSreen = flags;
		i_selectedIndex = index;
	}
	
	public void removeSfhcb(int index)
	{
		for (int i = 0; i < m_cbArray.size(); i++)
		{
			PlaySFHCallBack sfhcb = (PlaySFHCallBack) m_cbArray.get(i);
			if (sfhcb != null && sfhcb.mVIIndex == index){
				sfhcb.isExit = true;
			}
		}
	}

	@Override
	public int getCount()
	{
		// TODO Auto-generated method stub
		return m_itemArray.size();
	}

	@Override
	public Object getItem(int position)
	{
		// TODO Auto-generated method stub
		for (int i = 0; i < m_cbArray.size(); i++)
		{
			PlaySFHCallBack sfhcb = (PlaySFHCallBack) m_cbArray.get(i);
			if (sfhcb.mVIIndex == position)
				return sfhcb;
		}
		return null;
	}

	@Override
	public long getItemId(int position)
	{
		// TODO Auto-generated method stub
		return position;
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		// TODO Auto-generated method stub
		ViewHolder holder = null;

		// Create the view add holder。
		if (convertView == null)
		{
			//Log.i(TAG, "new a view to display +++ " + position);
			convertView = mInflater.inflate(R.layout.playitem, null);

			holder = new ViewHolder();
			holder.playitem = (RelativeLayout) convertView.findViewById(R.id.playitem);
			holder.sfv = (SurfaceView) convertView.findViewById(R.id.camera_surfaceview);
			holder.title = (TextView) convertView.findViewById(R.id.camera_name);
			holder.recodebtn = (ImageView) convertView.findViewById(R.id.camera_recod_btn);

			convertView.setTag(holder);
		}
		else{
			holder = (ViewHolder) convertView.getTag();
		}

		// Initialize the view holder
		holder.sfv.setLayoutParams(new RelativeLayout.LayoutParams(i_Width, i_Height));
		holder.recodebtn.setBackgroundResource(R.drawable.microphone_off);

		if(isTalking && position==i_lastPos )
			holder.recodebtn.setBackgroundResource(R.drawable.microphone_on);
		
		holder.title.setText("...");
		if (position == i_selectedIndex)
			holder.playitem.setBackgroundColor(Color.rgb(205, 115, 54));
		else
			holder.playitem.setBackgroundColor(Color.rgb(102, 102, 102));
		if (position < m_dataArray.size())
		{
			HashMap<String, Object> itemMap = m_itemArray.get(position);
			holder.title.setText((String) itemMap.get("ItemTitle"));

			// add callback to the surfaceHolder
			if (position == i_validCount)
			{
				Log.i(TAG, "new a callback to run --- " + position);

				PlaySFHCallBack sfhcb = new PlaySFHCallBack();
				sfhcb.mVIIndex = (Integer) itemMap.get("VIstance");
				sfhcb.mCamIndex = (Integer) itemMap.get("CameraID");
				sfhcb.isStoped = false;
				
				holder.sfv.getHolder().addCallback(sfhcb);
				//holder.recodebtn.setOnClickListener(new MyListener(holder, position));
				btnChart[position] = holder.recodebtn;
				
				m_cbArray.add(sfhcb);
				i_validCount++;
			}
		}

		return convertView;
	}
	
	private class MyListener implements OnClickListener
	{
		private ViewHolder mHolder;
		private int i_position;

		public MyListener(ViewHolder holder, int position)
		{
			mHolder = holder;
			i_position = position;
		}

		@Override
		public void onClick(View v)
		{
			// TODO Auto-generated method stub
			v.getParent();
			if (v.getId() == R.id.camera_recod_btn)
			{
				if (mHolder.isTalking)
					mHolder.recodebtn.setBackgroundResource(R.drawable.microphone_off);
				else
					mHolder.recodebtn.setBackgroundResource(R.drawable.microphone_on);
				mHolder.isTalking = !mHolder.isTalking;
			}
		}
	}

	public void setChartBtnBackgroud(boolean flags, int position)
	{
		Log.i(TAG, "set ChartBtn Backgroud --- " + position);
		if(i_lastPos < btnChart.length)
			btnChart[i_lastPos].setBackgroundResource(R.drawable.microphone_off);
		
		if(position >= btnChart.length)
			return;
				
		isTalking = flags;		
		if(isTalking){
			if(isFullSreen)
				btnChart[0].setBackgroundResource(R.drawable.microphone_on);
			else
				btnChart[position].setBackgroundResource(R.drawable.microphone_on);			
		}
		else{
			if(isFullSreen)
				btnChart[0].setBackgroundResource(R.drawable.microphone_off);	
			else
				btnChart[position].setBackgroundResource(R.drawable.microphone_off);	
		}
		i_lastPos = position;
	}
}
