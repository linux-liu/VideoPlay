//
// Created by liuxin on 19-11-23.
//


#include "BaseOpenGL.h"


BaseOpenGL::BaseOpenGL() {
    if(IS_DEBUG){
        ALOGD("BaseOpenGL");
    }
    program=-1;
    vertexs=new float[8];
    fragments=new float[8];
    //参考顶点坐标系
    float tempVertexs[8] = {
            1.0f,-1.0f,
            1.0f,1.0f,
            -1.0f,-1.0f,
            -1.0f,1.0f
    };

    vertexSize= sizeof(tempVertexs);
    memcpy(vertexs,tempVertexs, vertexSize);



    float tempFragments[8] ={
            1.0f,1.0f,
            1.0f,0.0f,
            0.0f,1.0f,
            0.0f,0.0f
    };
    fragmentSize=sizeof(tempFragments);
    memcpy(fragments,tempFragments, fragmentSize);


}

BaseOpenGL::~BaseOpenGL() {
    if(IS_DEBUG){
        ALOGD("~BaseOpenGL");
    }
     delete []vertexs;
     delete []fragments;
}

void BaseOpenGL::setPixels(void *pixels, int picWidth, int picHeight) {

}

void BaseOpenGL::setYUVPixels(void *Y, void *U, void *V, int picWidth, int picHeight) {

}



