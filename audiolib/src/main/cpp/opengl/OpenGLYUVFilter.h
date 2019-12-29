//
// Created by liuxin on 19-11-24.
//

#ifndef OPENGL_OPENGLYUVFILTER_H
#define OPENGL_OPENGLYUVFILTER_H

#include "BaseOpenGL.h"
#include "../mylog.h"
#include "../util/ShaderUtils.h"
#include "../util/MatrixUtils.h"
#include "OpenGLFbo.h"
#include <pthread.h>
class OpenGLYUVFilter: public BaseOpenGL {
private:
    GLuint textureIds[3]={0};
    GLint sampler_y=-1; //纹理
    GLint sampler_u=-1; //纹理
    GLint sampler_v=-1; //纹理
    void * y=NULL;
    void *u=NULL;
    void *v=NULL;
    OpenGLFbo *openGlFbo=NULL;

    GLuint fboTextureId=0;




public:
    OpenGLYUVFilter();
    ~OpenGLYUVFilter();
    void onCreate();
    void onChange(int width,int height);
    void onDraw();
    void onDestory();

    void setYUVPixels(void *Y, void *U, void *V,int picWidth,int picHeight);

private:
    void setMatriex();
};


#endif //OPENGL_OPENGLYUVFILTER_H
