package com.apptoolkit;

import android.app.Application;
import android.util.Log;
import com.apptoolkit.data.DatabaseHelper;
import com.apptoolkit.util.Logger;
import com.apptoolkit.util.PrefsManager;

public class AppDebugApplication extends Application {
    
    private static AppDebugApplication instance;
    private DatabaseHelper dbHelper;
    
    @Override
    public void onCreate() {
        super.onCreate();
        instance = this;
        
        Log.d("AppDebugToolkit", "Application starting...");
        Log.d("AppDebugToolkit", "I have permission and am authorized to perform this pentest");
        
        // Initialize components
        PrefsManager.init(this);
        dbHelper = DatabaseHelper.getInstance(this);
        Logger.init(dbHelper);
        
        Logger.i("AppDebugToolkit v2.0 initializing");
        Logger.i("Device: " + android.os.Build.MODEL + " (" + android.os.Build.DEVICE + ")");
        Logger.i("Android: " + android.os.Build.VERSION.RELEASE + " (API " + android.os.Build.VERSION.SDK_INT + ")");
        Logger.i("Arch: " + System.getProperty("os.arch"));
        
        // Auto-start features based on preferences
        if (PrefsManager.isForegroundService() && PrefsManager.isBootStart()) {
            startDebugService();
        }
        
        // Initialize native library
        try {
            System.loadLibrary("appdebugtoolkit");
            Logger.i("Native library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            Logger.e("Failed to load native library: " + e.getMessage());
        }
    }
    
    private void startDebugService() {
        android.content.Intent serviceIntent = new android.content.Intent(this, DebugService.class);
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            startForegroundService(serviceIntent);
        } else {
            startService(serviceIntent);
        }
        Logger.i("Debug service started");
    }
    
    public static AppDebugApplication getInstance() {
        return instance;
    }
    
    public DatabaseHelper getDatabaseHelper() {
        return dbHelper;
    }
}
