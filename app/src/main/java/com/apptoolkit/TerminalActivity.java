package com.apptoolkit;

import android.content.ContentValues;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import androidx.appcompat.app.AppCompatActivity;
import com.apptoolkit.util.RootShell;
import java.io.OutputStream;

public class TerminalActivity extends AppCompatActivity {
    private TextView terminalOutput;
    private EditText commandInput;
    private Switch rootSwitch;
    private ScrollView outputScroll;
    private final StringBuilder history = new StringBuilder();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_terminal);

        terminalOutput = findViewById(R.id.terminalOutput);
        commandInput = findViewById(R.id.commandInput);
        rootSwitch = findViewById(R.id.rootSwitch);
        outputScroll = findViewById(R.id.outputScroll);
        Button runButton = findViewById(R.id.runButton);
        Button clearButton = findViewById(R.id.clearButton);
        Button exportButton = findViewById(R.id.exportButton);

        rootSwitch.setChecked(RootShell.hasSu());
        appendLine("Terminal ready");
        appendLine("su available: " + RootShell.hasSu());

        runButton.setOnClickListener(v -> runCommand());
        clearButton.setOnClickListener(v -> {
            history.setLength(0);
            terminalOutput.setText("");
        });
        exportButton.setOnClickListener(v -> exportLog());
    }

    private void runCommand() {
        String command = commandInput.getText().toString().trim();
        if (command.isEmpty()) return;
        commandInput.setText("");
        boolean root = rootSwitch.isChecked();
        appendLine((root ? "# " : "$ ") + command);

        new Thread(() -> {
            try {
                int code = RootShell.runCommand(root, command, line -> runOnUiThread(() -> appendLine(line)));
                runOnUiThread(() -> appendLine("[exit " + code + "]"));
            } catch (Exception e) {
                runOnUiThread(() -> appendLine("[error] " + e.getMessage()));
            }
        }).start();
    }

    private void exportLog() {
        try {
            String fileName = "appdebug-terminal-" + System.currentTimeMillis() + ".txt";
            ContentValues values = new ContentValues();
            values.put(MediaStore.MediaColumns.DISPLAY_NAME, fileName);
            values.put(MediaStore.MediaColumns.MIME_TYPE, "text/plain");
            values.put(MediaStore.MediaColumns.RELATIVE_PATH, Environment.DIRECTORY_DOCUMENTS);
            Uri uri = getContentResolver().insert(MediaStore.Files.getContentUri("external"), values);
            if (uri == null) {
                Toast.makeText(this, "Export failed", Toast.LENGTH_SHORT).show();
                return;
            }
            OutputStream out = getContentResolver().openOutputStream(uri);
            if (out == null) {
                Toast.makeText(this, "Export failed", Toast.LENGTH_SHORT).show();
                return;
            }
            out.write(history.toString().getBytes());
            out.close();
            Toast.makeText(this, "Exported to Documents/" + fileName, Toast.LENGTH_LONG).show();
        } catch (Exception e) {
            Toast.makeText(this, "Export error: " + e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }

    private void appendLine(String line) {
        history.append(line).append("\n");
        terminalOutput.setText(history.toString());
        outputScroll.post(() -> outputScroll.fullScroll(ScrollView.FOCUS_DOWN));
    }
}
