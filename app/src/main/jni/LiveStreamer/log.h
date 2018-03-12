#ifndef NSTREAMING_LOG_H
#define NSTREAMING_LOG_H

#include <android/log.h>

#define LOG_TAG "LiveStreamer_native"

#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG,__VA_ARGS__)
#endif