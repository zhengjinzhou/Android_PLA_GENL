/**
 * create by LinZh107
 * copright sztopvs@2015
 * */
package com.topvs.player;

import com.topvs.platform.LibAPI;

import android.util.Log;

public class PZLControlThread
{
	private static String TAG = "PZLControlThread";
	private boolean isActivityExit;
	/*
	 * 球机云台控制
	 * */
	private Thread m_startCTLThread;
	private Thread m_stopCTLThread;
	private boolean isCTLRunning = false;
	private boolean isCTLbussy = false;
	
	private Object m_startCTLLock = new Object();
	private Object m_stopCTLLock = new Object();
	
	private int iCTLSPEED = 4;
	private int iCamIndex = 0;
	private String cmdVal = "";
	
	public PZLControlThread()
	{
		isActivityExit = false;
		
		/************* 云台控制-start线程 **************/
		m_startCTLThread = new Thread(new Runnable() {
			public void run(){
				startCTLThread();
			}
		});
		m_startCTLThread.start();
		

		/************* 云台控制-stop线程 **************/
		m_stopCTLThread = new Thread(new Runnable() {
			public void run(){
				stopCTLThread();
			}
		});
		m_stopCTLThread.start();		
	}
	
	private void startCTLThread()
	{
		while (!isActivityExit) 
		{			
			synchronized (m_startCTLLock) {
				try {
	                m_startCTLLock.wait();
                } catch (InterruptedException e) {
	                e.printStackTrace();
	                Log.e(TAG, "m_startCTLLock.wait() error.");
                }

                if(isActivityExit)
                	break;
                else {
                	isCTLbussy = true;
                	if(0 == LibAPI.DomeControl(iCamIndex, 1, iCTLSPEED, cmdVal))
                	{
    					isCTLRunning = true;
                	}
                	isCTLbussy = false;
                }
			}
		}
		Log.v(TAG, "PZL startCTLThread is exit.");
	}
	
	private void stopCTLThread()
	{
		while (!isActivityExit) 
		{			
			synchronized (m_stopCTLLock) {
				try {
					m_stopCTLLock.wait();
                } catch (InterruptedException e) {
	                e.printStackTrace();
	                Log.e(TAG, "m_stopCTLLock.wait() error.");
                }
				
				//等待“开始”动作结束，才能进行“停止”动作。
				while(isCTLbussy && !isActivityExit)
				{
					try {
	                    Thread.sleep(3);
                    } catch (InterruptedException e) {
	                    // TODO Auto-generated catch block
	                    e.printStackTrace();
                    }
				}
				
				if(isCTLRunning)
					LibAPI.DomeControl(iCamIndex, 0, iCTLSPEED, cmdVal);
				isCTLRunning = false;
			}
		}// end while()
		Log.v(TAG, "PZL stopCTLThread is exit.");
	}
	
	public void startCTL(int cIndex, int CTLSPEED, String cmd)
	{
		iCamIndex = cIndex;	
		cmdVal = cmd.toString();
		iCTLSPEED = CTLSPEED;
		synchronized (m_startCTLLock) {
			m_startCTLLock.notify();	
		}
	}

	public void stopCTL()
	{
		synchronized (m_stopCTLLock) {
			m_stopCTLLock.notify();	
		}
	}
	
	public void destroy()
	{
		isActivityExit = true;
		synchronized (m_startCTLLock) {
			m_startCTLLock.notify();
		}
		synchronized (m_stopCTLLock) {
			m_stopCTLLock.notify();
		}
	}
}
