//
// Created by liuxin on 19-11-9.
//

#include "Egl_Helper.h"


Egl_Helper::Egl_Helper() {
    mEglDisplay=EGL_NO_DISPLAY;
    mEglSurface=EGL_NO_SURFACE;
    mEGlConfig=NULL;
    mEGLContext=EGL_NO_CONTEXT;


}

Egl_Helper::~Egl_Helper() {

}

int Egl_Helper::init(EGLNativeWindowType windowType) {
   mEglDisplay=eglGetDisplay(EGL_DEFAULT_DISPLAY);

   if(mEglDisplay==EGL_NO_DISPLAY){
       if(IS_DEBUG){
           ALOGE("EGL NO Display");
       }
       return -1;
   }

     EGLint version[2]={0,0};

   if(!eglInitialize(mEglDisplay,&version[0],&version[1])){
       if(IS_DEBUG){
           ALOGE("init egl error");
       }
       return -1;
   }
    const EGLint  attribs[]={
            EGL_BUFFER_SIZE, 32,
            EGL_ALPHA_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE

   };

     EGLint num_config;
     //获取config的值
     if(!eglChooseConfig(mEglDisplay,attribs,&mEGlConfig,1,&num_config)){
         if(IS_DEBUG){
             ALOGE("egl choose config error");
         }
         return  -1;
     }

    if(num_config<=0){
        if(IS_DEBUG){
            ALOGE("egl choose config error");
        }
        return -1;
    }
    if(mEGlConfig==NULL){
        if(IS_DEBUG){
            ALOGE("egl choose config error");
        }
        return -1;
    }

     const EGLint attrs[]={
             EGL_CONTEXT_CLIENT_VERSION,2,
             EGL_NONE
     };

     mEGLContext=eglCreateContext(mEglDisplay,mEGlConfig,EGL_NO_CONTEXT,attrs);

     if(mEGLContext==EGL_NO_CONTEXT){
         if(IS_DEBUG){
             ALOGE("egl context error");
         }
         return -1;
     }
     mEglSurface=eglCreateWindowSurface(mEglDisplay,mEGlConfig,windowType,NULL);

     if(mEglSurface==EGL_NO_SURFACE){
         if(IS_DEBUG){
             ALOGE("egl surface error");
         }
         return -1;
     }

     if(!eglMakeCurrent(mEglDisplay,mEglSurface,mEglSurface,mEGLContext)){
         if(IS_DEBUG){
             ALOGE("egl make current error");
         }
         return -1;
     }

     ALOGE("egl环境成功");

    return 0;
}

void Egl_Helper::destory() {
     ALOGE("destory EglHelper");
    if(mEglDisplay!=EGL_NO_DISPLAY){
        eglMakeCurrent(mEglDisplay,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
    }

    if(mEglDisplay!=EGL_NO_DISPLAY&&mEglSurface!=EGL_NO_SURFACE){
        eglDestroySurface(mEglDisplay,mEglSurface);
        mEglSurface=EGL_NO_SURFACE;
    }


    if(mEglDisplay!=EGL_NO_DISPLAY&&mEGLContext!=EGL_NO_CONTEXT){
        eglDestroyContext(mEglDisplay,mEGLContext);
        mEGLContext=EGL_NO_CONTEXT;
    }
    if(mEglDisplay!=EGL_NO_DISPLAY){
        eglTerminate(mEglDisplay);
        mEglDisplay=EGL_NO_DISPLAY;
    }



}

int Egl_Helper::swapBuffers() {
    if(mEglDisplay!=EGL_NO_DISPLAY&&mEglSurface!=EGL_NO_SURFACE){
        if(eglSwapBuffers(mEglDisplay,mEglSurface)){
            return 0;
        }
    }

    return -1;
}
