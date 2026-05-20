#include <jni.h>
#include <string.h>
#include <android/log.h>

#define TAG "wydra-apk"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

jstring Java_com_wydra_apk_MainActivity_stringFromJNI(JNIEnv *env, jobject thiz) {
    LOGI("Native library loaded successfully on ARM64!");
    const char *message = "Hello from Wydra Native C Code - Android 15 ARM64";
    return (*env)->NewStringUTF(env, message);
}
