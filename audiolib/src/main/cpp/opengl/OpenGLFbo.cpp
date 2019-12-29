//
// Created by liuxin on 19-12-15.
//

#include "OpenGLFbo.h"

OpenGLFbo::OpenGLFbo() {
    vertex = "attribute vec4 v_Position;\n"
             "attribute vec2 f_Position;\n"
             "varying vec2 ft_Position;\n"
             "void main() {\n"
             "    ft_Position = f_Position;\n"
             "    gl_Position = v_Position;\n"
             "}";


    fragment = "precision mediump float;\n"
               "varying vec2 ft_Position;\n"
               "uniform sampler2D sTexture;\n"
               "void main() {\n"
               "    gl_FragColor=texture2D(sTexture, ft_Position);\n"
               "}";
}

OpenGLFbo::~OpenGLFbo() {

}

void OpenGLFbo::onCreate() {

    if (IS_DEBUG) {
        ALOGI("callbackSurfaceCreate");
    }
    program = createProgram(vertex, fragment, &vertexShader, &fragmentShader);
    if (IS_DEBUG) {
        ALOGI("program%d", program);
    }

    vPosition = glGetAttribLocation(program, "v_Position");
    fPosition = glGetAttribLocation(program, "f_Position");
    sTexture = glGetUniformLocation(program, "sTexture");

    //获取顶点缓冲对象


    glGenBuffers(1, &vbo);

    //绑定
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertexSize + fragmentSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, vertexs);
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize, fragmentSize, fragments);

    //解绑
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLFbo::onChange(int width, int height) {
    this->width=width;
    this->height=height;
    glViewport(0, 0, width, height);
}

void OpenGLFbo::onDraw() {
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //一定要写
    glUseProgram(program);



    //重新绑定

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //反射使参数可用
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition,2,GL_FLOAT, false,8,0);

    glEnableVertexAttribArray(fPosition);
    glVertexAttribPointer(fPosition, 2, GL_FLOAT, false, 8,
                          reinterpret_cast<const void *>(vertexSize));


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,textureId);
    glUniform1i(sTexture,0);


    glDrawArrays(GL_TRIANGLE_STRIP,0,4);



    glBindBuffer(GL_ARRAY_BUFFER, 0);
    //解除绑定
    glBindTexture(GL_TEXTURE_2D,0);
}

void OpenGLFbo::onDestory() {
    glDeleteBuffers(1, &vbo);
    if(program>0){
        glDetachShader(program,vertexShader);
        glDeleteShader(vertexShader);

        glDetachShader(program,fragmentShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
        vertexShader=-1;
        fragmentShader=-1;
        program=-1;
    }
}

void OpenGLFbo::setTextureId(GLuint textureId) {
    this->textureId = textureId;

}

GLint OpenGLFbo::getTexture() {
    return sTexture;
}


