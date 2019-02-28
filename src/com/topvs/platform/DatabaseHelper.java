package com.topvs.platform;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.util.Log;

public class DatabaseHelper
{
	private static final String TAG = "DatabaseActivity";
	public static final String DATABASE_NAME = "playlist.db";
	private static final String TABLE_NAME = "platconfig";

	SQLiteDatabase db;
	Context context;

	DatabaseHelper(Context _context)
	{
		context = _context;
		db = context.openOrCreateDatabase(DATABASE_NAME, Context.MODE_PRIVATE, null);
		CreateTable();
		Log.d(TAG, "DB_path=" + db.getPath());
	}

	public void CreateTable()
	{
		try
		{
			db.execSQL("CREATE TABLE IF NOT EXISTS " + TABLE_NAME + " (ID INTEGER PRIMARY KEY autoincrement,"
			        + "IP VARCHAR, PORT INTEGER, USER VARCHAR, PWD VARCHAR, INSDCARD INTEGER" + ");");
			Log.d(TAG, "Create Table t_user ok!");
		} catch (Exception e)
		{
			Log.e(TAG, "Create Table t_user err,table exists.");
		}
	}

	public boolean save(String ipAddr, int port, String user, String pwd, int bIncdCard)
	{
		String sql = "";
		try
		{
			sql = "delete from " + TABLE_NAME + " where IP='" + ipAddr + "' and USER='" + user + "'";
			db.execSQL(sql);

			sql = "insert into " + TABLE_NAME + " values(null,'" + ipAddr + "','" + port + "','"
						+ user + "','" + pwd + "','" + bIncdCard + "')";
			db.execSQL(sql);
			Log.d(TAG, "Insert Table t_user ok!");
			return true;

		} catch (Exception e)
		{
			Log.e(TAG, "Insert Table t_user err ,sql: " + sql);
			return false;
		}
	}

	public boolean update(int id, String ipAddr, int port, String user, String pwd, int bincdcard)
	{
		String sql = "";
		try
		{
			sql = "update " + TABLE_NAME + " set IP='" + ipAddr + "',PORT='" + port + "',USER='" + user 
					+ "',PWD='" + pwd + "',INSDCARD='" + bincdcard + "' where ID=" + id;
			db.execSQL(sql);
			Log.d(TAG, "Insert Table t_user ok!");
			return true;

		} catch (Exception e)
		{
			Log.e(TAG, "Update Table  err ,sql: " + sql);
			return false;
		}
	}

	public boolean delete(int id)
	{
		String sql = "";
		try
		{
			sql = "delete from " + TABLE_NAME + " where ID=" + id;
			db.execSQL(sql);
			Log.d(TAG, "Delete  ok!");
			return true;

		} catch (Exception e)
		{
			Log.e(TAG, "Delete Table  err ,sql: " + sql);
			return false;
		}
	}

	public boolean deleteAll()
	{
		String sql = "";
		try
		{
			sql = "delete from " + TABLE_NAME;
			db.execSQL(sql);
			Log.d(TAG, "Delete All  ok");
			return true;

		} catch (Exception e)
		{
			Log.e(TAG, "Delete Table  err ,sql: " + sql);
			return false;
		}
	}

	public Cursor loadAll()
	{
		Cursor cur = db.query(TABLE_NAME, new String[]{"ID", "IP", "PORT", "USER", "PWD", "INSDCARD"},
		        null, null, null, null, null);

		return cur;
	}

	public Cursor queryByID(int id)
	{
		Cursor cur = db.rawQuery("select * from " + TABLE_NAME + " where ID=" + id, null);
		Log.d(TAG, "QueryByID  ok!");
		return cur;
	}

	public void close()
	{
		db.close();
	}
}
