//
// Created by liuxin on 19-11-12.
//


#include "EglThread.h"
#include "Egl_Helper.h"

EglThread::EglThread() {
    pthread_mutex_init(&pthreadMutex, NULL);
    pthread_cond_init(&pthreadCond, NULL);
    pthread = -1;
    surfaceHeight = 0;
    surfaceWidth = 0;
    nativeWindow = NULL;
    isChange = false;
    isCreate = false;
    isExit = false;
    isStart = false;
    rendType = OPENGL_RENDAR_AUTO;

    pthread_mutex_init(&pDrawMutex,NULL);


}

EglThread::~EglThread() {

    nativeWindow = NULL;
    pthread_mutex_destroy(&pDrawMutex);
    pthread_cond_destroy(&pthreadCond);
    pthread_mutex_destroy(&pthreadMutex);


}

void *eglThreadCallBack(void *context) {
    auto *eglThread = static_cast<EglThread *>(context);
    if (eglThread != NULL) {
        Egl_Helper *eglHelper = new Egl_Helper();
        eglHelper->init(eglThread->nativeWindow);
        eglThread->isExit = false;
        while (true) {
            if (eglThread->isCreate) {
                ALOGE("egl Create");
                eglThread->isCreate = false;
                if (eglThread->onCreate != NULL) {
                    eglThread->onCreate(eglThread->onCreateCtx);
                }

            }

            if (eglThread->isChange) {
                eglThread->isChange = false;
                if (eglThread->onChange != NULL) {
                    ALOGE("egl onChange");
                    eglThread->onChange(eglThread->surfaceWidth, eglThread->surfaceHeight,
                                        eglThread->onChangeCtx);
                }
                eglThread->isStart = true;

            }

            if (eglThread->isChangeFilter) {
                eglThread->isChangeFilter = false;
                if (eglThread->onChangeFilter != NULL) {
                    eglThread->onChangeFilter(eglThread->surfaceWidth, eglThread->surfaceHeight,
                                              eglThread->onChangeFilterCtx);
                }
            }


            if (eglThread->isStart) {
                pthread_mutex_lock(&eglThread->pDrawMutex);
                if (eglThread->onDraw != NULL) {
                    ALOGE("egl nDraw");
                    eglThread->onDraw(eglThread->onDrawCtx);
                }
                eglHelper->swapBuffers();
                pthread_mutex_unlock(&eglThread->pDrawMutex);
            }
            if (eglThread->rendType == OPENGL_RENDAR_AUTO) {
                //60帧每秒
                if (eglThread->isExit) {
                    ALOGE("egl 线程线程销毁");
                    if (eglThread->onOwnDestory != NULL) {
                        eglThread->onOwnDestory(eglThread->onOwnDestoryCtx);
                    }

                    eglHelper->destory();
                    delete eglHelper;
                    eglHelper = NULL;

                    break;
                }
                usleep(1000 * 1000 / 60);
            } else {
                if (eglThread->isExit) {
                    ALOGE("egl 线程线程销毁");
                    if (eglThread->onOwnDestory != NULL) {
                        eglThread->onOwnDestory(eglThread->onOwnDestoryCtx);
                    }

                    eglHelper->destory();
                    delete eglHelper;
                    eglHelper = NULL;

                    break;
                }
                pthread_mutex_lock(&eglThread->pthreadMutex);
                ALOGE("egl 线程等待");
                pthread_cond_wait(&eglThread->pthreadCond, &eglThread->pthreadMutex);
                pthread_mutex_unlock(&eglThread->pthreadMutex);
                ALOGE("egl 线程等待结束");
            }


            // ALOGE("EGL线程创建完成");


        }


    }
    return 0;


}

void EglThread::onSurfaceCreated(EGLNativeWindowType window) {
    if (pthread == -1) {

        isCreate = true;
        this->nativeWindow = window;
        pthread_create(&pthread, NULL, eglThreadCallBack, this);

    }

}

void EglThread::onSurfaceChanged(int w, int h) {
    isChange = true;
    this->surfaceWidth = w;
    this->surfaceHeight = h;
    notifyDraw();
}

void EglThread::onSurfaceCreatedCallBack(EglThread::OnCreate create, void *ctx) {
    this->onCreate = create;
    this->onCreateCtx = ctx;
}

void EglThread::onSurfaceChangeCallBack(EglThread::OnChange change, void *ctx) {
    this->onChange = change;
    this->onChangeCtx = ctx;
}

void EglThread::onSurfaceDrawCallBack(EglThread::OnDraw draw, void *ctx) {
    this->onDraw = draw;
    this->onDrawCtx = ctx;
}

void EglThread::setRendType(int type) {
    this->rendType = type;

}

void EglThread::notifyDraw() {
    pthread_mutex_lock(&pthreadMutex);

    pthread_cond_signal(&pthreadCond);
    pthread_mutex_unlock(&pthreadMutex);

}

void EglThread::onDestory() {
    isExit = true;
      ALOGE("EglThread =onDestory");
    if (pthread != -1) {
        ALOGE("pthread !=-1");
        notifyDraw();
        ALOGE("notifyDraw()");
        pthread_join(pthread, NULL);

        ALOGE("pthread释放完成");
    }
    nativeWindow = NULL;
    pthread = -1;
}

void EglThread::onSurfaceChangeFilter() {
    isChangeFilter = true;
    notifyDraw();
}

void EglThread::onSurfaceChangeFilterCallBack(EglThread::OnChangeFilter changeFilter, void *ctx) {
    this->onChangeFilter = changeFilter;
    this->onChangeFilterCtx = ctx;
}

void EglThread::onSurfaceOwnDestoryCallBack(EglThread::OnOwnDestory ownDestory, void *ctx) {
    this->onOwnDestory = ownDestory;
    this->onOwnDestoryCtx = ctx;
}



