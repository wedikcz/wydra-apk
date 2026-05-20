# ProGuard configuration for wydra-apk

# Keep MainActivity
-keep class com.wydra.apk.MainActivity {
    public <init>();
    public void stringFromJNI();
}

# Keep JNI methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep AndroidX classes
-keep class androidx.** { *; }
-dontwarn androidx.**

# Keep native methods
-keepclasseswithmembers class * {
    native <methods>;
}

# Remove logging
-assumenosideeffects class android.util.Log {
    public static *** d(...);
    public static *** v(...);
    public static *** i(...);
}
