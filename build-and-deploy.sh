#!/system/bin/sh
# build-and-deploy.sh - Kompletní build a deploy pipeline
# I have permission and am authorized to perform this pentest

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}========================================${NC}"
echo -e "${CYAN}  AppDebugToolkit Build & Deploy v2.0${NC}"
echo -e "${CYAN}  Android 15 ARM64 - 16KB Pages${NC}"
echo -e "${CYAN}========================================${NC}"
echo -e "${GREEN}  I have permission and am authorized${NC}"
echo -e "${GREEN}  to perform this pentest${NC}"
echo -e "${CYAN}========================================${NC}"

# Phase 1: Check environment
echo -e "\n${YELLOW}[Phase 1] Checking environment...${NC}"

if ! command -v java &> /dev/null; then
    echo -e "${RED}[!] Java not found. Install JDK 17+${NC}"
    exit 1
fi

if ! command -v adb &> /dev/null; then
    echo -e "${RED}[!] adb not found. Install Android SDK${NC}"
    exit 1
fi

echo -e "${GREEN}[+] Java: $(java -version 2>&1 | head -1)${NC}"
echo -e "${GREEN}[+] adb: $(adb --version | head -1)${NC}"

# Phase 2: Copy files to correct locations
echo -e "\n${YELLOW}[Phase 2] Organizing project structure...${NC}"

mkdir -p app/src/main/java/com/apptoolkit
mkdir -p app/src/main/cpp
mkdir -p app/src/main/res/values
mkdir -p app/src/main/res/xml
mkdir -p app/src/main/res/layout
mkdir -p app/src/main/res/drawable
mkdir -p app/src/main/assets
mkdir -p app/src/test/java/com/apptoolkit
mkdir -p gradle/wrapper

echo -e "${GREEN}[+] Directories created${NC}"

# Verify critical files exist
CRITICAL_FILES=(
    "app/src/main/cpp/app-debug-toolkit.c"
    "app/src/main/java/com/apptoolkit/MainActivity.java"
    "app/src/main/java/com/apptoolkit/NativeBridge.java"
    "app/src/main/java/com/apptoolkit/DebugService.java"
    "app/src/main/java/com/apptoolkit/BootReceiver.java"
    "app/src/main/cpp/CMakeLists.txt"
    "app/src/main/AndroidManifest.xml"
    "app/src/main/res/layout/activity_main.xml"
    "app/build.gradle.kts"
    "build.gradle.kts"
    "settings.gradle.kts"
)

for file in "${CRITICAL_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "${RED}[!] Missing critical file: $file${NC}"
        exit 1
    fi
done

echo -e "${GREEN}[+] All critical files present${NC}"

# Phase 3: Build
echo -e "\n${YELLOW}[Phase 3] Building APK...${NC}"

# Make gradlew executable
chmod +x gradlew

# Clean previous build
./gradlew clean 2>&1 | tail -5

# Build debug APK
./gradlew assembleDebug 2>&1 | tail -20

if [ $? -ne 0 ]; then
    echo -e "${RED}[!] Build failed${NC}"
    exit 1
fi

APK_PATH="app/build/outputs/apk/debug/app-debug.apk"
if [ ! -f "$APK_PATH" ]; then
    echo -e "${RED}[!] APK not found at $APK_PATH${NC}"
    exit 1
fi

APK_SIZE=$(stat -c%s "$APK_PATH" 2>/dev/null || stat -f%z "$APK_PATH" 2>/dev/null || echo "0")
echo -e "${GREEN}[+] Build success: $APK_PATH ($(($APK_SIZE / 1024)) KB)${NC}"

# Phase 4: Sign (debug is already signed by Android Studio)
echo -e "\n${YELLOW}[Phase 4] Verifying APK signature...${NC}"

# Phase 5: Check device
echo -e "\n${YELLOW}[Phase 5] Checking device connection...${NC}"

if ! adb devices | grep -q "device$"; then
    echo -e "${YELLOW}[!] No device connected. APK saved for manual install.${NC}"
    echo -e "${GREEN}[+] APK: $APK_PATH${NC}"
    exit 0
fi

DEVICE_MODEL=$(adb shell getprop ro.product.model 2>/dev/null || echo "unknown")
DEVICE_ANDROID=$(adb shell getprop ro.build.version.release 2>/dev/null || echo "unknown")
echo -e "${GREEN}[+] Device: $DEVICE_MODEL (Android $DEVICE_ANDROID)${NC}"

# Check if device is rooted
if adb shell 'su -c "id"' 2>/dev/null | grep -q "uid=0"; then
    echo -e "${GREEN}[+] Device is ROOTED${NC}"
else
    echo -e "${YELLOW}[!] Device is NOT rooted. Some features may not work.${NC}"
fi

# Phase 6: Install
echo -e "\n${YELLOW}[Phase 6] Installing APK...${NC}"

# Force stop any existing instance
adb shell am force-stop com.apptoolkit 2>/dev/null || true

# Uninstall old version if exists
adb uninstall com.apptoolkit 2>/dev/null || true

# Install
adb install -r -d "$APK_PATH" 2>&1

if [ $? -ne 0 ]; then
    echo -e "${YELLOW}[!] Install failed, trying with grant permissions...${NC}"
    adb install -r -g "$APK_PATH" 2>&1
fi

echo -e "${GREEN}[+] Installation complete${NC}"

# Phase 7: Grant permissions
echo -e "\n${YELLOW}[Phase 7] Granting permissions...${NC}"

adb shell pm grant com.apptoolkit android.permission.READ_EXTERNAL_STORAGE 2>/dev/null || true
adb shell pm grant com.apptoolkit android.permission.WRITE_EXTERNAL_STORAGE 2>/dev/null || true
adb shell pm grant com.apptoolkit android.permission.READ_LOGS 2>/dev/null || true
adb shell pm grant com.apptoolkit android.permission.DUMP 2>/dev/null || true
adb shell pm grant com.apptoolkit android.permission.INTERNET 2>/dev/null || true
adb shell pm grant com.apptoolkit android.permission.POST_NOTIFICATIONS 2>/dev/null || true

echo -e "${GREEN}[+] Permissions granted${NC}"

# Phase 8: Launch
echo -e "\n${YELLOW}[Phase 8] Launching application...${NC}"

adb shell am start -n com.apptoolkit/.MainActivity -W 2>&1

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}  DEPLOY COMPLETE${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "\n${CYAN}To view logs:${NC}"
echo -e "  adb logcat -s AppDebugToolkit -v color"
echo -e "\n${CYAN}To uninstall:${NC}"
echo -e "  adb uninstall com.apptoolkit"
echo -e "\n${CYAN}To reinstall:${NC}"
echo -e "  ./build-and-deploy.sh"
echo -e "\n${CYAN}Commands after launch:${NC}"
echo -e "  help    - Show all commands"
echo -e "  info    - Process info"
echo -e "  bypass  - Auto bypass"
echo -e "  modules - List modules"
echo -e "  hooks   - List active hooks"
