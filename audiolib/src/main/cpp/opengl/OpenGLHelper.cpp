//
// Created by liuxin on 19-11-23.
//


#include "OpenGLHelper.h"


void callbackSurfaceCreate(void *context){
    OpenGLHelper *openGlHelper= static_cast<OpenGLHelper *>(context);
    if(openGlHelper!=NULL){
        if(openGlHelper->baseOpenGl!=NULL){
            ALOGE("openHelper onCreate");
            openGlHelper->baseOpenGl->onCreate();
        }

    }
}

void callbackSurfaceChange(int width,int height, void *context){
    OpenGLHelper *openGlHelper= static_cast<OpenGLHelper *>(context);
    if(openGlHelper!=NULL){
        if(openGlHelper->baseOpenGl!=NULL){
            openGlHelper->baseOpenGl->onChange(width,height);
        }
    }
}

void callbackSurfaceDraw(void *context){


    OpenGLHelper *openGlHelper= static_cast<OpenGLHelper *>(context);
    if(openGlHelper!=NULL){
        if(openGlHelper->baseOpenGl!=NULL){
            openGlHelper->baseOpenGl->onDraw();
        }
    }
}

void callbackChangeFilter(int width,int height,void *context){
    OpenGLHelper *openGlHelper= static_cast<OpenGLHelper *>(context);
    if(openGlHelper!=NULL){
        if(openGlHelper->baseOpenGl!=NULL){
            openGlHelper->baseOpenGl->onDestory();
            delete openGlHelper->baseOpenGl;
            openGlHelper->baseOpenGl=NULL;
        }
        openGlHelper->baseOpenGl=new OpenGLGreyFilter();
        openGlHelper->baseOpenGl->onCreate();
        openGlHelper->baseOpenGl->onChange(width,height);
        openGlHelper->baseOpenGl->setPixels(openGlHelper->mPixels,openGlHelper->picWidth,openGlHelper->picHeight);
        openGlHelper->eglThread->notifyDraw();

    }
}

void callbackSurfacOwnDestory(void *context){
    OpenGLHelper *openGlHelper= static_cast<OpenGLHelper *>(context);
    if(openGlHelper!=NULL){
        if(openGlHelper->baseOpenGl!=NULL){
            ALOGE("destory OpenGLYUV");
            openGlHelper->baseOpenGl->onDestory();
            delete openGlHelper->baseOpenGl;
            openGlHelper->baseOpenGl=NULL;
        }
    }
}

void OpenGLHelper::onCreate(JNIEnv *env, jobject surface) {


    windowType=ANativeWindow_fromSurface(env,surface);
    eglThread=new EglThread();
    eglThread->setRendType(OPENGL_RENDAR_MANUAL);
    eglThread->onSurfaceCreatedCallBack(callbackSurfaceCreate,this);
    eglThread->onSurfaceChangeCallBack(callbackSurfaceChange,this);
    eglThread->onSurfaceDrawCallBack(callbackSurfaceDraw,this);
    eglThread->onSurfaceChangeFilterCallBack(callbackChangeFilter,this);
    eglThread->onSurfaceOwnDestoryCallBack(callbackSurfacOwnDestory,this);

     baseOpenGl=new OpenGLYUVFilter();
    eglThread->onSurfaceCreated(windowType);

}

void OpenGLHelper::onChange(int width, int height) {

    if(eglThread!=NULL){
        eglThread->onSurfaceChanged(width,height);
    }
}

OpenGLHelper::OpenGLHelper() {
    isExit= false;
    picWidth=0;
    picHeight=0;


}

OpenGLHelper::~OpenGLHelper() {


}

void OpenGLHelper::onDestroySurface() {
    int try_count = 100000;
    int ret = -1;
    isExit= true;
    ALOGE("开始获egl锁成功");

    ALOGE("获egl锁成功");
    if(eglThread){
        ALOGE("开始eglTHeard释放");
        eglThread->onDestory();
        ALOGE("eglTHeard释放完成");
        delete  eglThread;
        eglThread=NULL;
    }
    if(windowType){
      ANativeWindow_release(windowType);
      windowType=NULL;
    }


    if(mPixels!=NULL){

        free(mPixels);
        mPixels=NULL;
    }

    if(y){
        free(y);
        y=NULL;
    }

    if(u){
        free(u);
        u=NULL;
    }

    if(v){
        free(v);
        v=NULL;
    }
}

void OpenGLHelper::setPixels(void *pixels, int picWidth, int picHeight,int length) {
    if(mPixels!=NULL){
        free(mPixels);
        mPixels=NULL;
    }
    this->picWidth=picWidth;
    this->picHeight=picHeight;

    mPixels=malloc(length);
    memcpy(mPixels,pixels,length);
    if(baseOpenGl!= NULL){
        baseOpenGl->setPixels(mPixels,picWidth,picHeight);
    }
    if(eglThread!=NULL){
        eglThread->notifyDraw();
    }
}

void OpenGLHelper::changeFilter() {
     if(eglThread!=NULL){
         eglThread->onSurfaceChangeFilter();
     }
}

void OpenGLHelper::setYUVPixels(void *Y, void *U, void *V, int picWidth, int picHeight) {
   if(Y==NULL||U==NULL||V==NULL||picWidth==0||picHeight==0||isExit||!eglThread->isStart) return;

   ALOGE("now w=>%d now h=%d preW=>%d preH=>%d",picWidth,picHeight,this->picWidth,this->picHeight);
    pthread_mutex_lock(&eglThread->pDrawMutex);

    if(picWidth!=this->picWidth||picHeight!=this->picHeight){
        this->picWidth=picWidth;
        this->picHeight=picHeight;
        if(y){
            free(y);
            y=NULL;
        }
        if(u){
            free(u);
            u=NULL;
        }
        if(v){
            free(v);
            v=NULL;
        }
        y=malloc(picWidth*picHeight);
        u=malloc(picWidth*picHeight/4);
        v=malloc(picWidth*picHeight/4);

    }



    memcpy(y,Y,picWidth*picHeight);
    memcpy(u,U,picWidth*picHeight/4);
    memcpy(v,V,picWidth*picHeight/4);

    if(baseOpenGl!= NULL){
        baseOpenGl->setYUVPixels(y,u,v,picWidth,picHeight);
    }

    if(eglThread!=NULL){
        eglThread->notifyDraw();
    }

    pthread_mutex_unlock(&eglThread->pDrawMutex);

}
