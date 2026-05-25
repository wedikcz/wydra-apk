package com.apptoolkit.util;

import android.util.Log;
import com.apptoolkit.data.DatabaseHelper;

public class Logger {
    private static final String TAG = "AppDebugToolkit";
    private static DatabaseHelper dbHelper;
    private static int logLevel = Log.INFO;
    
    public static final int LEVEL_VERBOSE = Log.VERBOSE;
    public static final int LEVEL_DEBUG = Log.DEBUG;
    public static final int LEVEL_INFO = Log.INFO;
    public static final int LEVEL_WARN = Log.WARN;
    public static final int LEVEL_ERROR = Log.ERROR;
    
    public static void init(DatabaseHelper helper) {
        dbHelper = helper;
        String level = PrefsManager.getLogLevel();
        switch (level) {
            case "VERBOSE": logLevel = LEVEL_VERBOSE; break;
            case "DEBUG": logLevel = LEVEL_DEBUG; break;
            case "WARN": logLevel = LEVEL_WARN; break;
            case "ERROR": logLevel = LEVEL_ERROR; break;
            default: logLevel = LEVEL_INFO;
        }
    }
    
    public static void v(String message) {
        if (logLevel <= LEVEL_VERBOSE) {
            Log.v(TAG, message);
            if (dbHelper != null) dbHelper.addLog("VERBOSE", message);
        }
    }
    
    public static void d(String message) {
        if (logLevel <= LEVEL_DEBUG) {
            Log.d(TAG, message);
            if (dbHelper != null) dbHelper.addLog("DEBUG", message);
        }
    }
    
    public static void i(String message) {
        if (logLevel <= LEVEL_INFO) {
            Log.i(TAG, message);
            if (dbHelper != null) dbHelper.addLog("INFO", message);
        }
    }
    
    public static void w(String message) {
        if (logLevel <= LEVEL_WARN) {
            Log.w(TAG, message);
            if (dbHelper != null) dbHelper.addLog("WARN", message);
        }
    }
    
    public static void e(String message) {
        if (logLevel <= LEVEL_ERROR) {
            Log.e(TAG, message);
            if (dbHelper != null) dbHelper.addLog("ERROR", message);
        }
    }
    
    public static void e(String message, Throwable tr) {
        if (logLevel <= LEVEL_ERROR) {
            Log.e(TAG, message, tr);
            if (dbHelper != null) dbHelper.addLog("ERROR", message + ": " + tr.getMessage());
        }
    }
}
