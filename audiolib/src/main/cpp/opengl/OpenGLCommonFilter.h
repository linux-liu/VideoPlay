//
// Created by liuxin on 19-11-23.
//

#ifndef OPENGL_OPENGLCOMMONFILTER_H
#define OPENGL_OPENGLCOMMONFILTER_H


#include "BaseOpenGL.h"
#include "../mylog.h"
#include "../util/ShaderUtils.h"
#include "../util/MatrixUtils.h"

class OpenGLCommonFilter: public BaseOpenGL {

private:
    GLuint textureId=-1;
    GLint sTexture=-1; //纹理
    void * pixels=NULL;

public:
    OpenGLCommonFilter();
    ~OpenGLCommonFilter();
    void onCreate();
    void onChange(int width,int height);
    void onDraw();
    void onDestory();
    void setPixels(void *pixels, int picWidth, int picHeight);

private:
    void setMatriex();
};


#endif //OPENGL_OPENGLCOMMONFILTER_H
