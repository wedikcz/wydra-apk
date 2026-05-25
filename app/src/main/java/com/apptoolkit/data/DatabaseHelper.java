package com.apptoolkit.data;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

public class DatabaseHelper extends SQLiteOpenHelper {
    
    private static final String DATABASE_NAME = "appdebug_toolkit.db";
    private static final int DATABASE_VERSION = 1;
    
    // Tables
    public static final String TABLE_HOOKS = "hooks";
    public static final String TABLE_PATCHES = "patches";
    public static final String TABLE_MODULES = "modules";
    public static final String TABLE_LOGS = "logs";
    public static final String TABLE_CONFIG = "config";
    
    private static DatabaseHelper instance;
    
    private DatabaseHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }
    
    public static synchronized DatabaseHelper getInstance(Context context) {
        if (instance == null) {
            instance = new DatabaseHelper(context.getApplicationContext());
        }
        return instance;
    }
    
    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE " + TABLE_HOOKS + " (" +
            "id INTEGER PRIMARY KEY AUTOINCREMENT," +
            "type TEXT NOT NULL," +
            "module_name TEXT," +
            "function_name TEXT," +
            "address INTEGER NOT NULL," +
            "replace_return INTEGER," +
            "active INTEGER DEFAULT 1," +
            "timestamp INTEGER NOT NULL" +
            ")");
        
        db.execSQL("CREATE TABLE " + TABLE_PATCHES + " (" +
            "id INTEGER PRIMARY KEY AUTOINCREMENT," +
            "address INTEGER NOT NULL," +
            "size INTEGER NOT NULL," +
            "original_data BLOB," +
            "patch_data BLOB," +
            "description TEXT," +
            "applied INTEGER DEFAULT 1," +
            "timestamp INTEGER NOT NULL" +
            ")");
        
        db.execSQL("CREATE TABLE " + TABLE_MODULES + " (" +
            "id INTEGER PRIMARY KEY AUTOINCREMENT," +
            "name TEXT NOT NULL," +
            "base_address INTEGER NOT NULL," +
            "size INTEGER NOT NULL," +
            "path TEXT," +
            "timestamp INTEGER NOT NULL" +
            ")");
        
        db.execSQL("CREATE TABLE " + TABLE_LOGS + " (" +
            "id INTEGER PRIMARY KEY AUTOINCREMENT," +
            "level TEXT NOT NULL," +
            "message TEXT NOT NULL," +
            "timestamp INTEGER NOT NULL" +
            ")");
        
        db.execSQL("CREATE TABLE " + TABLE_CONFIG + " (" +
            "key TEXT PRIMARY KEY," +
            "value TEXT NOT NULL" +
            ")");
        
        // Insert default config
        ContentValues cv = new ContentValues();
        cv.put("key", "auto_bypass");
        cv.put("value", "true");
        db.insert(TABLE_CONFIG, null, cv);
        
        cv.put("key", "anti_debug");
        cv.put("value", "true");
        db.insert(TABLE_CONFIG, null, cv);
        
        cv.put("key", "play_integrity_spoof");
        cv.put("value", "false");
        db.insert(TABLE_CONFIG, null, cv);
    }
    
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("DROP TABLE IF EXISTS " + TABLE_HOOKS);
        db.execSQL("DROP TABLE IF EXISTS " + TABLE_PATCHES);
        db.execSQL("DROP TABLE IF EXISTS " + TABLE_MODULES);
        db.execSQL("DROP TABLE IF EXISTS " + TABLE_LOGS);
        db.execSQL("DROP TABLE IF EXISTS " + TABLE_CONFIG);
        onCreate(db);
    }
    
    // Log operations
    public void addLog(String level, String message) {
        SQLiteDatabase db = getWritableDatabase();
        ContentValues cv = new ContentValues();
        cv.put("level", level);
        cv.put("message", message);
        cv.put("timestamp", System.currentTimeMillis());
        db.insert(TABLE_LOGS, null, cv);
    }
    
    // Config operations
    public String getConfig(String key, String defaultValue) {
        SQLiteDatabase db = getReadableDatabase();
        Cursor c = db.query(TABLE_CONFIG, new String[]{"value"}, 
            "key = ?", new String[]{key}, null, null, null);
        String value = defaultValue;
        if (c.moveToFirst()) {
            value = c.getString(0);
        }
        c.close();
        return value;
    }
    
    public void setConfig(String key, String value) {
        SQLiteDatabase db = getWritableDatabase();
        ContentValues cv = new ContentValues();
        cv.put("key", key);
        cv.put("value", value);
        db.replace(TABLE_CONFIG, null, cv);
    }
    
    // Hook persistence
    public void saveHook(String type, String module, String function, 
                         long address, long returnValue) {
        SQLiteDatabase db = getWritableDatabase();
        ContentValues cv = new ContentValues();
        cv.put("type", type);
        cv.put("module_name", module);
        cv.put("function_name", function);
        cv.put("address", address);
        cv.put("replace_return", returnValue);
        cv.put("timestamp", System.currentTimeMillis());
        db.insert(TABLE_HOOKS, null, cv);
    }
}
