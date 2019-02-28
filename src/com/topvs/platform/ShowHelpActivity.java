package com.topvs.platform;

import android.os.Bundle;
import android.webkit.WebView;

public class ShowHelpActivity extends BaseActivity
{
    
    WebView webView;

    @Override
	public void onCreate(Bundle icicle) 
    {
        super.onCreate(icicle);
        setContentView(R.layout.help);

        webView = (WebView) findViewById(R.id.help_contents);
        webView.loadUrl("file:///android_asset/help.html");
    }
}