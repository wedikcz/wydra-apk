# Keep NativeBridge JNI methods
-keep class com.apptoolkit.NativeBridge {
    native <methods>;
    *;
}

# Keep all classes in our package
-keep class com.apptoolkit.** { *; }

# Keep JNI methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep Serializable classes
-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private static final java.io.ObjectStreamField[] serialPersistentFields;
    !static !transient <fields>;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

# Remove logging in release
-assumenosideeffects class android.util.Log {
    public static boolean isLoggable(java.lang.String, int);
    public static int v(...);
    public static int d(...);
}

# Keep all native code references
-keep class * { *; }
-dontwarn com.apptoolkit.**
-dontoptimize
-dontobfuscate
