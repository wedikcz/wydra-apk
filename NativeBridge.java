package com.apptoolkit;

public class NativeBridge {
    static {
        System.loadLibrary("appdebugtoolkit");
    }

    // Core Memory Operations
    public static native String nativeGetVersion();
    public static native String nativeGetProcessInfo();
    public static native byte[] nativeReadMemory(long address, int size);
    public static native boolean nativeWriteMemory(long address, byte[] data);
    public static native String nativeListModules();

    // Hooking
    public static native int nativeHookExport(String module, String export, long replaceReturn);
    public static native int nativeRemoveAllHooks();
    public static native int nativeGetHookCount();

    // Patching
    public static native int nativeApplyPatch(long address, byte[] data, String description);
    public static native int nativeNopRange(long start, long end, String description);
    public static native int nativePatchConditionalJump(long address, boolean alwaysTaken);
    public static native int nativeGetPatchCount();

    // License Bypass
    public static native int nativeAutoBypassLicense();

    // Anti-Debug
    public static native int nativeInitAntiDebug();

    // Thread Control
    public static native int nativeSuspendWatchdogs();
    public static native int nativeSuspendAllThreads();
    public static native int nativeResumeAllThreads();

    // Play Integrity
    public static native int nativeSpoofPlayIntegrity();

    // Scanner
    public static native int nativeFindString(String str, long start, long end);

    // Wrapper metody s ošetřením chyb
    public static String getVersion() {
        try { return nativeGetVersion(); } 
        catch (UnsatisfiedLinkError e) { return "ERROR: Library not loaded"; }
    }

    public static byte[] readMemory(long address, int size) {
        try { return nativeReadMemory(address, size); }
        catch (Exception e) { return null; }
    }

    public static boolean writeMemory(long address, byte[] data) {
        try { return nativeWriteMemory(address, data); }
        catch (Exception e) { return false; }
    }

    public static boolean hookExport(String module, String export, long retval) {
        return nativeHookExport(module, export, retval) >= 0;
    }

    public static boolean applyPatch(long address, byte[] data, String desc) {
        return nativeApplyPatch(address, data, desc) >= 0;
    }

    public static boolean initAntiDebug() {
        return nativeInitAntiDebug() > 0;
    }

    public static int suspendWatchdogs() {
        return nativeSuspendWatchdogs();
    }
}
