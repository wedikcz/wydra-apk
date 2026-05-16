package com.apptoolkit.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

public class PrefsManager {
    private static final String PREF_AUTO_BYPASS = "pref_auto_bypass";
    private static final String PREF_ANTI_DEBUG = "pref_anti_debug";
    private static final String PREF_PLAY_INTEGRITY = "pref_play_integrity";
    private static final String PREF_FOREGROUND_SERVICE = "pref_foreground_service";
    private static final String PREF_BOOT_START = "pref_boot_start";
    private static final String PREF_THEME = "pref_theme";
    private static final String PREF_LOG_LEVEL = "pref_log_level";
    private static final String PREF_HOOK_PERSISTENCE = "pref_hook_persistence";
    
    private static SharedPreferences prefs;
    private static SharedPreferences.Editor editor;
    
    public static void init(Context context) {
        prefs = PreferenceManager.getDefaultSharedPreferences(context);
        editor = prefs.edit();
    }
    
    public static boolean isAutoBypass() {
        return prefs.getBoolean(PREF_AUTO_BYPASS, true);
    }
    
    public static void setAutoBypass(boolean value) {
        editor.putBoolean(PREF_AUTO_BYPASS, value).apply();
    }
    
    public static boolean isAntiDebug() {
        return prefs.getBoolean(PREF_ANTI_DEBUG, true);
    }
    
    public static void setAntiDebug(boolean value) {
        editor.putBoolean(PREF_ANTI_DEBUG, value).apply();
    }
    
    public static boolean isPlayIntegritySpoof() {
        return prefs.getBoolean(PREF_PLAY_INTEGRITY, false);
    }
    
    public static void setPlayIntegritySpoof(boolean value) {
        editor.putBoolean(PREF_PLAY_INTEGRITY, value).apply();
    }
    
    public static boolean isForegroundService() {
        return prefs.getBoolean(PREF_FOREGROUND_SERVICE, true);
    }
    
    public static boolean isBootStart() {
        return prefs.getBoolean(PREF_BOOT_START, true);
    }
    
    public static String getLogLevel() {
        return prefs.getString(PREF_LOG_LEVEL, "INFO");
    }
    
    public static boolean isHookPersistence() {
        return prefs.getBoolean(PREF_HOOK_PERSISTENCE, false);
    }
}
