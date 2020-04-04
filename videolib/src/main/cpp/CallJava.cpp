//
// Created by liuxin on 19-8-24.
//

#include "CallJava.h"

CallJava::CallJava(JavaVM *javaVM, JNIEnv *jniEnv, jobject obj) {
    this->jvm = javaVM;
    this->jnv = jniEnv;
    this->jobj = obj;
    jclass jcls = jniEnv->GetObjectClass(jobj);
    this->jmd = jniEnv->GetMethodID(jcls, "JniCallPrepare", "()V");
    this->jprogressmd = jniEnv->GetMethodID(jcls, "JniCallProgress", "(II)V");
    this->jstatusmd = jniEnv->GetMethodID(jcls, "JniCallPlayStatus", "(I)V");
    this->jcompletemd = jniEnv->GetMethodID(jcls, "JniCallComplete", "()V");
    this->jerrormd = jniEnv->GetMethodID(jcls, "JniCallError", "(ILjava/lang/String;)V");
    this->jdbmd = jniEnv->GetMethodID(jcls, "JniCallDB", "(I)V");
    this->jloadmd = jniEnv->GetMethodID(jcls, "JniCallLoad", "(Z)V");
    this->jsupporthardcode = jniEnv->GetMethodID(jcls, "JniCallSupportHard",
                                                 "(Ljava/lang/String;)Z");
    this->jmediacodecinit = jniEnv->GetMethodID(jcls, "JniCallMeadCodecInit",
                                                "(Ljava/lang/String;II[B[B)V");
    this->jmediacodedecode = jniEnv->GetMethodID(jcls, "JNICallDecodeAvPacket", "([BI)V");

}

void CallJava::onPrePare(int type) {
    if (IS_DEBUG) {
        ALOGD("onPrePare=>%d", type);
    }
    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);

        jniEnv->CallVoidMethod(jobj, jmd);

        jvm->DetachCurrentThread();
    } else if (type == MainThread) {
        jnv->CallVoidMethod(jobj, jmd);
    }
}

CallJava::~CallJava() {

}

void CallJava::onProgress(int type, int duration, int current) {
    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);

        jniEnv->CallVoidMethod(jobj, jprogressmd, duration, current);

        jvm->DetachCurrentThread();
    } else if (type == MainThread) {

        jnv->CallVoidMethod(jobj, jprogressmd, duration, current);
    }

}

void CallJava::onPlayStatus(int type, int status) {

    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);

        jniEnv->CallVoidMethod(jobj, jstatusmd, status);

        jvm->DetachCurrentThread();
    } else if (type == MainThread) {

        jnv->CallVoidMethod(jobj, jstatusmd, status);
    }
}

void CallJava::onComplete(int type) {
    if (IS_DEBUG) {
        ALOGD("onComplete=>%d", type);
    }
    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);

        jniEnv->CallVoidMethod(jobj, jcompletemd);

        jvm->DetachCurrentThread();
    } else if (type == MainThread) {

        jnv->CallVoidMethod(jobj, jcompletemd);
    }
}

void CallJava::onError(int type, int code, char *msg) {
    if (IS_DEBUG) {
        ALOGD("onError=>%d", code);
    }

    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);

        jstring strMsg = jniEnv->NewStringUTF(msg);

        jniEnv->CallVoidMethod(jobj, jerrormd, code, strMsg);

        jniEnv->DeleteLocalRef(strMsg);

        jvm->DetachCurrentThread();
    } else if (type == MainThread) {
        jstring strMsg = jnv->NewStringUTF(msg);
        jnv->CallVoidMethod(jobj, jerrormd, code, strMsg);

        jnv->DeleteLocalRef(strMsg);
    }
}

void CallJava::onDb(int type, int db) {
    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);

        jniEnv->CallVoidMethod(jobj, jdbmd, db);

        jvm->DetachCurrentThread();
    } else if (type == MainThread) {

        jnv->CallVoidMethod(jobj, jdbmd, db);
    }
}

void CallJava::onLoad(int type, bool isload) {
    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);
        auto jload = (isload ? JNI_TRUE : JNI_FALSE);

        jniEnv->CallVoidMethod(jobj, jloadmd, jload);

        jvm->DetachCurrentThread();
    } else if (type == MainThread) {
        auto jload = (isload ? JNI_TRUE : JNI_FALSE);
        jnv->CallVoidMethod(jobj, jloadmd, jload);
    }
}

bool CallJava::isSupportHardCode(int type, const char *name) {
    bool support = false;
    if (type == ChildThread) {
        bool support = false;
        JNIEnv *jniEnv;
        if (jvm->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (IS_DEBUG) {
                ALOGE("call onCallComplete worng");
            }
            return support;
        }

        jstring t = jniEnv->NewStringUTF(name);
        support = jniEnv->CallBooleanMethod(jobj, jsupporthardcode, t);

        ALOGD("support1=>%d", support);
        jniEnv->DeleteLocalRef(t);
        ALOGD("support2=>%d", support);
        jvm->DetachCurrentThread();

        ALOGD("support3=>%d", support);
        return support;


    } else if (type == MainThread) {
        jstring name_ = jnv->NewStringUTF(name);

        support = jnv->CallBooleanMethod(jobj, jsupporthardcode, name_);

        jnv->DeleteLocalRef(name_);
    }

    ALOGD("isSupport=%d", support);

    return support;
}

void
CallJava::onMediaCodecInit(int type, const char *name, int width, int height, const uint8_t *csd_0,
                           int csd_0_size, const uint8_t *csd_1, int csd_1_size) {

    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);
        jstring name_ = jniEnv->NewStringUTF(name);

        jbyteArray csd_0_ = jniEnv->NewByteArray(csd_0_size);
        jniEnv->SetByteArrayRegion(csd_0_, 0, csd_0_size, reinterpret_cast<const jbyte *>(csd_0));

        jbyteArray csd_1_ = jniEnv->NewByteArray(csd_1_size);
        jniEnv->SetByteArrayRegion(csd_1_, 0, csd_1_size, reinterpret_cast<const jbyte *>(csd_1));


        jniEnv->CallVoidMethod(jobj, jmediacodecinit, name_, width, height, csd_0_, csd_1_);

        jniEnv->DeleteLocalRef(csd_1_);
        jniEnv->DeleteLocalRef(csd_0_);

        jniEnv->DeleteLocalRef(name_);
        jvm->DetachCurrentThread();

    } else if (type == MainThread) {
        jstring name_ = jnv->NewStringUTF(name);

        jbyteArray csd_0_ = jnv->NewByteArray(csd_0_size);
        jnv->SetByteArrayRegion(csd_0_, 0, csd_0_size, reinterpret_cast<const jbyte *>(csd_0));

        jbyteArray csd_1_ = jnv->NewByteArray(csd_1_size);
        jnv->SetByteArrayRegion(csd_1_, 0, csd_1_size, reinterpret_cast<const jbyte *>(csd_1));


        jnv->CallVoidMethod(jobj, jmediacodecinit, name_, width, height, csd_0_, csd_1_);

        jnv->DeleteLocalRef(csd_1_);
        jnv->DeleteLocalRef(csd_0_);

        jnv->DeleteLocalRef(name_);
    }


}

void CallJava::onMediaCodeDecode(int type, const uint8_t *data, int size) {
    if (type == ChildThread) {
        JNIEnv *jniEnv = NULL;
        jvm->AttachCurrentThread(&jniEnv, NULL);

        jbyteArray data_ = jniEnv->NewByteArray(size);

        jniEnv->SetByteArrayRegion(data_, 0, size, reinterpret_cast<const jbyte *>(data));

        jniEnv->CallVoidMethod(jobj, jmediacodedecode, data_, size);
        jniEnv->DeleteLocalRef(data_);


        jvm->DetachCurrentThread();

    } else if (type == MainThread) {
        jbyteArray data_ = jnv->NewByteArray(size);

        jnv->SetByteArrayRegion(data_, 0, size, reinterpret_cast<const jbyte *>(data));

        jnv->CallVoidMethod(jobj, jmediacodedecode, data_, size);
        jnv->DeleteLocalRef(data_);

    }
}




