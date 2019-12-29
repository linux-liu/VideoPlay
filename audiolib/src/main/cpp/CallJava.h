//
// Created by liuxin on 19-8-24.
//

#ifndef AUDIOPLAY_CALLJAVA_H
#define AUDIOPLAY_CALLJAVA_H


#include "jni.h"
#include "mylog.h"
#define MainThread 1
#define ChildThread 0

class CallJava {
public:
    JavaVM *jvm=NULL;
    JNIEnv *jnv=NULL;
    jobject jobj=NULL;
    jmethodID jmd=NULL;

    jmethodID jstatusmd=NULL;

    jmethodID jprogressmd=NULL;

    jmethodID jcompletemd=NULL;

    jmethodID jerrormd=NULL;

    jmethodID  jdbmd=NULL;

    jmethodID  jloadmd=NULL;

    jmethodID  jsupporthardcode=NULL;

    jmethodID  jmediacodecinit=NULL;

    jmethodID jmediacodedecode=NULL;






public:
    CallJava(JavaVM *javaVM,JNIEnv *jniEnv,jobject jobj);

    void onPrePare(int type);
    void onProgress(int type,int duration,int current);

    void onComplete(int type);

    void onError(int type,int code,char *msg);

    void onPlayStatus(int type,int status);



    void onDb(int type,int db);

    void onLoad(int type, bool isload);

    bool isSupportHardCode(int type,const char *name);

    void onMediaCodecInit(int type, const char *name,int width,int height, const uint8_t *csd_0,
                          int csd_0_size,const uint8_t *csd_1,int csd_1_siz);

    void onMediaCodeDecode(int type, const uint8_t *data,int size);

    ~CallJava();

};


#endif //AUDIOPLAY_CALLJAVA_H
