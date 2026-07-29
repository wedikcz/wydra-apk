package com.wydra.apk

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log
import androidx.core.content.ContextCompat

class BootReceiver : BroadcastReceiver() {
    companion object {
        private const val TAG = "BootReceiver"
    }

    override fun onReceive(context: Context?, intent: Intent?) {
        if (intent?.action == Intent.ACTION_BOOT_COMPLETED) {
            Log.d(TAG, "Boot completed - starting debug service")
            val serviceIntent = Intent(context, DebugService::class.java)
            context?.let { ContextCompat.startForegroundService(it, serviceIntent) }
        }
    }
}
