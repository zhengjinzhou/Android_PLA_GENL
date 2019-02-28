package com.topvs.player;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;

import com.topvs.platform.R;
import com.topvs.platform.ShowHelpActivity;

import android.R.color;
import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemLongClickListener;

public class FileListActivity extends ListActivity implements OnItemLongClickListener {
    private ArrayList<String> items = null;   // items：存放显示的名称
    private ArrayList<HashMap<String, Object>> photoslistItem;
    private SimpleAdapter photosListItemAdapter;
    boolean bIsPhotoFile;// 是图片还是录像
    private String strGUID;
    private String strGUName;

    private int selIndex;
    private String selFilePath;
    private ImageButton btnHelp;
    private ImageButton btnShare;
    private TextView LableName;
    private Resources rs;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        setContentView(R.layout.filelist);

        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE, R.layout.titlebar);
        rs = getResources();
        LableName = (TextView) findViewById(R.id.textViewTitleBar);

        btnHelp = (ImageButton) findViewById(R.id.btnHelp);
        btnHelp.setOnClickListener(listener);
        btnShare = (ImageButton) findViewById(R.id.btnShare);
        btnShare.setOnClickListener(listener);

        Bundle extras = getIntent().getExtras();
        bIsPhotoFile = extras.getBoolean("IsPhotos");
        strGUID = extras.getString("GUID");
        strGUName = extras.getString("GUName");
        LableName.setText("" + strGUName);

        String strFileSD = "/sdcard";
        String strFileApp = getApplicationContext().getFilesDir().getAbsolutePath();
        if (bIsPhotoFile) {
            strFileSD += "/tuopu/picture/";
            strFileApp += "/tuopu/picture/";
        } else {
            strFileSD += "/tuopu/record/";
            strFileApp += "/tuopu/record/";
        }
        strFileSD += strGUName;
        strFileApp += strGUName;

        photoslistItem = new ArrayList<HashMap<String, Object>>();
        items = new ArrayList<String>();

        File fileSD = new File(strFileSD);
        if (fileSD.exists()) {
            File[] files = fileSD.listFiles();
            if (files != null) {
                // 将所有文件添加ArrayList中
                for (int i = 0; i < files.length; i++) {
                    if (files[i].isFile()) {
                        HashMap<String, Object> map = new HashMap<String, Object>();
                        if (bIsPhotoFile)
                            map.put("ItemImage", R.drawable.imagefile);// 图像资源的ID
                        else map.put("ItemImage", R.drawable.videofile);// 图像资源的ID
                        map.put("ItemTitle", files[i].getName());
                        items.add(strFileSD + "/" + files[i].getName());
                        photoslistItem.add(map);
                    }
                }
            }
        }

        File fileApp = new File(strFileApp);
        if (fileApp.exists()) {
            File[] files = fileApp.listFiles();
            if (files != null) {
                // 将所有文件添加ArrayList中
                for (int i = 0; i < files.length; i++) {

                    if (files[i].isFile()) {
                        HashMap<String, Object> map = new HashMap<String, Object>();
                        if (bIsPhotoFile)
                            map.put("ItemImage", R.drawable.imagefile);// 图像资源的ID
                        else map.put("ItemImage", R.drawable.videofile);// 图像资源的ID
                        map.put("ItemTitle", files[i].getName());
                        items.add(strFileApp + "/" + files[i].getName());
                        photoslistItem.add(map);
                    }
                }
            }
        }
        if (photoslistItem.size() == 0) {
            if (bIsPhotoFile)
                Toast.makeText(FileListActivity.this, rs.getString(R.string.msg_nophotos), Toast.LENGTH_SHORT).show();
            else
                Toast.makeText(FileListActivity.this, rs.getString(R.string.msg_norecords), Toast.LENGTH_SHORT).show();
        }
        // 生成适配器的Item和动态数组对应的元素
        photosListItemAdapter = new SimpleAdapter(this, photoslistItem, // 数据源
                R.layout.filelistitem, // ListItem的XML实现
                // 动态数组与ImageItem对应的子项
                new String[]{"ItemImage", "ItemTitle"},
                // ImageItem的XML文件里面的一个ImageView,两个TextView ID
                new int[]{R.id.file_Image, R.id.file_Title});

        // 添加并且显示
        setListAdapter(photosListItemAdapter);
        getListView().setOnItemLongClickListener(this);
    }

    OnClickListener listener = new OnClickListener() {
        public void onClick(View v) {
            if (v.getId() == R.id.btnHelp) {
                Intent intent = new Intent(FileListActivity.this, ShowHelpActivity.class);

                FileListActivity.this.startActivity(intent);

            } else {
                Intent it = new Intent(Intent.ACTION_VIEW);
                it.putExtra("sms_body", "下载地址：http://www.sztopvs.com/");
                it.setType("vnd.android-dir/mms-sms");
                startActivity(it);
            }
        }
    };

    protected void onDestroy() {
        super.onDestroy();
    }

    /**
     * 设置ListItem被点击时要做的动作
     */
    @SuppressLint("ResourceAsColor")
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        v.setBackgroundColor(color.holo_blue_light);
        selFilePath = items.get(position);
        if (bIsPhotoFile) {
            Intent intent = new Intent(FileListActivity.this, ImageDisplayActivity.class);
            intent.putExtra("INDEX", position);
            intent.putExtra("LIST", items);
            startActivity(intent);
        } else {
            AlertDialog.Builder builder = new AlertDialog.Builder(FileListActivity.this);
            builder.setMessage(rs.getString(R.string.file_openwith)).setCancelable(false)
                    .setPositiveButton(rs.getString(R.string.file_innerplayer), new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int id) {
                            Intent intent = new Intent(FileListActivity.this, RecordPlayActivity.class);
                            intent.putExtra("FILEPATH", selFilePath);
                            startActivity(intent);
                            dialog.cancel();
                        }
                    }).setNegativeButton(rs.getString(R.string.file_outerplayer), new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int id) {
                    myOpenFile(selFilePath);
                    dialog.cancel();
                }
            });
            builder.show();
        }
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        selIndex = arg2;

        AlertDialog.Builder builder = new AlertDialog.Builder(FileListActivity.this);
        builder.setMessage(rs.getString(R.string.war_delete)).setCancelable(false)
                .setPositiveButton(rs.getString(R.string.war_delete_yes), new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        String filename = items.get(selIndex);
                        File f = new File(filename);
                        if (f.exists()) {
                            f.delete();
                            items.remove(selIndex);
                            photoslistItem.remove(selIndex);
                            photosListItemAdapter.notifyDataSetChanged();
                        }
                    }
                }).setNegativeButton(rs.getString(R.string.war_delete_no), new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                dialog.cancel();
            }
        });

        builder.show();

        return true;
    }

    /**
     * 打开文件
     *
     * @param filePath
     */
    protected void myOpenFile(String filePath) {
        Intent intent = new Intent();
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        // 设置intent的Action属性
        intent.setAction(Intent.ACTION_VIEW);
        // 获取文件file的MIME类型
        String type = MIME.getMIMEType(filePath);
        // 设置intent的data和Type属性.
        intent.setDataAndType(Uri.parse("file://" + filePath), type);
        startActivity(intent);
    }
}
