package com.wydra.apk

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val textView = findViewById<TextView>(R.id.sample_text)
        textView.text = stringFromJNI()
    }

    /**
     * A native method that is implemented by the 'wydra_native' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'wydra_native' library on application startup.
        init {
            System.loadLibrary("wydra_native")
        }
    }
}
