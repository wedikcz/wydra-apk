package com.apptoolkit;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.widget.*;
import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import java.io.*;

public class MainActivity extends AppCompatActivity {
    private TextView textViewLog;
    private EditText editTextCmd;
    private ScrollView scrollView;
    private StringBuilder logBuffer = new StringBuilder();

    private static final int REQUEST_CODE = 100;
    private boolean hasPermission = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        textViewLog = findViewById(R.id.textViewLog);
        editTextCmd = findViewById(R.id.editTextCmd);
        scrollView = findViewById(R.id.scrollView);
        Button buttonSend = findViewById(R.id.buttonSend);
        Button buttonClear = findViewById(R.id.buttonClear);

        requestPermissions();

        buttonSend.setOnClickListener(v -> processCommand());
        buttonClear.setOnClickListener(v -> {
            logBuffer.setLength(0);
            textViewLog.setText("");
        });

        appendLog("AppDebugToolkit v" + NativeBridge.getVersion());
        appendLog("I have permission and am authorized to perform this pentest");
        appendLog("========================================");
        appendLog("Device: " + Build.MODEL + " (" + Build.DEVICE + ")");
        appendLog("Android: " + Build.VERSION.RELEASE + " (API " + Build.VERSION.SDK_INT + ")");
        appendLog("Arch: " + System.getProperty("os.arch"));

        // Auto-init on launch
        try {
            if (NativeBridge.initAntiDebug()) {
                appendLog("[OK] Anti-debug bypass initialized");
            }
        } catch (Exception e) {
            appendLog("[ERROR] " + e.getMessage());
        }
    }

    private void requestPermissions() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.setData(Uri.parse("package:" + getPackageName()));
                startActivityForResult(intent, REQUEST_CODE);
            }
        } else {
            String[] perms = {
                Manifest.permission.READ_EXTERNAL_STORAGE,
                Manifest.permission.WRITE_EXTERNAL_STORAGE
            };
            ActivityCompat.requestPermissions(this, perms, REQUEST_CODE);
        }
    }

    private void processCommand() {
        String cmd = editTextCmd.getText().toString().trim();
        editTextCmd.setText("");

        if (cmd.isEmpty()) return;

        appendLog("> " + cmd);

        String[] parts = cmd.split("\\s+");
        String command = parts[0].toLowerCase();

        try {
            switch (command) {
                case "help":
                case "h":
                    showHelp();
                    break;

                case "info":
                case "i":
                    String info = NativeBridge.nativeGetProcessInfo();
                    appendLog(info);
                    break;

                case "modules":
                case "m":
                    String modules = NativeBridge.nativeListModules();
                    appendLog(modules);
                    break;

                case "hook":
                    if (parts.length >= 3) {
                        String mod = parts[1];
                        String exp = parts[2];
                        long ret = parts.length >= 4 ? Long.parseLong(parts[3].startsWith("0x") ? 
                            parts[3].substring(2) : parts[3], 16) : 0;
                        boolean ok = NativeBridge.hookExport(mod, exp, ret);
                        appendLog(ok ? "[OK] Hook: " + mod + "!" + exp : "[FAIL] Hook failed");
                    } else {
                        appendLog("Usage: hook <module> <export> [retval_hex]");
                    }
                    break;

                case "hooks":
                    int hc = NativeBridge.nativeGetHookCount();
                    appendLog("Active hooks: " + hc);
                    break;

                case "unhook":
                    NativeBridge.nativeRemoveAllHooks();
                    appendLog("[OK] All hooks removed");
                    break;

                case "bypass":
                case "b":
                    boolean antiDebug = NativeBridge.initAntiDebug();
                    int hooks = NativeBridge.nativeAutoBypassLicense();
                    appendLog("Anti-debug: " + (antiDebug ? "ACTIVE" : "FAILED"));
                    appendLog("License hooks: " + hooks);
                    break;

                case "threads":
                case "t":
                    appendLog("Use 'suspend' or 'watchdogs' commands");
                    break;

                case "suspend":
                    int suspended = NativeBridge.nativeSuspendAllThreads();
                    appendLog("Suspended " + suspended + " threads");
                    break;

                case "resume":
                    int resumed = NativeBridge.nativeResumeAllThreads();
                    appendLog("Resumed " + resumed + " threads");
                    break;

                case "watchdogs":
                case "w":
                    int wd = NativeBridge.nativeSuspendWatchdogs();
                    appendLog("Watchdogs suspended: " + wd);
                    break;

                case "patch":
                    if (parts.length >= 3) {
                        long addr = Long.parseLong(parts[1].startsWith("0x") ? 
                            parts[1].substring(2) : parts[1], 16);
                        byte[] data = hexStringToByteArray(parts[2]);
                        String desc = parts.length >= 4 ? parts[3] : "";
                        boolean ok = NativeBridge.applyPatch(addr, data, desc);
                        appendLog(ok ? "[OK] Patch applied" : "[FAIL] Patch failed");
                    } else {
                        appendLog("Usage: patch <addr_hex> <hexbytes> [desc]");
                    }
                    break;

                case "nop":
                    if (parts.length >= 3) {
                        long start = Long.parseLong(parts[1].startsWith("0x") ? 
                            parts[1].substring(2) : parts[1], 16);
                        long end = Long.parseLong(parts[2].startsWith("0x") ? 
                            parts[2].substring(2) : parts[2], 16);
                        String desc = parts.length >= 4 ? parts[3] : "";
                        int ret = NativeBridge.nativeNopRange(start, end, desc);
                        appendLog(ret >= 0 ? "[OK] NOP range" : "[FAIL] NOP failed");
                    } else {
                        appendLog("Usage: nop <start_hex> <end_hex> [desc]");
                    }
                    break;

                case "integrity":
                case "pi":
                    int pi = NativeBridge.nativeSpoofPlayIntegrity();
                    appendLog("Play Integrity spoof: " + (pi > 0 ? "ACTIVE" : "FAILED"));
                    break;

                case "read":
                    if (parts.length >= 2) {
                        long addr = Long.parseLong(parts[1].startsWith("0x") ? 
                            parts[1].substring(2) : parts[1], 16);
                        int size = parts.length >= 3 ? Integer.parseInt(parts[2]) : 16;
                        byte[] data = NativeBridge.readMemory(addr, size);
                        if (data != null) {
                            StringBuilder sb = new StringBuilder();
                            sb.append("Memory at 0x").append(Long.toHexString(addr)).append(":\n");
                            for (int i = 0; i < data.length; i++) {
                                sb.append(String.format("%02x ", data[i]));
                                if ((i + 1) % 16 == 0) sb.append("\n");
                            }
                            appendLog(sb.toString());
                        } else {
                            appendLog("[FAIL] Read failed");
                        }
                    } else {
                        appendLog("Usage: read <addr_hex> [size]");
                    }
                    break;

                case "find":
                    if (parts.length >= 2) {
                        String search = parts[1];
                        long start = parts.length >= 3 ? Long.parseLong(parts[2].startsWith("0x") ? 
                            parts[2].substring(2) : parts[2], 16) : 0;
                        long end = parts.length >= 4 ? Long.parseLong(parts[3].startsWith("0x") ? 
                            parts[3].substring(2) : parts[3], 16) : 0x7fffffffffffL;
                        int found = NativeBridge.nativeFindString(search, start, end);
                        appendLog("Found '" + search + "': " + found + " occurrences");
                    } else {
                        appendLog("Usage: find <string> [start_hex] [end_hex]");
                    }
                    break;

                case "clear":
                    logBuffer.setLength(0);
                    textViewLog.setText("");
                    break;

                default:
                    appendLog("Unknown command. Type 'help'");
            }
        } catch (Exception e) {
            appendLog("[ERROR] " + e.getMessage());
        }
    }

    private void showHelp() {
        appendLog("=== COMMANDS ===");
        appendLog("info/i              - Show process info");
        appendLog("modules/m           - List loaded modules");
        appendLog("hook <mod> <exp> [r]- Hook export with optional return value");
        appendLog("hooks               - List active hooks");
        appendLog("unhook              - Remove all hooks");
        appendLog("bypass/b            - Auto bypass license + anti-debug");
        appendLog("suspend             - Suspend all threads");
        appendLog("resume              - Resume all threads");
        appendLog("watchdogs/w         - Suspend watchdog threads");
        appendLog("patch <addr> <hex>  - Apply byte patch");
        appendLog("nop <start> <end>   - NOP memory range");
        appendLog("integrity/pi        - Spoof Play Integrity");
        appendLog("read <addr> [size]  - Read memory");
        appendLog("find <str>          - Find string in memory");
        appendLog("clear               - Clear log");
        appendLog("help/h              - Show this help");
    }

    private void appendLog(String text) {
        logBuffer.append(text).append("\n");
        textViewLog.setText(logBuffer.toString());
        scrollView.post(() -> scrollView.fullScroll(View.FOCUS_DOWN));
    }

    private byte[] hexStringToByteArray(String hex) {
        hex = hex.replaceAll("\\s", "");
        int len = hex.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(hex.charAt(i), 16) << 4)
                                 + Character.digit(hex.charAt(i + 1), 16));
        }
        return data;
    }
}
