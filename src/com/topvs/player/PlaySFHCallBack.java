/*
 * create by LinZh107 at 2015-4-27 10:05:20 SurfaceHolder CallBack class
 * Compright
 */
package com.topvs.player;

import com.topvs.platform.LibAPI;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Rect;
import android.util.Log;
import android.view.SurfaceHolder;

class PlaySFHCallBack implements SurfaceHolder.Callback
{
	private final String TAG = "PlaySFHCallBack";
	
	int i_Width = 352;
	int i_Height = 288;
	int mVIIndex = -1;
	int mCamIndex = -1; 
	
	// SurfaceHolder parameter
	SurfaceHolder mSurfaceHolder;
	private Thread mSfvThread = null;
	
	// Video decode parameter
	volatile boolean isStoped = false;
	volatile boolean isExit = false;
	
	Bitmap mBitmap = null;
	int i_frameRate = 0;
	int[] VideoParams = {352, 288};

	@Override
	public void surfaceCreated(SurfaceHolder holder)
	{
		//Log.i(TAG, "the surfaceView has Created. " + mVIIndex);
		mSurfaceHolder = holder;
		mSfvThread = new Thread(new Runnable() {
			public void run()
			{
				StartDecode_Video();
			}
		});
		mSfvThread.start();
	}
	
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
	{
		//Log.i(TAG, "the surfaceView has Changed. " + mVIIndex);
		mSurfaceHolder = holder;
		i_Width = width;
		i_Height = height;
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder)
	{
		//Log.e(TAG, "Exit surfaceDestroyed called. " + mVIIndex);
		isExit = true;
		isStoped = false;
		mSfvThread.interrupt();
		if (mBitmap != null) {
			mBitmap.recycle();
			mBitmap = null;
		}
	}

	// 2014-5-15 LinZh107 更换新的解码库和显示方案
	private void StartDecode_Video()
	{
		try {
			int nConsume_Time = 0;
			Canvas canvas = null;
			while (!isExit)
			{
				// 2014-5-10 LinZh107 更换新的显示方案 修改以支持视频格式自适应
				if (!isStoped) 
				{
					long startTime = System.currentTimeMillis();
					i_frameRate = 0; // getVideoFrame return i_frameRate

					if (VideoParams[0] < i_Width) {	// 107.视频分辨率小于屏幕，底层SWS_SCALE不进行bitmap尺寸变换
						mBitmap = Bitmap.createBitmap(VideoParams[0], VideoParams[1], Bitmap.Config.RGB_565);
					}
					else {								// 107.视频分辨率 >= 屏幕，底层 SWS_SCALE 进行bitmap缩小变换
						VideoParams[0] = i_Width;
						VideoParams[1] = i_Height;
						mBitmap = Bitmap.createBitmap(VideoParams[0], VideoParams[1], Bitmap.Config.RGB_565);
					}

					i_frameRate = LibAPI.GetVideoFrame(VideoParams, mBitmap, mVIIndex);// 底层创建的位图没有进行缩放
					if (i_frameRate > 0) {
						Rect rect = new Rect(0, 0, i_Width, i_Height);
						canvas = mSurfaceHolder.lockCanvas(rect);
						canvas.drawBitmap(mBitmap, null, rect, null); 		// 107.java层进行bitmap尺寸变换
						mSurfaceHolder.unlockCanvasAndPost(canvas);			// 解锁画布，提交画好的图像

						nConsume_Time = (int) (1000.0 / i_frameRate - (System.currentTimeMillis() - startTime) - 5);
						// Log.i(TAG, "Video nConsume_Time = " + String.valueOf(nConsume_Time));
						if (nConsume_Time > 0) {
							Thread.sleep(nConsume_Time);
							nConsume_Time = 0;
						}
					}
					else{ 
						//Log.e(TAG, "decode failed and sleep for while.");
						Thread.sleep(10);
					}
				}			
				else {
					//Log.i(TAG, "start decode thread not yet.");
					Thread.sleep(100);
				}//end if (isStoped)
			}//end while (!isExit)
		} catch (Exception e1) {
			Log.v(TAG, "Exit VThread "+ mVIIndex + " " +e1.toString());
		}
	}
}
