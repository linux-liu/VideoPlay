#include <jni.h>
#include <android/log.h>

#include <string>
#include <unistd.h>
#include "LXFFmpeg.h"
#include "opengl/OpenGLHelper.h"


JavaVM *jvm = NULL;
CallJava *callJava = NULL;
LXFFmpeg *lxfFmpeg = NULL;
PlayStatus *playStatus = NULL;
bool isexit = true;
pthread_t releaseThread;
pthread_t seekThread;
jobject globelobject = NULL;
int seekSec = 0;


OpenGLHelper *openGlHelper = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *javaVM, void *reserved) {
    jvm = javaVM;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved) {
    if (globelobject != NULL) {
        JNIEnv *jniEnv = NULL;


        vm->GetEnv((void **) &jniEnv, JNI_VERSION_1_6);
        if (jniEnv != NULL) {
            if (IS_DEBUG) {
                ALOGD("释放资源 JNI_OnUnload");
            }

            jniEnv->DeleteGlobalRef(globelobject);
            globelobject = NULL;
        }
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_prepare(JNIEnv *env, jobject instance, jstring url) {


    if (lxfFmpeg == NULL && isexit) {
        const char *_url = env->GetStringUTFChars(url, NULL);
        if (globelobject == NULL) {
            globelobject = env->NewGlobalRef(instance);
        }
        if (IS_DEBUG) {
            ALOGD("开始初始化....");
        }
        if (callJava == NULL) {
            callJava = new CallJava(jvm, env, globelobject);
        }
        if (playStatus == NULL) {
            playStatus = new PlayStatus();
        }
        lxfFmpeg = new LXFFmpeg(playStatus, _url, callJava);
        lxfFmpeg->setOpenGLHelper(openGlHelper);
        lxfFmpeg->prepare();
        env->ReleaseStringUTFChars(url, _url);
    }


}extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_start(JNIEnv *env, jobject instance) {
    if (lxfFmpeg != NULL) {

        lxfFmpeg->start();


    }


}







extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_play(JNIEnv *env, jobject thiz) {
    if (lxfFmpeg) {
        lxfFmpeg->play();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_pause(JNIEnv *env, jobject thiz) {
    if (lxfFmpeg) {
        lxfFmpeg->pause();

    }
}


static void *releaseCallBack(void *data) {
    isexit = false;
    if (lxfFmpeg) {

        lxfFmpeg->release();
        delete lxfFmpeg;
        lxfFmpeg = NULL;

        if (playStatus) {
            delete playStatus;
            playStatus = NULL;
        }
        if (callJava) {
            delete callJava;
            callJava = NULL;
        }

    }
    isexit = true;
    JNIEnv *jniEnv = NULL;
    jvm->AttachCurrentThread(&jniEnv, NULL);


    jclass jcls = jniEnv->GetObjectClass(globelobject);
    jmethodID jcallswitchmd = jniEnv->GetMethodID(jcls, "JniCallSwitch", "()V");
    jniEnv->CallVoidMethod(globelobject, jcallswitchmd);
    jvm->DetachCurrentThread();
    if (IS_DEBUG) {
        ALOGD("完全释放了内存");
    }

    pthread_exit(&releaseThread);


}


extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_release(JNIEnv *env, jobject thiz) {

    if (!isexit) {
        return;
    }
    if (globelobject == NULL) {
        globelobject = env->NewGlobalRef(thiz);
    }


    pthread_create(&releaseThread, NULL, releaseCallBack, NULL);


}

static void *seekCallBack(void *data) {
    if (lxfFmpeg != NULL) {
        lxfFmpeg->seek(seekSec);
    }
    pthread_exit(&seekThread);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_seek(JNIEnv *env, jobject thiz, jint sec) {

    seekSec = sec;
    pthread_create(&seekThread, NULL, seekCallBack, NULL);


}extern "C"
JNIEXPORT jint JNICALL
Java_com_liuxin_videolib_LXPlayer_getDuration(JNIEnv *env, jobject instance) {
    if (lxfFmpeg != NULL) {
        return lxfFmpeg->getDuration();
    }
    return 0;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setVolume(JNIEnv *env, jobject instance, jint volume) {

    if (lxfFmpeg != NULL) {
        lxfFmpeg->setVolume(volume);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setMute(JNIEnv *env, jobject instance, jboolean mute) {

    if (lxfFmpeg != NULL) {
        lxfFmpeg->setMute(mute);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setChannelSolo(JNIEnv *env, jobject instance, jint channel) {

    if (lxfFmpeg != NULL) {
        lxfFmpeg->setChannelSolo(channel);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setPitch(JNIEnv *env, jobject instance, jdouble pitch) {

    // TODO

    if (lxfFmpeg != NULL) {
        lxfFmpeg->setPitch(pitch);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setTemPo(JNIEnv *env, jobject instance, jdouble temPo) {

    // TODO
    if (lxfFmpeg != NULL) {
        lxfFmpeg->setTempPo(temPo);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setSurface(JNIEnv *env, jobject thiz, jobject surface) {

    ALOGD("create Surface");
    if (openGlHelper == NULL) {
        openGlHelper = new OpenGLHelper();
        openGlHelper->onCreate(env, surface);
    }

    if (lxfFmpeg != NULL) {
        lxfFmpeg->setOpenGLHelper(openGlHelper);
        if (lxfFmpeg->video != NULL) {
            lxfFmpeg->video->setOpenGLHelper(openGlHelper);
        }
    }


}


extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setSurfaceChange(JNIEnv *env, jobject thiz, jint width,

                                                   jint height) {
    ALOGD("Surface change");
    if (openGlHelper != NULL) {
        openGlHelper->onChange(width, height);
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_destorySurface(JNIEnv *env, jobject thiz) {

    if (lxfFmpeg != NULL) {
        lxfFmpeg->setOpenGLHelper(NULL);
        if (lxfFmpeg->video != NULL) {
            lxfFmpeg->video->setOpenGLHelper(NULL);
        }
        lxfFmpeg->pause();
    }
    if (openGlHelper != NULL) {
        openGlHelper->onDestroySurface();
        openGlHelper = NULL;
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_liuxin_videolib_LXPlayer_setYUVData(JNIEnv *env, jobject thiz, jbyteArray yuvdata,
                                             jint pic_width, jint pic_height, jint formattype,jint width,jint height) {


    if (openGlHelper != NULL && yuvdata != NULL) {


        jbyte *yuvdata_ = env->GetByteArrayElements(yuvdata, NULL);

        openGlHelper->setYUVPixels(reinterpret_cast<uint8_t *>(yuvdata_), pic_width, pic_height, formattype,width,height);

        env->ReleaseByteArrayElements(yuvdata, yuvdata_, JNI_FALSE);


    }
}

