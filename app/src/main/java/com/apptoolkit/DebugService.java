package com.apptoolkit;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;
import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;

public class DebugService extends Service {
    
    private static final String CHANNEL_ID = "AppDebugToolkitChannel";
    private static final int NOTIFICATION_ID = 1001;
    
    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
        
        Notification notification = new NotificationCompat.Builder(this, CHANNEL_ID)
            .setContentTitle("AppDebugToolkit Active")
            .setContentText("Instrumentation framework running")
            .setSmallIcon(android.R.drawable.ic_menu_manage)
            .setPriority(NotificationCompat.PRIORITY_MIN)
            .setOngoing(true)
            .build();
        
        startForeground(NOTIFICATION_ID, notification);
        Log.d("AppDebugToolkit", "DebugService started in foreground");
    }
    
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return START_STICKY;
    }
    
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    
    private void createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "AppDebugToolkit Service",
                NotificationManager.IMPORTANCE_LOW
            );
            channel.setDescription("Background instrumentation service");
            
            NotificationManager manager = getSystemService(NotificationManager.class);
            if (manager != null) {
                manager.createNotificationChannel(channel);
            }
        }
    }
    
    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.d("AppDebugToolkit", "DebugService destroyed");
    }
}
