# Wydra APK - Android 15 ARM64 Debug Toolkit

[![Build Status](https://github.com/wedikcz/wydra-apk/actions/workflows/android-build.yml/badge.svg)](https://github.com/wedikcz/wydra-apk/actions)
[![License: BSL 1.0](https://img.shields.io/badge/License-BSL%201.0-blue.svg)](LICENSE)

Profesionální Android aplikace pro debugování a inspekci aplikací na Android 15 (API 35) s nativním C kódem optimalizovaným pro ARM64 architekturu.

## 📋 Obsah

- [Přehled](#přehled)
- [Systémové požadavky](#systémové-požadavky)
- [Instalace](#instalace)
- [Build](#build)
- [Struktura projektu](#struktura-projektu)
- [Konfigurace](#konfigurace)
- [Řešení problémů](#řešení-problémů)
- [Contributing](#contributing)
- [Licence](#licence)

## 📱 Přehled

Wydra APK je komplexní Android aplikace určená pro:

- **Debugování**: Inspekce běžících aplikací
- **Monitoring**: Sledování systémových prostředků
- **JNI Integration**: Propojení Java a nativního C kódu
- **Android 15 Kompatibilita**: Plná podpora nejnovější verze Androidu
- **ARM64 Optimalizace**: Výkonná implementace pro 64-bitové procesory

## 🔧 Systémové požadavky

### Hardware
- **Procesor**: ARM64 (64-bit)
- **RAM**: Minimálně 2GB (doporučeno 4GB+)
- **Disk**: Minimálně 500MB volného místa

### Software
- **Android API**: 35 (Android 15) - minimum API 24 (Android 7.0)
- **Gradle**: 8.6+
- **Android Studio**: Arctic Fox nebo novější
- **Java/JDK**: 11+
- **NDK**: 27.0.12077973 nebo novější
- **CMake**: 3.22.1+

## 📥 Instalace

### Příprava prostředí

1. **Nainstalujte Android Studio** z [developer.android.com](https://developer.android.com/studio)

2. **Nainstalujte NDK a CMake** v Android Studio:
   ```
   SDK Manager → SDK Tools
   ✓ Android NDK (version 27.0.12077973 nebo novější)
   ✓ CMake (version 3.22.1+)
   ```

3. **Klonujte repozitář**:
   ```bash
   git clone https://github.com/wedikcz/wydra-apk.git
   cd wydra-apk
   ```

4. **Stáhněte dependencies**:
   ```bash
   ./gradlew build
   ```

## 🏗️ Build

### Build Debug APK

```bash
# Přímý build
./gradlew assembleDebug

# Build s verbose výstupem
./gradlew assembleDebug --info

# Build bez cache
./gradlew clean assembleDebug
```

**Výstup**: `app/build/outputs/apk/debug/app-debug.apk`

### Build Release APK

```bash
# Release build s optimizacemi
./gradlew assembleRelease

# Podepsaný release (vyžaduje keystore)
./gradlew assembleRelease -Pversion.name=1.0.0
```

**Výstup**: `app/build/outputs/apk/release/app-release-unsigned.apk`

### Build a instalace na zařízení

```bash
# Install debug APK na připojené zařízení
./gradlew installDebug

# Install a spuštění
./gradlew installDebug

# Uninstall
./gradlew uninstallDebug
```

### Volby buildu

```bash
# Build s detailním výstupem C kódu
./gradlew assembleDebug --info

# Build bez optimizací (zjednodušené debugování)
./gradlew assembleDebug -PdisableOptimizations=true

# Build s vyčištěním cache
./gradlew clean assembleDebug
```

## 📁 Struktura projektu

```
wydra-apk/
│
├── app/                              # Android aplikace
│   ├── build.gradle.kts             # Gradle konfigurace (Kotlin DSL)
│   ├── proguard-rules.pro           # ProGuard / R8 obfuskace
│   │
│   └── src/main/
│       ├── cpp/
│       │   ├── CMakeLists.txt       # CMake konfigurace
│       │   └── app-debug-toolkit.c  # Nativní C kód (~2500 řádků)
│       │
│       ├── java/com/apptoolkit/     # Java třídy
│       │   ├── MainActivity.java
│       │   ├── AppDebugApplication.java
│       │   ├── DebugService.java
│       │   ├── NativeBridge.java
│       │   ├── SettingsActivity.java
│       │   ├── AboutActivity.java
│       │   ├── BootReceiver.java
│       │   ├── data/
│       │   │   └── DatabaseHelper.java
│       │   └── util/
│       │       ├── PrefsManager.java
│       │       └── Logger.java
│       │
│       ├── res/
│       │   ├── layout/
│       │   │   ├── activity_main.xml
│       │   │   ├── activity_settings.xml
│       │   │   ├── activity_about.xml
│       │   │   └── drawer_header.xml
│       │   ├── menu/
│       │   │   └── navigation_menu.xml
│       │   ├── values/
│       │   │   ├── strings.xml
│       │   │   ├── colors.xml
│       │   │   ├── themes.xml
│       │   │   └── dimens.xml
│       │   ├── values-v35/
│       │   │   └── themes.xml
│       │   ├── xml/
│       │   │   ├── network_security_config.xml
│       │   │   └── app_config.xml
│       │   └── drawable/
│       │       └── ic_terminal.xml
│       │
│       ├── assets/
│       │   └── frida-agent.js
│       │
│       └── AndroidManifest.xml
│
├── gradle/
│   └── wrapper/                     # Gradle Wrapper
│       ├── gradle-wrapper.jar
│       └── gradle-wrapper.properties
│
├── .github/
│   └── workflows/
│       └── android-build.yml        # GitHub Actions CI/CD
│
├── build.gradle.kts                 # Root build script
├── settings.gradle.kts              # Gradle settings
├── gradle.properties                # Gradle properties
├── gradlew                          # Linux/macOS Gradle wrapper
├── gradlew.bat                      # Windows Gradle wrapper
├── gradlew.sh                       # Shell wrapper
│
├── build-and-deploy.sh              # Deploy script
├── proguard-rules.pro               # ProGuard pravidla
├── checklist.txt                    # Build checklist
│
├── README.md                        # Tato dokumentace
├── LICENSE                          # Boost Software License 1.0
└── .gitignore                       # Git ignore pravidla
```

## ⚙️ Konfigurace

### Android SDK Konfigurace

V `app/build.gradle.kts`:

```kotlin
android {
    compileSdk = 35              // Target SDK (Android 15)
    minSdk = 24                  // Minimum SDK (Android 7.0)
    targetSdk = 35               // Target runtime version
    
    ndkVersion = "27.0.12077973" // NDK verze
    
    defaultConfig {
        applicationId = "com.apptoolkit"
        versionCode = 3           // Build number
        versionName = "3.0.0"     // Public version
        
        // ARM64 architektura
        ndk {
            abiFilters.add("arm64-v8a")
        }
    }
}
```

### CMake Konfigurace

V `app/src/main/cpp/CMakeLists.txt`:

```cmake
# ARM64 specifické nastavení
set(ANDROID_ABI "arm64-v8a")
set(ANDROID_PLATFORM "android-35")
set(ANDROID_STL "c++_shared")

# Optimalizační flagy
target_compile_options(
    appdebugtoolkit
    PRIVATE
    -O2                    # Optimization level 2
    -fPIC                  # Position Independent Code
    -fvisibility=hidden    # Hidden symbols
    -ffunction-sections    # Separate sections
    -fdata-sections        # Separate data
)
```

### Gradle Properties

V `gradle.properties`:

```properties
# JVM nastavení
org.gradle.jvmargs=-Xmx4g

# Gradle daemon
org.gradle.daemon=true
org.gradle.parallel=true
org.gradle.caching=true

# Android specifické
android.useAndroidX=true
android.enableJetifier=true
```

## 🔍 JNI Implementace

### C kod (native-lib.c)

```c
#include <jni.h>
#include <string.h>
#include <android/log.h>

#define TAG "wydra-apk"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

jstring Java_com_apptoolkit_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {
    LOGI("Native function called from Java");
    return (*env)->NewStringUTF(env, "Hello from C - ARM64");
}
```

### Java kod (MainActivity.java)

```java
public class MainActivity extends AppCompatActivity {
    static {
        System.loadLibrary("appdebugtoolkit");  // Načtení nativní knihovny
    }
    
    public native String stringFromJNI();  // JNI deklarace
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String result = stringFromJNI();
        // Použití výsledku...
    }
}
```

## 🐛 Řešení problémů

### 1. "CMake executable not found"

**Řešení:**
```bash
# V Android Studio:
SDK Manager → SDK Tools → CMake → Nainstalujte nejnovější verzi

# Nebo manuálně nastavte v build.gradle.kts:
externalNativeBuild {
    cmake {
        version = "3.22.1"  // Explicitní verze
    }
}
```

### 2. "NDK not found"

**Řešení:**
```bash
# Nainstalujte NDK přes Android Studio
# Nebo nastavte ANDROID_NDK_HOME
export ANDROID_NDK_HOME=/path/to/ndk/27.0.12077973

# Ověřte instalaci
ls $ANDROID_NDK_HOME/
```

### 3. "Build fails for arm64-v8a"

**Ověřte:**
1. NDK verze: `ndkVersion = "27.0.12077973"` v build.gradle.kts
2. CMake verze: `cmake { version = "3.22.1" }`
3. Android API: `targetSdk = 35` a `compileSdk = 35`
4. Gradle verze: `./gradlew --version` (měl by být 8.6+)

**Řešení:**
```bash
# Vyčistěte a znovu buildujte
./gradlew clean assembleDebug --info

# Pokud pořád selže, zkuste:
./gradlew --stop                    # Zastavte Gradle daemon
./gradlew clean
./gradlew assembleDebug
```

### 4. "Gradle build timeout"

**Řešení - zvětšete timeout v gradle.properties:**
```properties
org.gradle.jvmargs=-Xmx4g
org.gradle.daemon=true
org.gradle.parallel=true
```

### 5. "App crashes on startup"

**Ověřte:**
- Knihovna `appdebugtoolkit.so` je součástí APK
- JNI funkce jsou správně definovány
- Native kód se kompiluje bez chyb

```bash
# Zkontrolujte logcat
adb logcat | grep "appdebugtoolkit"

# Ověřte, že .so je v APK
unzip -l app/build/outputs/apk/debug/app-debug.apk | grep "\.so"
```

## 🔄 Continuous Integration

Projekt obsahuje GitHub Actions workflow pro automatizaci buildů:

**Soubor:** `.github/workflows/android-build.yml`

Workflow automaticky:
- Builduje APK na push do `main` branche
- Spouští unit testy
- Kontroluje code quality

## 🤝 Contributing

Příspěvky jsou vítány! 

1. Forkujte repozitář
2. Vytvořte feature branch (`git checkout -b feature/amazing-feature`)
3. Commitujte změny (`git commit -m 'Add amazing feature'`)
4. Pushujte do branche (`git push origin feature/amazing-feature`)
5. Otevřete Pull Request

## 📝 Licence

Projekt je licencován pod **Boost Software License 1.0** - viz [LICENSE](LICENSE) soubor.

## 📞 Support

Pokud máte problémy nebo otázky:

1. Zkontrolujte [Řešení problémů](#řešení-problémů) sekci
2. Otevřete [GitHub Issue](https://github.com/wedikcz/wydra-apk/issues)
3. Zkontrolujte [Android Developer Docs](https://developer.android.com/docs)

## 🎯 Roadmap

- [x] Android 15 ARM64 support
- [x] CMake integration
- [x] JNI implementation
- [ ] Unit tests
- [ ] Performance benchmarks
- [ ] Frida integration
- [ ] Advanced debugging features

---

**Naposledy aktualizováno:** 2026-05-25 | **Verze:** 3.0.0
