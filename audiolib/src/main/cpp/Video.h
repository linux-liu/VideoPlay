//
// Created by liuxin on 19-11-30.
//

#ifndef VIDEOPLAY_VIDEO_H
#define VIDEOPLAY_VIDEO_H
extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
};
#include "AvPacketQuene.h"
#include "PlayStatus.h"
#include "CallJava.h"
#include <unistd.h>
#include "opengl/OpenGLHelper.h"
#include "Audio.h"


#define SOTT_DECODE 0
#define HARD_DECODE 1
class Video {

public:
    int videoStreamIndex=-1;
    AVCodecContext *codecContext = NULL;
    AvPacketQuene *avPacketQuene=NULL;
    PlayStatus *playStatus=NULL;
    pthread_t decode_thread=-1;
    CallJava *callJava=NULL;

    Audio *audio=NULL;
    AVRational time_base;
    double  current=0;
    double  delayTime=0;
    double  defaultDelayTime=0;

    //解码的锁
    pthread_mutex_t decodeMutex;


    AVBSFContext *avbsfContext=NULL;

    int decodeType=SOTT_DECODE;

    OpenGLHelper *openGlHelper=NULL;

    bool  isPause= false;





public:
    Video(PlayStatus *status,CallJava *callJava);

    void  setOpenGLHelper(OpenGLHelper *helper);

    double getDifferTimetamp(AVFrame *avFrame,AVPacket *avPacket);

    double getDelayTime(double diff);
    void start();

    void pause();
    void release();
    void  play();
    ~Video();
};


#endif //VIDEOPLAY_VIDEO_H
