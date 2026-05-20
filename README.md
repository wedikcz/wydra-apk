# Wydra APK - Android 15 ARM64 Build

## Přehled

Toto je Android aplikace s nativním C kódem skompilovaným pro ARM64 architekturu cílící na Android 15 (API 35).

## Struktura projektu

```
wydra-apk/
├── CMakeLists.txt              # CMake konfigurace pro nativní kód
├── build.gradle.kts            # Gradle build script (Kotlin DSL)
├── settings.gradle.kts         # Gradle settings
├── gradle.properties           # Gradle properties
├── src/
│   ├── main/
│   │   ├── java/              # Java/Kotlin source code
│   │   ├── jni/               # Nativní C/C++ code
│   │   ├── res/               # Android resources
│   │   └── AndroidManifest.xml
```

## Konfigurace

### Android SDK Požadavky
- **compileSdk**: 35 (Android 15)
- **targetSdk**: 35
- **minSdk**: 24 (Android 7.0)
- **NDK**: 25.x nebo novější
- **Architektura**: arm64-v8a (ARM64)

### CMake Konfigurace
- **CMake verze**: 3.22.1+
- **C++ Standard**: C++17
- **Android ABI**: arm64-v8a
- **Android Platform**: android-35
- **STL**: c++_shared

## Build

### Požadavky
1. Android Studio s NDK
2. Gradle 8.x
3. Kotlin 2.0+

### Build příkazy

```bash
# Build release APK
./gradlew assembleRelease

# Build debug APK
./gradlew assembleDebug

# Build a instalace
./gradlew installDebug

# Run testy
./gradlew test
```

### Výstupní APK
- **Debug**: `app/build/outputs/apk/debug/app-debug.apk`
- **Release**: `app/build/outputs/apk/release/app-release.apk`

## JNI Kód

Nativní C funkce v `src/main/jni/native-lib.c`:

```c
jstring Java_com_wydra_apk_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz)
```

Tato funkce je volána z Kotlin kódu a vrací řetězec z nativní vrstvy.

## Kotlin/Java

**MainActivity.kt** - Hlavní aktivita aplikace, která:
- Načítá nativní knihovnu `wydra_native`
- Volá JNI funkci `stringFromJNI()`
- Zobrazuje výsledek v TextView

## Gradle DSL (Kotlin)

Projekt používá moderní Kotlin DSL pro Gradle (`build.gradle.kts` místo Groovy):

- Lepší type-safety
- IDE autocomplete
- Lepší čitelnost
- Moderní Gradle best-practices

## Systémové požadavky

- **OS**: Windows, macOS, Linux
- **Java**: JDK 11+
- **Android API**: 35 (Android 15)
- **ARM Architektura**: arm64-v8a

## Poznámky

- Aplikace je optimalizovaná pro ARM64 architekturu
- Všechny dependencies jsou verze kompatibilní s Android 15
- NDK je nakonfigurován pro auto-download přes Gradle
- Projekt používá AndroidX libraries

## Licence

Boost Software License 1.0
