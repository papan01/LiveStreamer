#include "edu_papan01_livestreamer_Live555Native.h"
#include "live_streamer.h"

extern "C"
{
JNIEXPORT jboolean JNICALL Java_edu_papan01_livestreamer_Live555Native_initialize
  (JNIEnv *, jobject, jint, jint);
JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_loopNative
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_stopNative
  (JNIEnv *, jobject);
JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_feedH264Data
  (JNIEnv *, jobject, jbyteArray);
JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_destroy
  (JNIEnv *, jobject);
}

LiveStreamer* getInstance(JNIEnv *env, jobject jthiz)
{
    const char* fieldName = "NativeInstance";
    jclass cls = env->GetObjectClass(jthiz);
    jfieldID instanceFieldId = env->GetFieldID(cls, fieldName, "J");
    if(instanceFieldId == NULL){
        LOGI("LiveStreamer_Native has no long field named with: NativeInstance");
        return NULL;
    }
    jlong instanceValue = env->GetLongField(jthiz,instanceFieldId);
    if(instanceValue == 0)
    {
        LOGI("instanceValue NULL ");
        return NULL;
    }
    else{
        LOGI("instanceValue NOT NULL ");
    }
    return (LiveStreamer*)instanceValue;
}

void storeInstance(JNIEnv *env, jobject jthiz,LiveStreamer* instance)
{
    const char* fieldName = "NativeInstance";
    jclass cls = env->GetObjectClass(jthiz);
    jfieldID instanceFieldId = env->GetFieldID(cls, fieldName, "J");
    if(instanceFieldId == NULL){
        LOGI("LiveStreamer_Native has no long field named with: NativeInstance");
        return;
    }
    jlong value = (instance == NULL) ? 0 : (jlong)instance;
    env->SetLongField(jthiz,instanceFieldId,value);
}


JNIEXPORT jboolean JNICALL Java_edu_papan01_livestreamer_Live555Native_initialize
        (JNIEnv *env, jobject jthiz, jint fps, jint port)
{
    LiveStreamer* streamer = new LiveStreamer(fps,port);
    storeInstance(env,jthiz,streamer);
    return streamer->init();
}

JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_loopNative
        (JNIEnv *env, jobject jthiz)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer != NULL) streamer->loop();
}


JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_stopNative
        (JNIEnv *env, jobject jthiz)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer != NULL) streamer->stop();
}

JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_feedH264Data
        (JNIEnv *env, jobject jthiz, jbyteArray dataArray)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer == NULL) return;
    int len = env->GetArrayLength(dataArray);
    char* buf = new char[len];
    env->GetByteArrayRegion (dataArray, 0, len, reinterpret_cast<jbyte*>(buf));
    streamer->dataPushed(buf,len);
}

JNIEXPORT void JNICALL Java_edu_papan01_livestreamer_Live555Native_destroy
        (JNIEnv *env, jobject jthiz)
{
    LiveStreamer* streamer = getInstance(env,jthiz);
    if(streamer != NULL){
        delete streamer;
        streamer = 0;
        storeInstance(env,jthiz,NULL);
    }
}