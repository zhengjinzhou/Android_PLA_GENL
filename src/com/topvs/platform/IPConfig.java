package com.topvs.platform;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.Toast;

public class IPConfig extends BaseActivity
{
	private final String TAG = "IPConfig";
	EditText ipEdit;
	EditText portEdit;
	EditText userEdit;
	EditText pwdEdit;
//	EditText platformEdit;
	RadioGroup box;
	int isModify;
	int iID;
	int InSDCard;

	@Override
	public void onCreate(Bundle icicle)
	{
		super.onCreate(icicle);
		setContentView(R.layout.ipconfig);

		ipEdit = (EditText) findViewById(R.id.configip);
		portEdit = (EditText) findViewById(R.id.configport);
		userEdit = (EditText) findViewById(R.id.configuser);
		pwdEdit = (EditText) findViewById(R.id.configpwd);
//		platformEdit = (EditText) findViewById(R.id.configplatform);
		box = (RadioGroup) findViewById(R.id.RadioGroup01);

		Bundle extras = getIntent().getExtras();
		isModify = extras.getInt("IsModify");
		if (isModify == 1)
		{
			iID = extras.getInt("ID");
			String strPort = "";
			strPort += extras.getInt("PORT");
//			platformEdit.setText(extras.getString("PLATFORM"));
			ipEdit.setText(extras.getString("IP"));
			portEdit.setText(strPort);
			userEdit.setText(extras.getString("USER"));
			pwdEdit.setText("");
			InSDCard = extras.getInt("INSDCARD");
			if (1 == InSDCard)
				box.check(R.id.RadioButton_SDCard);
			else box.check(R.id.RadioButton_Phone);
		}

		Button btn_save = (Button) findViewById(R.id.button_save);
		btn_save.setOnClickListener(listener);
		Button btn_exit = (Button) findViewById(R.id.button_exit);
		btn_exit.setOnClickListener(listener);

	}
	// 按钮响应函数
	OnClickListener listener = new OnClickListener()
	{
		public void onClick(View v)
		{
			if (v.getId() == R.id.button_save)
			{				
				if (box.getCheckedRadioButtonId() == R.id.RadioButton_SDCard)
				{
					InSDCard = 1;
					String sDStateString = android.os.Environment.getExternalStorageState();
					Log.d(TAG, sDStateString);
					if (!(sDStateString.equals(android.os.Environment.MEDIA_MOUNTED) || sDStateString
					        .equals(android.os.Environment.MEDIA_MOUNTED_READ_ONLY)))
					{
						Toast.makeText(IPConfig.this, "没有找到SD卡。", Toast.LENGTH_LONG).show();
						return;
					}
				}
				else
					InSDCard = 0;

				String portStr = portEdit.getText().toString();

				Boolean result = true;
				for (int i = portStr.length(); --i >= 0;)
				{
					if (!Character.isDigit(portStr.charAt(i)))
					{
						result = false;
						break;
					}
				}

				if (result == false)
				{
					Toast.makeText(IPConfig.this, "端口号输入错误，请输入数字。", Toast.LENGTH_LONG).show();
					return;
				}

				Bundle bundle = new Bundle();
				bundle.putInt("IsModify", isModify);
				if (isModify == 1)
					bundle.putInt("ID", iID);

				// Date: 2013-10-24 Author: yms
				if ((ipEdit.getText() == null || ipEdit.getText().length() <= 0)
		        || (portEdit.getText() == null || portEdit.getText().length() <= 0))
//		        || (platformEdit.getText() == null || platformEdit.getText().length() <= 0))
				{
					Toast.makeText(IPConfig.this, "配置信息不能为空。", Toast.LENGTH_LONG).show();
					return;
				}

				bundle.putString("IP", ipEdit.getText().toString());
				bundle.putString("PORT", portEdit.getText().toString());
				bundle.putString("USER", userEdit.getText().toString());
				bundle.putString("PWD", pwdEdit.getText().toString());
//				bundle.putString("PLATFORM", platformEdit.getText().toString());
				bundle.putInt("INSDCARD", InSDCard);

				Intent mIntent = new Intent();
				mIntent.putExtras(bundle);
				setResult(RESULT_OK, mIntent);
				finish();
			}
			else if (v.getId() == R.id.button_exit)
			{
				finish();
			}

		}

	};

}
