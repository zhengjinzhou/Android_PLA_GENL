package com.topvs.player;

import java.util.ArrayList;

import com.topvs.platform.BaseActivity;
import com.topvs.platform.R;
import com.topvs.platform.R.id;
import com.topvs.platform.R.layout;
import com.topvs.platform.R.string;
import com.topvs.player.GestureUtils.Screen;

import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.view.ViewGroup.LayoutParams;
import android.widget.ImageView;
import android.widget.Toast;

public class ImageDisplayActivity extends BaseActivity {
    /** Called when the activity is first created. */
	private ImageView imgview = null;
	private int currentIndex=0;
	private Bitmap bmp  = null;

	private String path="";
	private int scaleVideoWidth = 352;   
    private int scaleVideoHeight = 288;
    private   int VideoWidth = 352;   
    private   int VideoHeight = 288;  
    private int deviceWidth ;   
    private int deviceHeight;
	
    private GestureDetector gestureDetector;
    private Screen screen; 
    private ArrayList<String> pathitems = null;   //items：存放该目录下所有图片的路径
    private int IMG_LEFT = 0;
    private int IMG_RIGHT = 1;
    
    private Resources rs;
    
    @SuppressWarnings("unchecked")
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.imageview);
   
        rs = getResources();
        
        Bundle extras = getIntent().getExtras();
        currentIndex =  extras.getInt("INDEX");
		pathitems =   (ArrayList<String>) getIntent().getSerializableExtra("LIST"); 
		Log.d("PATH",path);		
				
		Display display = getWindowManager().getDefaultDisplay();
    	deviceHeight = display.getHeight();
    	deviceWidth = display.getWidth();    	
    	scaleVideoHeight = (int)(deviceWidth*1.0/VideoWidth*VideoHeight);
    	scaleVideoWidth = deviceWidth;
    	 	    	    
		imgview = (ImageView)findViewById(R.id.displayimg); 
		LayoutParams lp = imgview.getLayoutParams();
    	lp.height = scaleVideoHeight;
		lp.width = scaleVideoWidth; 
		imgview.setLayoutParams(lp);
		bmp = BitmapFactory.decodeFile(pathitems.get(currentIndex));
		imgview.setImageBitmap(bmp);

		gestureDetector = new GestureDetector(this,onGestureListener);
		//得到屏幕的大小
        screen = GestureUtils.getScreenPix(this);	
    }
   
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        return gestureDetector.onTouchEvent(event);
    }
    
    GestureDetector.OnGestureListener onGestureListener = new GestureDetector.SimpleOnGestureListener(){

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX,
                float velocityY) {
            float x = e2.getX() - e1.getX();
            float y = e2.getY() - e1.getY();
            //限制必须得划过屏幕的1/3才能算划过
            float x_limit = screen.widthPixels / 3;
            //float y_limit = screen.heightPixels / 3;
            float x_abs = Math.abs(x);
            float y_abs = Math.abs(y);
            if(x_abs >= y_abs){
                //gesture left or right
                if(x > x_limit || x < -x_limit){
                    if(x>0){
                        //right
                        show(IMG_RIGHT);
                    }else if(x<0){
                        //left
                        show(IMG_LEFT);
                    }
                }
            }
            return true;
        }
        
    };
    
    private void show(int ori){
        if(ori == IMG_RIGHT)
        {
        	if(currentIndex == 0)
        		Toast.makeText(this, rs.getString(R.string.msg_firstfile), Toast.LENGTH_SHORT).show();
        	else{
        		currentIndex -= 1;
        		bmp = BitmapFactory.decodeFile(pathitems.get(currentIndex));
        		imgview.setImageBitmap(bmp);
        	}   	
        }
        else if(ori == IMG_LEFT){
        	if(currentIndex == pathitems.size()-1)
        		Toast.makeText(this, rs.getString(R.string.msg_lastfile), Toast.LENGTH_SHORT).show();
        	else{
        		currentIndex += 1;
        		bmp = BitmapFactory.decodeFile(pathitems.get(currentIndex));
        		imgview.setImageBitmap(bmp);
        	} 
        }
    }
    	
	public void onConfigurationChanged(Configuration _newConfig) {   	   
		super.onConfigurationChanged(_newConfig);   
		if (_newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) 
		{   
			LayoutParams lp = imgview.getLayoutParams();
	    	lp.height = deviceWidth;
			lp.width = deviceHeight;			
			imgview.setLayoutParams(lp); 
			if(bmp != null)
			{
				bmp = BitmapFactory.decodeFile(pathitems.get(currentIndex));
				imgview.setImageBitmap(bmp);
			}
			getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}   
		else {   
			LayoutParams lp = imgview.getLayoutParams();
			scaleVideoHeight = (int)(deviceWidth*1.0/VideoWidth*VideoHeight);
	    	scaleVideoWidth = deviceWidth;
	    	lp.height = scaleVideoHeight;
			lp.width  = scaleVideoWidth;
			imgview.setLayoutParams(lp);
			if(bmp != null)
			{
				bmp = BitmapFactory.decodeFile(pathitems.get(currentIndex));
	    		imgview.setImageBitmap(bmp);
			}
		}   	   
	}  
			
    protected void onDestroy() {  

        super.onDestroy();   
        Log.d("TAG","play onDestroy called");
    }

   
}