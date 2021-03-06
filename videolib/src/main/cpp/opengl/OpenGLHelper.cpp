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

void OpenGLHelper::setYUVPixels(uint8_t *Y, uint8_t *U, uint8_t *V, int picWidth, int picHeight) {
   if(Y==NULL||U==NULL||V==NULL||picWidth==0||picHeight==0||isExit||!eglThread->isStart) return;

    if(IS_DEBUG){
        ALOGD("picWidth=>%d,picHeigth=>%d",picWidth,picHeight);
    }
    pthread_mutex_lock(&eglThread->pthreadRender);
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
        y= static_cast<uint8_t *>(malloc(picWidth * picHeight));
        u=static_cast<uint8_t *>(malloc(picWidth*picHeight/4));
        v=static_cast<uint8_t *>(malloc(picWidth*picHeight/4));

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
    pthread_mutex_unlock(&eglThread->pthreadRender);


}

void OpenGLHelper::setYUVPixels(uint8_t *yuvdata_, int picWidth, int picHeight,int formattype,int width,int heigth) {
    if(yuvdata_==NULL||picWidth==0||picHeight==0||isExit||!eglThread->isStart) return;

    if(IS_DEBUG){
        ALOGD("picWidth=>%d,picHeigth=>%d",picWidth,picHeight);
    }
    pthread_mutex_lock(&eglThread->pthreadRender);
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
        y=static_cast<uint8_t *>(malloc(picWidth*picHeight));
        u=static_cast<uint8_t *>(malloc(picWidth*picHeight/4));
        v=static_cast<uint8_t *>(malloc(picWidth*picHeight/4));
    }
    if(IS_DEBUG){
        ALOGD("type=>%d",formattype);
    }
    if (formattype == 1) {
        //yuv nv12 yyyyy uv uv


        memcpy(y, yuvdata_, picWidth * picHeight);

        uint8_t * currentP = yuvdata_ + (picWidth * picHeight);

        int i = 0;
        int j=0;
        for (i; i <(picWidth * picHeight) / 2; i=i+2,j++) {
            u[j] = currentP[i];
            v[j] = currentP[i+1];
        }

    } else {
        //yuv420p yyyy u v


        memcpy(y, yuvdata_, picWidth * picHeight);
        memcpy(u, yuvdata_ + (picWidth * picHeight), picWidth * picHeight / 4);
        memcpy(v, yuvdata_ + (5 * picWidth * picHeight / 4), picWidth * picHeight / 4);
    }

    if(baseOpenGl!= NULL){
        baseOpenGl->setYUVPixels(y,u,v,picWidth,picHeight);
    }

    if(eglThread!=NULL){
        eglThread->notifyDraw();
    }

    pthread_mutex_unlock(&eglThread->pthreadRender);
}
