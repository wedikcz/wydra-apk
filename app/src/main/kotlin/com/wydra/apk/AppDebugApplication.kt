package com.wydra.apk

import android.app.Application
import android.util.Log

class AppDebugApplication : Application() {
    companion object {
        private const val TAG = "AppDebugApplication"
        lateinit var instance: AppDebugApplication
    }

    override fun onCreate() {
        super.onCreate()
        instance = this
        Log.d(TAG, "Wydra APK Debug Toolkit initialized")
        Log.d(TAG, "Package: $packageName")
        Log.d(TAG, "Debug mode active")
    }
}
