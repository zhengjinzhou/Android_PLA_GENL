package com.topvs.platform;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.ImageButton;


public class BaseActivity extends Activity {
    /** Called when the activity is first created. */
	
	private ImageButton btnHelp;
	private ImageButton btnShare;
	
	@Override
	public void onCreate(Bundle icicle) 
	{
		super.onCreate(icicle);
		//requestWindowFeature(Window.FEATURE_NO_TITLE);
		requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
		setContentView(R.layout.main); 
		getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.titlebar);

		btnHelp = (ImageButton)findViewById(R.id.btnHelp);
		btnHelp.setOnClickListener(listener);
		btnShare = (ImageButton)findViewById(R.id.btnShare);
		btnShare.setOnClickListener(listener);		
	}
	
	 OnClickListener listener = new OnClickListener() 
	    {
		    	public void onClick(View v) 
		    	{
		    		if(v.getId() == R.id.btnHelp)
		    		{ 	  
		    			Intent intent = new Intent(BaseActivity.this, ShowHelpActivity.class); 
		    		
		    			BaseActivity.this.startActivity(intent); 
		    			
		    		}else{
		    			Intent it = new Intent(Intent.ACTION_VIEW);    
		    			it.putExtra("sms_body", "http://www.sztopvs.com/");    
		    			it.setType("vnd.android-dir/mms-sms");    
		    			startActivity(it); 
		    		}
		    	}
	   };
}
