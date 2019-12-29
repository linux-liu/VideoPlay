//
// Created by liuxin on 19-11-24.
//

#include <EGL/egl.h>

#include "OpenGLYUVFilter.h"

OpenGLYUVFilter::OpenGLYUVFilter() {
    vertex = "attribute vec4 v_Position;\n"
             "attribute vec2 f_Position;\n"
             "varying vec2 ft_Position;\n"
             "uniform mat4 u_Matrix;\n"
             "void main() {\n"
             "    ft_Position = f_Position;\n"
             "    gl_Position = v_Position*u_Matrix;\n"
             "}";


    fragment = "precision mediump float;\n"
               "varying vec2 ft_Position;\n"
               "uniform sampler2D sampler_y;\n"
               "uniform sampler2D sampler_u;\n"
               "uniform sampler2D sampler_v;\n"
               "void main() {\n"
               "float y,u,v;\n"
               "y = texture2D(sampler_y,ft_Position).r;\n"
               "u = texture2D(sampler_u,ft_Position).r- 0.5;\n"
               "v = texture2D(sampler_v,ft_Position).r- 0.5;\n"
               "vec3 rgb;\n"
               "rgb.r = y + 1.403 * v;\n"
               "rgb.g = y - 0.344 * u - 0.714 * v;\n"
               "rgb.b = y + 1.770 * u;\n"
               "gl_FragColor = vec4(rgb,1);\n"
               "}";




    openGlFbo = new OpenGLFbo();
}


void OpenGLYUVFilter::onCreate() {
      openGlFbo->onCreate();

    if (IS_DEBUG) {
        ALOGE("callbackSurfaceCreate");
    }
    program = createProgram(vertex, fragment, &vertexShader, &fragmentShader);
    if (IS_DEBUG) {
        ALOGE("program%d", program);
    }


    vPosition = glGetAttribLocation(program, "v_Position");
    fPosition = glGetAttribLocation(program, "f_Position");

    sampler_y = glGetUniformLocation(program, "sampler_y");
    sampler_u = glGetUniformLocation(program, "sampler_u");
    sampler_v = glGetUniformLocation(program, "sampler_v");

    uMatrixs = glGetUniformLocation(program, "u_Matrix");
    if (IS_DEBUG) {
        ALOGD("v=>%d u=>%d,v=>%d", sampler_y, sampler_u, sampler_v);
    }

    //获取顶点缓冲对象


    glGenBuffers(1, &vbo);

    //绑定
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertexSize + fragmentSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, vertexs);
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize, fragmentSize, fragments);

    //解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    //获取纹理绑定纹理
    glGenTextures(3, textureIds);

    for (int i = 0; i < 3; i++) {


        glBindTexture(GL_TEXTURE_2D, textureIds[i]);

        if (IS_DEBUG) {
            ALOGI("textureId=>%d", textureIds[i]);
        }
        //设置参数
        //填充方式x y轴
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        //缩放方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);


    }



    if (IS_DEBUG) {
        ALOGI("AttribLacation%d", vPosition);
    }
}

void OpenGLYUVFilter::onChange(int width, int height) {
    glViewport(0, 0, width, height);
    if (IS_DEBUG) {
        ALOGE("onChange w=>%d h=>%d pw=>%d,ph=>%d", width, height, pic_width, pic_height);

    }
    this->width = width;
    this->height = height;
    glGenBuffers(1, &fbo);
    //创建纹理缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1,&fboTextureId);
    //激活设置纹理,要一致

    glBindTexture(GL_TEXTURE_2D,fboTextureId);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    openGlFbo->setTextureId(fboTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);



    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextureId, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        ALOGE("出错了绑定");
    } else {
        ALOGE("绑定成功");
    }
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    openGlFbo->onChange(width, height);
    setMatriex();
}

void OpenGLYUVFilter::onDraw() {

   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   ALOGE("绘制中");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    //一定要写
    glUseProgram(program);


    //矩阵变换关联
    glUniformMatrix4fv(uMatrixs, 1, GL_FALSE, matriex);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //反射使参数可用
    glEnableVertexAttribArray(vPosition);

    glVertexAttribPointer(vPosition, 2, GL_FLOAT, false, 8, 0);


    glEnableVertexAttribArray(fPosition);
    glVertexAttribPointer(fPosition, 2, GL_FLOAT, false, 8,
                          reinterpret_cast<const void *>(vertexSize));

    ALOGE("绘制5");
    if (pic_height > 0 && pic_width > 0) {

        if (y != NULL) {
            //激活设置纹理,要一致0
            glActiveTexture(GL_TEXTURE0);
            //重新绑定
            glBindTexture(GL_TEXTURE_2D, textureIds[0]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, pic_width, pic_height, 0, GL_LUMINANCE,
                         GL_UNSIGNED_BYTE, y);

            glUniform1i(sampler_y, 0);
            ALOGE("绘制10");
        }
        if (u != NULL) {
            //激活设置纹理,要一致1

           glActiveTexture(GL_TEXTURE1);
            //重新绑定
            glBindTexture(GL_TEXTURE_2D, textureIds[1]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, pic_width / 2, pic_height / 2, 0,
                         GL_LUMINANCE, GL_UNSIGNED_BYTE, u);

           glUniform1i(sampler_u, 1);
            ALOGE("绘制15");
        }

        if (v != NULL) {

            //激活设置纹理,要一致2
            glActiveTexture(GL_TEXTURE2);
            //重新绑定
            glBindTexture(GL_TEXTURE_2D, textureIds[2]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, pic_width / 2, pic_height / 2, 0,
                         GL_LUMINANCE, GL_UNSIGNED_BYTE, v);
            glUniform1i(sampler_v, 2);
            ALOGE("绘制20");
        }


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //解除绑定
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        ALOGE("绘制25");


    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    openGlFbo->onDraw();


}

void OpenGLYUVFilter::setYUVPixels(void *Y, void *U, void *V, int picWidth, int picHeight) {


    this->y = Y;
    this->u = U;
    this->v = V;
    this->pic_width = picWidth;
    this->pic_height = picHeight;
    if (picWidth > 0 && picHeight > 0) {
        setMatriex();
    }

}

void OpenGLYUVFilter::setMatriex() {
    float rotateM[16]={0};
    init(rotateM);

    rotateX(180,rotateM);

    float projectM[16]={0};
    init(projectM);

    if (pic_width > 0 && pic_height > 0) {
        double screen_r = 1.0 * width / height;
        double picture_r = 1.0 * pic_width / pic_height;
        if (screen_r < picture_r) {
            //高度缩放
            double r = height / (1.0 * width / pic_width * pic_height);
            projection(-1, 1, r, -r, projectM);

        } else {
            //宽度缩放
            double r = width / (1.0 * height / pic_height * pic_width);

            projection(-r, r, 1, -1, projectM);
        }
    }
    init(matriex);

    //矩阵相乘
    quareMult(rotateM,projectM,matriex);

}

void OpenGLYUVFilter::onDestory() {
    ALOGE("vbo=>%d,fbo=>%d",vbo,fbo);
  if(vbo>0){
       glDeleteBuffers(1, &vbo);
   }
      if(fbo>0){
          glDeleteBuffers(1,&fbo);
      }
    if (textureIds[0] > 0)
        glDeleteTextures(3, textureIds);

    if(fboTextureId>0){
       glDeleteTextures(1,&fboTextureId);
    }
    if(openGlFbo){
        openGlFbo->onDestory();
        delete openGlFbo;
        openGlFbo=NULL;
    }

    if (program > 0) {
        glDetachShader(program, vertexShader);
        glDeleteShader(vertexShader);

        glDetachShader(program, fragmentShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        vertexShader = -1;
        fragmentShader = -1;
        program = -1;
    }
}

OpenGLYUVFilter::~OpenGLYUVFilter() {
    this->y = NULL;
    this->u = NULL;
    this->v = NULL;

}



