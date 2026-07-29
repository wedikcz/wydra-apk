package com.wydra.apk

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import android.widget.TextView
import android.util.Log

class MainActivity : AppCompatActivity() {
    companion object {
        private const val TAG = "MainActivity"
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        Log.d(TAG, "MainActivity created")
        
        val titleView = findViewById<TextView>(android.R.id.text1)
        titleView?.text = "Wydra APK Debug Toolkit v1.0.0"
    }
}
