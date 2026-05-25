package com.apptoolkit.util;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public final class RootShell {
    private RootShell() {}

    public interface LineCallback {
        void onLine(String line);
    }

    public static boolean hasSu() {
        String[] paths = {
            "/system/xbin/su",
            "/system/bin/su",
            "/sbin/su",
            "/su/bin/su"
        };
        for (String path : paths) {
            if (new java.io.File(path).exists()) {
                return true;
            }
        }
        return false;
    }

    public static int runCommand(boolean useRoot, String command, LineCallback callback) throws Exception {
        List<String> cmd = new ArrayList<>();
        if (useRoot && hasSu()) {
            cmd.add("su");
            cmd.add("-c");
            cmd.add(command);
        } else {
            cmd.add("sh");
            cmd.add("-c");
            cmd.add(command);
        }

        ProcessBuilder pb = new ProcessBuilder(cmd);
        pb.redirectErrorStream(true);
        Process p = pb.start();

        try (BufferedReader reader = new BufferedReader(new InputStreamReader(p.getInputStream()))) {
            String line;
            while ((line = reader.readLine()) != null) {
                callback.onLine(line);
            }
        }

        return p.waitFor();
    }
}
