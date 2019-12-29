//
// Created by liuxin on 19-12-15.
//

#ifndef VIDEOPLAY_OPENGLFBO_H
#define VIDEOPLAY_OPENGLFBO_H


#include "BaseOpenGL.h"
#include "../mylog.h"
#include "../util/ShaderUtils.h"
/**
 * FBO帧缓冲实际绘制的类
 */
class OpenGLFbo : BaseOpenGL{

private:
    GLuint textureId=-1;
    GLint sTexture=-1; //纹理

public:
    OpenGLFbo();
    ~OpenGLFbo();

    void setTextureId(GLuint textureId);

    GLint getTexture();
    void onCreate();
    void onChange(int width,int height);
    void onDraw();
    void onDestory();

};


#endif //VIDEOPLAY_OPENGLFBO_H
