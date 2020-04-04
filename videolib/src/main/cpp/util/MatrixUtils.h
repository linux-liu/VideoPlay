//
// Created by liuxin on 19-11-20.
//

#ifndef OPENGL_MATRIXUTILS_H
#define OPENGL_MATRIXUTILS_H


#include "math.h"

static void init(float *matrixs) {
    for (int i = 0; i < 16; i++) {
        if (i % 5 == 0) {
            matrixs[i] = 1;
        } else {
            matrixs[i] = 0;
        }
    }
}

static void rotateX(double angle, float *matrixs) {
    double f_angle = angle * (M_PI / 180);
    matrixs[0] = 1;
    matrixs[5] = cos(f_angle);

    matrixs[6] = -sin(f_angle);
    matrixs[9] = sin(f_angle);
    matrixs[10] = cos(f_angle);
}

static void rotateY(double angle, float *matrixs) {
    double f_angle = angle * (M_PI / 180);
    matrixs[0] = cos(f_angle);
    matrixs[2] = sin(f_angle);

    matrixs[8] =-sin(f_angle);
    matrixs[10] = cos(f_angle);
}

static void rotateZ(double angle, float *matrixs) {
    double f_angle = angle * (M_PI / 180);
    matrixs[0] = cos(f_angle);
    matrixs[1] = -sin(f_angle);

    matrixs[4] = sin(f_angle);
    matrixs[5] = cos(f_angle);
}

static void scale(double scale, float *matrixs) {
    matrixs[0] = scale;//缩放x轴
    matrixs[5] = scale;//缩放y轴


}

static void translate(double transX, double transY, float *matrixs) {
    matrixs[3] = transX;
    matrixs[7] = transY;
}

static void projection(double left, double right, double top, double bottom, float *matrixs) {
    matrixs[0] = 2.0 / (right - left);
    matrixs[3] = -(right + left) / (right - left);
    matrixs[5] = 2.0 / (top - bottom);
    matrixs[7] = -(top + bottom) / (top - bottom);

}

static void quareMult(float *matrix1, float *matrix2, float *out) {

    int i = 0, j = 0, m = 0;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            float sum = 0;
            for (m = 0; m < 4; m++) {
                sum = sum + matrix1[i*4+m] * matrix2[m*4+j];
            }
            out[i*4+j] = sum;
        }
    }

    for(int t=0;t<16;t++){
        ALOGE("matrix%f",out[t]);
    }


}


#endif //OPENGL_MATRIXUTILS_H
