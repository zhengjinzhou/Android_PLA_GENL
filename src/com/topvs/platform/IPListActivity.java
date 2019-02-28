package com.topvs.platform;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

//@SuppressLint("HandlerLeak")
public class IPListActivity extends BaseActivity
{
	private static final String TAG = "ipListActivity";
	public static final int SELECT_ID = Menu.FIRST;
	public static final int MODIFY_ID = Menu.FIRST + 1;
	public static final int DEL_ID = Menu.FIRST + 2;
	private Button btn_add;
	private Button btn_exit;
	private ListView list;
	private Handler mHandler = null;

	static final int IPCONFIG_REQUEST = 0;
	private DatabaseHelper dbHelper = null;
	private ListMoreAdapter listItemAdapter;
	private int[] ids;
	private int selectedItem = -1;
	private int num;
	private String[] Users;
	private String[] ipAddr;
	public int userid;
	
	public enum iplist_data{ID, IP, PORT, USER, PWD, INSDCARD};
	
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.iplist);

		dbHelper = new DatabaseHelper(this);

		list = (ListView) findViewById(R.id.ipListView);

		initDatas();
		listItemAdapter = new ListMoreAdapter(this);
		list.setAdapter(listItemAdapter);
		
		btn_add = (Button) findViewById(R.id.button_add);
		btn_add.setOnClickListener(onListener);
		btn_exit = (Button) findViewById(R.id.button_back);
		btn_exit.setOnClickListener(onListener);

		mHandler = new Handler()
		{
			public void handleMessage(Message msg)
			{
				if (list != null)
				{
					initDatas();
					list.invalidate();
					list.setAdapter(listItemAdapter);
				}
			}
		};
		// 添加点击
		list.setOnItemClickListener(new OnItemClickListener()
		{
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3)
			{
				if (arg2 < 0)
					return;

				Cursor cursor = dbHelper.queryByID(ids[arg2]);
				if (cursor.getCount() <= 0)
					return;
				cursor.moveToFirst();
				String addr = cursor.getString(cursor.getColumnIndex("IP"));
				String user = cursor.getString(cursor.getColumnIndex("USER"));
				String pwd = cursor.getString(cursor.getColumnIndex("PWD"));
//				String platform = cursor.getString(cursor.getColumnIndex("PLATFORM"));
				int port = cursor.getInt(cursor.getColumnIndex("PORT"));
				int InSDCard = cursor.getInt(cursor.getColumnIndex("INSDCARD"));

				Bundle bundle = new Bundle();
				bundle.putString("IP", addr);
				bundle.putInt("PORT", port);
				bundle.putString("USER", user);
				bundle.putString("PWD", pwd);
//				bundle.putString("PLATFORM", platform);
				bundle.putInt("INSDCARD", InSDCard);
				Intent mIntent = new Intent();
				mIntent.putExtras(bundle);
				setResult(RESULT_OK, mIntent);
				finish();
			}
		});
	}

	public void initDatas()
	{
		Cursor cursor = dbHelper.loadAll();
		num = cursor.getCount();
		cursor.moveToFirst();
		if (num > 0)
		{
			ids = new int[num];
			ipAddr = new String[num];
			Users = new String[num];
		}
		for (int i = 0; i < num; i++)
		{
			ipAddr[i] = cursor.getString(iplist_data.IP.ordinal());
			Users[i] = cursor.getString(iplist_data.USER.ordinal());
			ids[i] = cursor.getInt(iplist_data.ID.ordinal());
			cursor.moveToNext();
		}
	}
	
	OnClickListener onListener = new OnClickListener()
	{
		public void onClick(View v)
		{
			if (v.getId() == R.id.button_add)
			{
				Intent in = new Intent(IPListActivity.this, IPConfig.class);
				in.putExtra("IsModify", 0);
				startActivityForResult(in, IPCONFIG_REQUEST);
			}
			else if (v.getId() == R.id.button_back)
			{		
//				finish();    //2014-6-13  LinZh107   修复数据库打开没关闭bug，下移
				dbHelper.close();
				finish();  
			}
			else if (v.getId() == MODIFY_ID)
			{
				if (selectedItem < 0)
					return;
				Intent in = new Intent(IPListActivity.this, IPConfig.class);
				in.putExtra("IsModify", 1);
				Cursor cursor = dbHelper.queryByID(ids[selectedItem]);
				if (cursor.getCount() <= 0)
					return;
				cursor.moveToFirst();
				Log.d(TAG, "queryByID  count " + cursor.getCount());
				in.putExtra("ID", cursor.getInt(cursor.getColumnIndex("ID")));
				in.putExtra("IP", cursor.getString(cursor.getColumnIndex("IP")));
				in.putExtra("PORT", cursor.getInt(cursor.getColumnIndex("PORT")));
				in.putExtra("USER", cursor.getString(cursor.getColumnIndex("USER")));
				in.putExtra("PWD", cursor.getString(cursor.getColumnIndex("PWD")));
				in.putExtra("INSDCARD", cursor.getInt(cursor.getColumnIndex("INSDCARD")));
//				in.putExtra("PLATFORM", cursor.getString(cursor.getColumnIndex("PLATFORM")));

				startActivityForResult(in, IPCONFIG_REQUEST);
			}
			else if (v.getId() == DEL_ID)
			{
				// **********************************************************
				final PopupWindow pop;
				LayoutInflater inflater;
				View layout;
				Button delete_yes;
				Button delete_no;

				inflater = (LayoutInflater) getSystemService(LAYOUT_INFLATER_SERVICE);

				layout = inflater.inflate(R.layout.warn, null);

				pop = new PopupWindow(layout, getWindowManager().getDefaultDisplay().getWidth(), 300);

				pop.showAtLocation(v, Gravity.CENTER, 0, 0);

				pop.showAsDropDown(layout);

				delete_yes = (Button) layout.findViewById(R.id.delete_yes);
				delete_no = (Button) layout.findViewById(R.id.delete_no);

				delete_yes.setOnClickListener(new OnClickListener()
				{
					public void onClick(View v)
					{
						dbHelper.delete(ids[selectedItem]);
						selectedItem = -1;
						mHandler.sendMessage(mHandler.obtainMessage());
						Toast.makeText(IPListActivity.this, "删除成功！", Toast.LENGTH_LONG).show();
						pop.dismiss();
						mHandler.sendMessage(mHandler.obtainMessage());
					}
				});

				delete_no.setOnClickListener(new OnClickListener()
				{
					public void onClick(View v)
					{
						pop.dismiss();
					}
				});
				// ********************************************
			}
		}
	};
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		if (requestCode == IPCONFIG_REQUEST)
		{
			if (resultCode == RESULT_OK)
			{
				String strIP = (String) data.getCharSequenceExtra("IP");
				String strPort = (String) data.getCharSequenceExtra("PORT");
				String strUser = (String) data.getCharSequenceExtra("USER");
				String strPwd = (String) data.getCharSequenceExtra("PWD");
//				String platform = (String) data.getCharSequenceExtra("PLATFORM");
				int InSDCard = data.getIntExtra("INSDCARD", 0);
				if (data.getIntExtra("IsModify", 0) == 1)
				{
					int id = data.getIntExtra("ID", -1);
					dbHelper.update(id, strIP, Integer.parseInt(strPort), strUser, strPwd, InSDCard);
				}
				else
				{
					dbHelper.save(strIP, Integer.parseInt(strPort), strUser, strPwd, InSDCard);
				}
				mHandler.sendMessage(mHandler.obtainMessage());
			}
		}
	}

	public class ListMoreAdapter extends BaseAdapter
	{
		Activity activity;
		LayoutInflater lInflater;
		public ListMoreAdapter(Activity a)
		{
			activity = a;
			lInflater = activity.getLayoutInflater();
		}

		public int getCount()
		{
			// TODO Auto-generated method stub
			return num;
		}

		public Object getItem(int position)
		{
			// TODO Auto-generated method stub
			return null;
		}

		public long getItemId(int position)
		{
			// TODO Auto-generated method stub
			return position;
		}

		public View getView(int position, View convertView, ViewGroup parent)
		{
			// TODO Auto-generated method stub
			LinearLayout layout = new LinearLayout(activity);
			layout.setOrientation(LinearLayout.VERTICAL);

			layout.addView(addTitleView(position));

			if (selectedItem == position)
			{
				layout.addView(addCustomView(position));
			}

			return layout;
		}

		public View addTitleView(final int i)
		{
			RelativeLayout layout = new RelativeLayout(activity);
			layout = (RelativeLayout) lInflater.inflate(R.layout.iplistitem, null);
			TextView tv = (TextView) layout.findViewById(R.id.ip_ItemTitle);
			tv.setText(ipAddr[i]);
			TextView tv2 = (TextView) layout.findViewById(R.id.ip_ItemText);
			tv2.setText(Users[i]);

			Button btn = (Button) layout.findViewById(R.id.btn_expand);
			btn.setOnClickListener(new OnClickListener()
			{
				public void onClick(View v)
				{
					if (selectedItem == i)
					{
						selectedItem = -1;
						listItemAdapter.notifyDataSetChanged();						
					}
					else
					{
						selectedItem = i;
						listItemAdapter.notifyDataSetChanged();
					}
				}
			});
			return layout;
		}

		public View addCustomView(int i)
		{
			LinearLayout layout = new LinearLayout(activity);			
			layout.setOrientation(LinearLayout.HORIZONTAL);
			
			Button btnModify = new Button(activity);
			btnModify.setText(R.string.modify_btn);
			btnModify.setTextSize(14);
			btnModify.setId(MODIFY_ID);
			btnModify.setAlpha((float) 0.5);
			btnModify.setOnClickListener(onListener);			
			layout.addView(btnModify, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT,
			        LinearLayout.LayoutParams.WRAP_CONTENT));

			Button btnDel = new Button(activity);
			btnDel.setText(R.string.del_btn);
			btnDel.setId(DEL_ID);
			btnDel.setAlpha((float) 0.5);
			btnDel.setOnClickListener(onListener);
			btnDel.setTextSize(14);
			layout.addView(btnDel, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT,
			        LinearLayout.LayoutParams.WRAP_CONTENT));
			return layout;

		}
	}

}
