package com.wydra.apk

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log
import androidx.core.app.NotificationCompat

class DebugService : Service() {
    companion object {
        private const val TAG = "DebugService"
        private const val NOTIFICATION_ID = 1
    }

    override fun onCreate() {
        super.onCreate()
        Log.d(TAG, "Debug Service created")
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        val notification = NotificationCompat.Builder(this, "debug_channel")
            .setContentTitle("Wydra Debug Service")
            .setContentText("Monitoring active...")
            .setSmallIcon(android.R.drawable.ic_menu_info_details)
            .build()

        startForeground(NOTIFICATION_ID, notification)
        Log.d(TAG, "Foreground service started")

        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? = null

    override fun onDestroy() {
        super.onDestroy()
        Log.d(TAG, "Debug Service destroyed")
    }
}
