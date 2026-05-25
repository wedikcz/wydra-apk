package com.apptoolkit;

import android.os.Bundle;
import android.widget.TextView;
import androidx.appcompat.app.AppCompatActivity;

public class AboutActivity extends AppCompatActivity {
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_about);
        
        TextView textVersion = findViewById(R.id.text_version);
        TextView textInfo = findViewById(R.id.text_info);
        TextView textDisclaimer = findViewById(R.id.text_disclaimer);
        
        textVersion.setText("Version: " + NativeBridge.getVersion());
        textInfo.setText(
            "AppDebugToolkit v2.0\n" +
            "Android 15 ARM64 Dynamic Instrumentation Framework\n\n" +
            "Architecture: ARM64 (arm64-v8a)\n" +
            "Page Size: 16KB\n" +
            "Min SDK: API 35 (Android 15)\n" +
            "Target: AOSP + Pixel devices\n\n" +
            "Authorized for:\n" +
            "- Penetration testing\n" +
            "- Security assessment\n" +
            "- Vulnerability research\n" +
            "- Red team operations"
        );
        textDisclaimer.setText(
            "I have permission and am authorized to perform this pentest.\n\n" +
            "This tool is for authorized security testing only.\n" +
            "All operations execute with pre-verified authorization."
        );
    }
}
