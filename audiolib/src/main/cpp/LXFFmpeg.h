//
// Created by liuxin on 19-8-24.
//

#ifndef AUDIOPLAY_LXFFMPEG_H
#define AUDIOPLAY_LXFFMPEG_H


#include "CallJava.h"
#include "mylog.h"
#include "Audio.h"
#include <pthread.h>
#include "AvPacketQuene.h"
#include "PlayStatus.h"
#include <unistd.h>
#include "Video.h"
#include <pthread.h>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
};

class LXFFmpeg {

public:
    CallJava *callJava = NULL;
    PlayStatus *playStatus = NULL;
    Audio *audio = NULL;
    Video *video = NULL;

    AVFormatContext *formatContext = NULL;
    pthread_t dthread=-1;
    pthread_mutex_t pthreadMutex;


    pthread_mutex_t pthreadSeek;

    pthread_t dStartThread=-1;
    int64_t duration;

    //硬编码相关的
    bool isSupportHardCodec = false;
    const AVBitStreamFilter *bitStreamFilter=NULL;


    char url[1000];

    OpenGLHelper *openGlHelper=NULL;




public:
    LXFFmpeg(PlayStatus *status, const char *url, CallJava *callJava);

    void setOpenGLHelper(OpenGLHelper *helper);

    void prepare();

    void start();

    void realPrepare();

    void realStart();

    int createavcodec(AVCodecContext **pCodecContext, int streamIndex);


    void play();

    void pause();

    void seek(int sec);

    void release();

    int getDuration();

    void setVolume(int volume);

    void setMute(bool mute);

    void setChannelSolo(int channel);

    void setPitch(double pitch);

    void setTempPo(double temPo);


    ~LXFFmpeg();
};


#endif //AUDIOPLAY_LXFFMPEG_H
