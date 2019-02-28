/**
 * @author LinZh107
 *
 */
package com.topvs.expandablelist;

import android.app.SearchManager;
import android.content.ComponentName;
import android.content.Context;
import android.view.Menu;
import android.view.MenuInflater;
import android.widget.SearchView;

public class SearchableActivity
{
	public boolean onCreateOptionsMenu(Menu menu) 
	{     
		// Inflate the options menu from XML     
		MenuInflater inflater = getMenuInflater();     
//		inflater.inflate(R.menu.options_menu, menu);      
		
		// Get the SearchView and set the searchable configuration     
		SearchManager searchManager = (SearchManager) getSystemService(Context.SEARCH_SERVICE);     
//		SearchView searchView = (SearchView) menu.findItem(R.id.menu_search).getActionView();    
//		
//		// Assumes current activity is the searchable activity     
//		searchView.setSearchableInfo(searchManager.getSearchableInfo(getComponentName()));     
//		searchView.setIconifiedByDefault(false); 
		// Do not iconify the widget; expand it by default      
		return true; 
	}

	private ComponentName getComponentName()
    {
	    // TODO Auto-generated method stub
	    return null;
    }

	private SearchManager getSystemService(String searchService)
    {
	    // TODO Auto-generated method stub
	    return null;
    }

	private MenuInflater getMenuInflater()
    {
	    // TODO Auto-generated method stub
	    return null;
    }
}
