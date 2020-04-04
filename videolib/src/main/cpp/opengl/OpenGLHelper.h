//
// Created by liuxin on 19-11-23.
//

#ifndef OPENGL_OPENGLHELPER_H
#define OPENGL_OPENGLHELPER_H

#include <EGL/eglplatform.h>
#include "../egl/EglThread.h"
#include "android/native_window_jni.h"
#include "BaseOpenGL.h"
#include "OpenGLCommonFilter.h"
#include "OpenGLGreyFilter.h"
#include "OpenGLYUVFilter.h"
#include <cstdlib>
class OpenGLHelper {

public:
    EGLNativeWindowType windowType=NULL;
    EglThread *eglThread=NULL;
    void *mPixels=NULL;
    BaseOpenGL *baseOpenGl=NULL;

    int picWidth=0;
    int picHeight=0;

    uint8_t *y=NULL;

    uint8_t *u=NULL;

    uint8_t *v=NULL;

    bool  isExit= false;






public:
    OpenGLHelper();
    ~OpenGLHelper();
    void  onCreate(JNIEnv *env,jobject surface);
    void  onChange(int width,int height);

    void  onDestroySurface();

    void  changeFilter();

    void  setPixels(void *pixels, int picWidth, int picHeight,int length);

    void setYUVPixels(uint8_t *Y, uint8_t *U, uint8_t *V,int picWidth,int picHeight);

    void setYUVPixels(uint8_t *data,int picWidth,int picHeight,int formatType,int width, int heigth);


};


#endif //OPENGL_OPENGLHELPER_H
