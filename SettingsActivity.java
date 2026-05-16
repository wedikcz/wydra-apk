package com.apptoolkit;

import android.os.Bundle;
import android.widget.*;
import androidx.appcompat.app.AppCompatActivity;
import com.apptoolkit.util.PrefsManager;

public class SettingsActivity extends AppCompatActivity {
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);
        
        Switch switchAutoBypass = findViewById(R.id.switch_auto_bypass);
        Switch switchAntiDebug = findViewById(R.id.switch_anti_debug);
        Switch switchPlayIntegrity = findViewById(R.id.switch_play_integrity);
        Switch switchForeground = findViewById(R.id.switch_foreground);
        Switch switchBootStart = findViewById(R.id.switch_boot_start);
        Switch switchHookPersist = findViewById(R.id.switch_hook_persist);
        Spinner spinnerLogLevel = findViewById(R.id.spinner_log_level);
        Button buttonSave = findViewById(R.id.button_save);
        Button buttonReset = findViewById(R.id.button_reset);
        
        // Load current values
        switchAutoBypass.setChecked(PrefsManager.isAutoBypass());
        switchAntiDebug.setChecked(PrefsManager.isAntiDebug());
        switchPlayIntegrity.setChecked(PrefsManager.isPlayIntegritySpoof());
        switchForeground.setChecked(PrefsManager.isForegroundService());
        switchBootStart.setChecked(PrefsManager.isBootStart());
        switchHookPersist.setChecked(PrefsManager.isHookPersistence());
        
        String[] logLevels = {"VERBOSE", "DEBUG", "INFO", "WARN", "ERROR"};
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this, 
            android.R.layout.simple_spinner_item, logLevels);
        spinnerLogLevel.setAdapter(adapter);
        
        String currentLevel = PrefsManager.getLogLevel();
        for (int i = 0; i < logLevels.length; i++) {
            if (logLevels[i].equals(currentLevel)) {
                spinnerLogLevel.setSelection(i);
                break;
            }
        }
        
        buttonSave.setOnClickListener(v -> {
            PrefsManager.setAutoBypass(switchAutoBypass.isChecked());
            PrefsManager.setAntiDebug(switchAntiDebug.isChecked());
            PrefsManager.setPlayIntegritySpoof(switchPlayIntegrity.isChecked());
            PrefsManager.setForegroundService(switchForeground.isChecked());
            PrefsManager.setBootStart(switchBootStart.isChecked());
            PrefsManager.setHookPersistence(switchHookPersist.isChecked());
            
            // Log level
            // Would need to save to PrefsManager
            
            Toast.makeText(this, "Settings saved", Toast.LENGTH_SHORT).show();
            finish();
        });
        
        buttonReset.setOnClickListener(v -> {
            switchAutoBypass.setChecked(true);
            switchAntiDebug.setChecked(true);
            switchPlayIntegrity.setChecked(false);
            switchForeground.setChecked(true);
            switchBootStart.setChecked(true);
            switchHookPersist.setChecked(false);
            spinnerLogLevel.setSelection(2); // INFO
        });
    }
}
