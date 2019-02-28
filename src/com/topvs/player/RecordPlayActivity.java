/*
 * 2014-5-20 LinZh107
 * 1.换新的解码库后播放能力提升导致了回放的速度异常，现已修改，在性能满足的情况下能按帧率播放
 */

package com.topvs.player;

import java.util.Timer;
import java.util.TimerTask;

import com.topvs.platform.BaseActivity;
import com.topvs.platform.LibAPI;
import com.topvs.platform.R;

import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.MediaController;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

public class RecordPlayActivity extends BaseActivity
{
	/** Called when the activity is first created. */
	private final String TAG = "RecordPlayActivity";
	
	private final int INVALID_MP4 = -1;
	private final int RetFalse = 0;
	private final int RetTrue = 1;
	private final int RetEOF = 2;	
	public boolean isExit;
	private boolean isStop = false;	
	
	private int mp4TrackParam;

	private Handler mHandler;
	LayoutParams lp = null;
	// private Thread mAudioThread = null;
	// private int msgCount = 0;
	// 2014-5-13 LinZh107 增加视频缩放比例,以区别横、竖屏时的视频长宽比
	private float scaleXY;
	private float Hscale = 1.5f;
	private float Vscale = 1.25f; // 默认的长宽比例	
	public int mVideoRect[] = {352, 288};
	
	private Thread mVideoThread = null;		// 视频播放线程	
	public Bitmap mBitmap = null;
	private int viindex = 0;	
	
	private int scaleWidth;
	private int scaleHeight;
	private int deviceWidth;
	private int deviceHeight;

	private String filePath = "";
	SurfaceView sfv;
	SurfaceHolder sfh;
	TextView mTextView;
	Button btnStop;
	Button btnExit;	
	
	private Thread mAudioThread = null;
	public int AudioParams[] = {0,0};		//音频帧速和帧长
	private AudioTrack track = null;
	public short[] AudioArray = null;
	final int frame_cnt = 50;
	
	Timer mAudioTimer = null;
	TimerTask mTimerTask = null;
	private final int delay = 20;
	private final int period = 990;
	Resources rs;
	ProgressBar mProBar;
	
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.recordplay);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);		
		rs = getResources();
		
		// TextLayout.setText(path);
		btnStop = (Button) findViewById(R.id.btn_stopplay);
		btnStop.setOnClickListener(listener);
		btnExit = (Button) findViewById(R.id.btn_goback);
		btnExit.setOnClickListener(listener);
		mTextView = (TextView)findViewById(R.id.playinfo2);
		mTextView.setText(filePath);
		mProBar = (ProgressBar) findViewById(R.id.progressBar1);
		
		Bundle extras = getIntent().getExtras();
		filePath = extras.getString("FILEPATH");
		
		/**
		 *  open record file 
		 */
		mp4TrackParam = LibAPI.OpenRecordFile(filePath, viindex);
		if(INVALID_MP4 == mp4TrackParam)
			return ;

		Looper looper = Looper.myLooper();        
        //此处甚至可以不需要设置Looper，因为 Handler默认就使用当前线程的Looper
	   	mHandler = new MessageHandler(looper);
		
		DisplayMetrics dm = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);
		deviceWidth = dm.widthPixels; // 屏幕宽（px，如：480px）
		deviceHeight = dm.heightPixels;

		sfv = (SurfaceView) this.findViewById(R.id.SurfaceViewID2);
		lp = sfv.getLayoutParams();
		lp.width = scaleWidth = deviceWidth;
		lp.height = scaleHeight = (int) (deviceWidth * 1.0 / Vscale); // 默認竖屏，按竖屏比例进行变换，在真正获取视频帧后将改为视频的比例
		sfv.setLayoutParams(lp);

		sfh = sfv.getHolder();
		sfh.setKeepScreenOn(true);
		sfh.addCallback(new MySFHCallBack());// 自动运行surfaceCreated以及surfaceChanged
		
		mAudioThread = new Thread(new Runnable()
		{
			public void run(){
				StartDecode_Audio();
			}
		});
		mAudioThread.start();
		
		CreateAudioTimer();		
	}

	private void CreateAudioTimer()
	{
	   	mAudioTimer = new Timer();
	   	mTimerTask =  new TimerTask(){
    		// TimerTask 是个抽象类,实现的是Runable类
			@Override
			public void run()
			{
				//Log.v(TAG, "Enter the GetRecordAudioFrame thread.");
				//获取PCM 音频数据
				AudioArray = LibAPI.GetRecordAudioFrame(mp4TrackParam, 
						frame_cnt, viindex);
			}
		};
		mAudioTimer.schedule(mTimerTask, delay, period);
	}
	
	private void CancelAudioTimer()
	{
		if (mAudioTimer != null){
			mAudioTimer.cancel();
			mAudioTimer = null;
		}
		if(mTimerTask != null){
			mTimerTask.cancel();
			mTimerTask = null;
		}
	}
	
	OnClickListener listener = new OnClickListener()
	{
		public void onClick(View v)
		{
			if (v.getId() == R.id.btn_stopplay)
			{
				if (isStop){
					btnStop.setText(rs.getString(R.string.play_btn_pause));
					isStop = false;
					CreateAudioTimer();
				}
				else{
					btnStop.setText(rs.getString(R.string.play_btn_play));
					isStop = true;
					CancelAudioTimer();
				}
			}
			else if (v.getId() == R.id.btn_goback)
			{
				isExit = true;
				finish();
			}
		}
	};	

	class MySFHCallBack implements SurfaceHolder.Callback
	{
		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
		{
			// TODO Auto-generated method stub
		}

		@Override
		public void surfaceCreated(SurfaceHolder holder)
		{			
			mVideoThread = new Thread(new Runnable()
			{
				public void run(){
					StartDecode_Video();
				}
			});
			mVideoThread.start();			
		}

		@Override
		public void surfaceDestroyed(SurfaceHolder holder)
		{
			if (mVideoThread != null)
			{
				boolean retry = true;
				isExit = true;
				while (retry)
				{
					try
					{
						if (mAudioThread != null)
							mAudioThread.join(); // 等待线程退出
						retry = false; 
					}
					catch (InterruptedException e)
					{		}
	}	}	}	}
	
	//2014-5-13 LinZh107  增加消息提示
	class MessageHandler extends Handler 
	{		
		public MessageHandler(Looper looper) 
	    {  
	        super(looper);  
	    }  
	    @Override  
		public void handleMessage(Message msg) 
		{
			// TODO Auto-generated method stub
			switch(msg.what)
			{
			case 0://横屏启动时强制恢复竖屏，想全屏看请竖屏启动.
				RecordPlayActivity.this.runOnUiThread(new Runnable()
				{
					@Override
					public void run()
					{
						Toast.makeText(RecordPlayActivity.this,
								"横屏启动时强制恢复竖屏，想全屏看请竖屏启动.", Toast.LENGTH_SHORT).show();
						lp = sfv.getLayoutParams();
						lp.width = scaleWidth;
						lp.height = scaleHeight;
						sfv.setLayoutParams(lp);
					}
				});
				break;
			case 1://已到文件末尾.
				RecordPlayActivity.this.runOnUiThread(new Runnable()
				{
					@Override
					public void run()
					{
						Toast.makeText(RecordPlayActivity.this,"已到文件末尾.", Toast.LENGTH_LONG).show();
						btnStop.setText("结束");
					}
				});
				break;
			}
		}
	}
			
	// 2014-5-15 LinZh107 更换新的解码库和显示方案
	private void StartDecode_Video()
	{
		Log.v(TAG, "Enter the StartDecode_Video thread.");
		try
		{
			Canvas canvas = null;
			long startTime;
			long endTime;
			int nConsume_Time;
			
			int frame_rate = 25;
			int ret = 0;
			
			// 2014-4-25 修复横屏状态下打开问题
			if (getResources().getConfiguration().orientation != 1){
				setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
				scaleXY = Vscale;
				scaleWidth = deviceWidth;
				scaleHeight = (int) (deviceWidth * 1.0 / scaleXY);
				mHandler.sendMessage(mHandler.obtainMessage(0));//横屏启动时强制恢复竖屏，想全屏看请竖屏启动.
			}
			
			while (!isExit) // 2014-5-12 LinZh107 解决异常时无法退出的bug
			{
				if (isStop){
					Thread.sleep(500);
					continue;
				}
				
				//remember current time by millisecond
				startTime = System.currentTimeMillis();
				
				// 107.视频分辨率小于屏幕，底层SWS_SCALE不进行bitmap尺寸变换
				if (mVideoRect[0] < scaleWidth){
					mBitmap = Bitmap.createBitmap(mVideoRect[0], mVideoRect[1], Bitmap.Config.RGB_565);
				}
				// 107.视频分辨率 >= 屏幕，底层 SWS_SCALE 进行bitmap缩小变换
				else{
					mVideoRect[0] = scaleWidth;
					mVideoRect[1] = scaleHeight;
					mBitmap = Bitmap.createBitmap(mVideoRect[0], mVideoRect[1], Bitmap.Config.RGB_565);
				}
				
				// 底层创建的位图没有进行缩放
				ret = LibAPI.GetRecordVideoFrame(mp4TrackParam, mVideoRect, mBitmap, viindex); 
				
				if (RetTrue == ret)
				{
					Rect rect = new Rect(0, 0, scaleWidth, scaleHeight);					
					canvas = sfh.lockCanvas(rect);
					
					// 107.java层进行bitmap尺寸变换
					canvas.drawBitmap(mBitmap, null, rect, null);
					// 解锁画布，提交画好的图像
					sfh.unlockCanvasAndPost(canvas);
					
					//get the end up time by millisecond
					endTime = System.currentTimeMillis();
					
					nConsume_Time = (int)( (1000.0/frame_rate) - (endTime - startTime) - 5);
					if(nConsume_Time > 0){
						//Log.d("TAG", "nConsume_Time = "+String.valueOf(nConsume_Time));
						Thread.sleep(nConsume_Time);
						nConsume_Time = 0;
					}
				}
				else if(RetEOF == ret)
				{
					mHandler.sendMessage(mHandler.obtainMessage(1)); //到达文件尾部了
					isExit = true;
					Thread.sleep(10);
				}
				else if(RetFalse == ret){
					Thread.sleep(10);
				}
			}
		} catch (Exception e1)
		{
			e1.printStackTrace();
		}
	}

	private void StartDecode_Audio()
	{
		Log.v(TAG, "Enter the StartDecode_Audio thread.");
		try
		{
	        int SAMPLE_RATE = 8000;
	        int bufferSize = 0;
	        AudioParams[0] = 1;
	        AudioParams[1] = 160;
	        
	        bufferSize = frame_cnt * AudioParams[0]* AudioParams[1] ;
	        
	        track = new AudioTrack(
	        		AudioManager.STREAM_MUSIC,
	        		SAMPLE_RATE,
	        		AudioFormat.CHANNEL_OUT_MONO,
	        		AudioFormat.ENCODING_PCM_16BIT,
	                bufferSize,
	                AudioTrack.MODE_STREAM);
	        track.play();
	        	        
	        Thread.sleep(20);
	        
	        while(!isExit)
			{
	        	if (isStop){
					Thread.sleep(500);
					continue;
				}
	        	
				if(AudioArray != null  && AudioArray.length == bufferSize)
				{
					//Log.d("TAG","AudioArray.length=" + AudioArray.length);
					track.write(AudioArray, 0, AudioArray.length );
				}
				else Thread.sleep(10);
			}
		} catch (Exception e1)
		{		
			e1.printStackTrace();
		} 
	}
	
	public void onConfigurationChanged(Configuration _newConfig)
	{
		super.onConfigurationChanged(_newConfig);
		if (_newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE)
		{			
			lp = sfv.getLayoutParams();
			scaleXY = Hscale;
			scaleWidth =  deviceHeight;
			scaleHeight = deviceWidth;				
			lp.width = scaleWidth;
			lp.height = scaleHeight;
			sfv.setLayoutParams(lp);
			getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			LinearLayout layout = (LinearLayout) findViewById(R.id.btn_manu);
			layout.setVisibility(View.GONE);
		}
		else
		{			
			lp = sfv.getLayoutParams();
			scaleXY = Vscale;
			scaleWidth = deviceWidth;
			scaleHeight = (int) (deviceWidth * 1.0 / scaleXY);
			lp.width = scaleWidth;
			lp.height = scaleHeight;			
			sfv.setLayoutParams(lp);
			
			getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			LinearLayout layout = (LinearLayout) findViewById(R.id.btn_manu);
			layout.setVisibility(View.VISIBLE);
		}
	}

	protected void onDestroy()
	{
		super.onDestroy();
		isExit = true;		
		if(INVALID_MP4 != mp4TrackParam)
		{
			LibAPI.CloseRecordFile(mp4TrackParam, viindex);
			
			CancelAudioTimer();
			
			if (mBitmap != null){
				mBitmap.recycle();
				mBitmap = null;
			}
			
			if(track != null){
				track.stop();//停止播放
				track.release();//释放底层资源。
				track = null;
			}
		}
		this.finish();
	}
}
