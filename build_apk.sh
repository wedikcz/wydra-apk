#!/bin/bash

# Wydra APK Build Script
# Android 15 ARM64 Debug Toolkit

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Wydra APK - Android 15 ARM64 Builder${NC}"
echo -e "${GREEN}========================================${NC}\n"

# Phase 1: Environment Check
echo -e "${YELLOW}[Phase 1] Checking environment...${NC}"

if ! command -v java &> /dev/null; then
    echo -e "${RED}[!] Java not found. Install JDK 11+${NC}"
    exit 1
fi

JAVA_VERSION=$(java -version 2>&1 | head -1)
echo -e "${GREEN}[+] $JAVA_VERSION${NC}"

# Phase 2: Make gradlew executable
echo -e "\n${YELLOW}[Phase 2] Preparing Gradle wrapper...${NC}"
chmod +x gradlew
echo -e "${GREEN}[+] Gradle wrapper ready${NC}"

# Phase 3: Clean build
echo -e "\n${YELLOW}[Phase 3] Cleaning previous builds...${NC}"
./gradlew clean --info 2>&1 | tail -5
echo -e "${GREEN}[+] Clean complete${NC}"

# Phase 4: Build Debug APK
echo -e "\n${YELLOW}[Phase 4] Building Debug APK...${NC}"
echo -e "${YELLOW}Target: Android 15 (API 35) ARM64${NC}"
echo -e "${YELLOW}Output: app/build/outputs/apk/debug/app-debug.apk${NC}\n"

./gradlew assembleDebug --info 2>&1 | grep -E "(BUILD|compiling|linking|Creating)" | tail -20

if [ $? -ne 0 ]; then
    echo -e "${RED}[!] Build FAILED${NC}"
    exit 1
fi

# Phase 5: Verify APK
echo -e "\n${YELLOW}[Phase 5] Verifying APK...${NC}"

APK_PATH="app/build/outputs/apk/debug/app-debug.apk"
if [ ! -f "$APK_PATH" ]; then
    echo -e "${RED}[!] APK not found at $APK_PATH${NC}"
    exit 1
fi

APK_SIZE=$(stat -f%z "$APK_PATH" 2>/dev/null || stat -c%s "$APK_PATH" 2>/dev/null || echo "0")
APK_SIZE_MB=$(echo "scale=2; $APK_SIZE / 1024 / 1024" | bc 2>/dev/null || echo "unknown")

echo -e "${GREEN}[+] APK Successfully Built!${NC}"
echo -e "${GREEN}[+] Location: $APK_PATH${NC}"
echo -e "${GREEN}[+] Size: $APK_SIZE_MB MB${NC}"

# Phase 6: Check native libraries
echo -e "\n${YELLOW}[Phase 6] Verifying native libraries...${NC}"
if unzip -l "$APK_PATH" | grep -q "\.so"; then
    echo -e "${GREEN}[+] Native libraries (.so) found in APK${NC}"
    unzip -l "$APK_PATH" | grep "\.so"
else
    echo -e "${RED}[!] WARNING: No .so libraries found${NC}"
fi

echo -e "\n${GREEN}========================================${NC}"
echo -e "${GREEN}BUILD COMPLETE! ✅${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}APK: $APK_PATH${NC}"
echo -e "${GREEN}Size: $APK_SIZE_MB MB${NC}"
echo -e "${GREEN}API Level: 35 (Android 15)${NC}"
echo -e "${GREEN}Architecture: ARM64${NC}"
echo -e "${GREEN}========================================${NC}\n"
