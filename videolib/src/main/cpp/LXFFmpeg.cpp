//
// Created by liuxin on 19-8-24.
//


#include "LXFFmpeg.h"


void *decodeThread(void *data) {
    LXFFmpeg *lxfFmpeg = (LXFFmpeg *) data;
    lxfFmpeg->realPrepare();
    return 0;

}

LXFFmpeg::LXFFmpeg(PlayStatus *status, const char *url, CallJava *callJava) {
    memcpy(this->url, url, 1000);
    this->callJava = callJava;
    this->playStatus = status;
    duration = 0;
    dStartThread = -1;
    dthread = -1;

    pthread_mutex_init(&pthreadMutex, NULL);
    pthread_mutex_init(&pthreadSeek, NULL);

}


void LXFFmpeg::prepare() {
    if (dthread == -1) {
        pthread_create(&dthread, NULL, decodeThread, this);
    }

}

static int interrupt_call_back(void *data) {
    LXFFmpeg *lxfFmpeg = (LXFFmpeg *) data;

    if (lxfFmpeg != NULL && lxfFmpeg->playStatus != NULL) {
        if (lxfFmpeg->playStatus->isExit) {
            return AVERROR_EOF;
        } else {
            return 0;
        }
    } else {
        return AVERROR_EOF;
    }

}


void LXFFmpeg::realPrepare() {

    pthread_mutex_lock(&pthreadMutex);
    callJava->onLoad(ChildThread, true);
    callJava->onPlayStatus(ChildThread, 0);
    avformat_network_init();

    formatContext = avformat_alloc_context();
    formatContext->interrupt_callback.callback = interrupt_call_back;
    formatContext->interrupt_callback.opaque = this;
    int ret = 0;

    if ((ret = avformat_open_input(&formatContext, url, NULL, NULL)) < 0) {
        if (IS_DEBUG) {
            ALOGE("打开输入流失败%s", av_err2str(ret));
        }

        callJava->onError(ChildThread, ERROR_IO, av_err2str(ret));

        pthread_mutex_unlock(&pthreadMutex);
        return;
    }


    if ((ret = avformat_find_stream_info(formatContext, NULL)) < 0) {
        if (IS_DEBUG) {
            ALOGE("找到流信息失败");

        }
        callJava->onError(ChildThread, ERROR_IO_INFO, av_err2str(ret));
        pthread_mutex_unlock(&pthreadMutex);
        return;
    }

    ALOGD("%d stream num",formatContext->nb_streams);


    for(int i = 0; i < formatContext->nb_streams; i++)
    {
        if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)//得到音频流
        {
            if(audio == NULL)
            {

                if (this->audio == NULL) {
                    duration = (int64_t) (formatContext->duration / AV_TIME_BASE);
                    this->audio = new Audio(playStatus, callJava);
                    audio->audioStreamIndex = i;
                    this->audio->duration = duration;
                    this->audio->time_base = formatContext->streams[audio->audioStreamIndex]->time_base;
                    if (callJava != NULL) {
                        callJava->onProgress(ChildThread, (int) this->audio->duration, 0);
                    }

                }


            }
        }
        else if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if(video == NULL)
            {
                this->video = new Video(playStatus, callJava);
                video->videoStreamIndex = i;

                this->video->time_base = formatContext->streams[video->videoStreamIndex]->time_base;
                int num = formatContext->streams[video->videoStreamIndex]->r_frame_rate.num;
                int den = formatContext->streams[video->videoStreamIndex]->r_frame_rate.den;
                if (num != 0 && den != 0) {
                    int fps = num / den;
                    this->video->defaultDelayTime = 1.0 / fps;
                }
            }
        }

    }





    if (audio != NULL) {
        ret = createavcodec(&audio->codecContext, audio->audioStreamIndex);
        if (ret < 0) {
            return;
        }
    }

    if (video != NULL) {
        this->video->audio = audio;
        ret = createavcodec(&video->codecContext, video->videoStreamIndex);
        if (ret < 0) {
            return;
        }
    }


    pthread_mutex_unlock(&pthreadMutex);

    if (callJava) {
        callJava->onPrePare(ChildThread);
    }

}

int LXFFmpeg::createavcodec(AVCodecContext **codecContext, int streamIndex) {
    *codecContext = avcodec_alloc_context3(NULL);
    if (!(*codecContext)) {
        if (IS_DEBUG) {
            ALOGE("分配编码上下文失败");

        }
        callJava->onError(ChildThread, ERROR_CONTEXT_MALLOC_FAIL,
                          "failed to malloc avcodec context");
        pthread_mutex_unlock(&pthreadMutex);
        return -1;
    }

    int ret = 0;

    if ((ret = avcodec_parameters_to_context(*codecContext,
                                             formatContext->streams[streamIndex]->codecpar)) <
        0) {
        if (IS_DEBUG) {
            ALOGE("拷贝参数失败");
        }
        callJava->onError(ChildThread, ERROR_COPY_PARM_FAIL, av_err2str(ret));
        pthread_mutex_unlock(&pthreadMutex);
        return -1;
    }



    AVCodec *avCodec = avcodec_find_decoder(
            formatContext->streams[streamIndex]->codecpar->codec_id);


    if (!avCodec) {
        if (IS_DEBUG) {
            ALOGE("找到解码器失败");
        }

        callJava->onError(ChildThread, ERROR_FIND_DECODE_FAIL, "find decoder error");
        pthread_mutex_unlock(&pthreadMutex);
        return -1;
    }
    if(IS_DEBUG){
        ALOGD("codec name%s",avCodec->name);
    }
  //  AVDictionary *avDictionary=NULL;
  //  av_dict_set_int(&avDictionary,"bufsize",64*1024,0);

    if ((ret = avcodec_open2(*codecContext, avCodec, NULL)) < 0) {
        if (IS_DEBUG) {
            ALOGE("打开解码器失败");
        }
        callJava->onError(ChildThread, ERROR_OPEN_DECODE_FAIL, av_err2str(ret));
        pthread_mutex_unlock(&pthreadMutex);
        return -1;
    }


    return 0;
}


static void *startCallBack(void *data) {
    LXFFmpeg *lxfFmpeg = (LXFFmpeg *) data;
    lxfFmpeg->realStart();
    pthread_exit(&lxfFmpeg->dStartThread);
}


void LXFFmpeg::start() {
    if (playStatus == NULL || playStatus->isExit) {
        if (IS_DEBUG) {
            ALOGE("请先调用prepare方法准备音频流");
        }
        return;
    }

    if (audio == NULL) {
        return;
    }

    if (video == NULL) {
        return;
    }
    if (callJava == NULL) {
        return;
    }
    video->setOpenGLHelper(openGlHelper);

    const char *codecName = video->codecContext->codec->name;

    isSupportHardCodec = callJava->isSupportHardCode(MainThread, codecName);
    ALOGD("isSupportHardCodec==>%d", isSupportHardCodec);
    if (isSupportHardCodec) {
        ALOGD("相等");
        if (strcasecmp(codecName, "h264") == 0) {
            ALOGD("h264");
            bitStreamFilter = av_bsf_get_by_name("h264_mp4toannexb");

        } else if (strcasecmp(codecName, "h265") == 0) {
            bitStreamFilter = av_bsf_get_by_name("hevc_mp4toannexb");
        }

        if (bitStreamFilter == NULL) {
            isSupportHardCodec = false;
            goto end;
        }


        if (av_bsf_alloc(bitStreamFilter, &video->avbsfContext) != 0) {
            isSupportHardCodec = false;
            ALOGD("alloc失敗");
            goto end;
        }


        if (avcodec_parameters_copy(video->avbsfContext->par_in,
                                    formatContext->streams[video->videoStreamIndex]->codecpar) <
            0) {
            isSupportHardCodec = false;
            av_bsf_free(&video->avbsfContext);
            video->avbsfContext = NULL;
            goto end;
        }
        if (av_bsf_init(video->avbsfContext) != 0) {
            isSupportHardCodec = false;
            av_bsf_free(&video->avbsfContext);
            video->avbsfContext = NULL;
            goto end;
        }

        video->avbsfContext->time_base_in = video->time_base;


    }
    end:

    if (isSupportHardCodec) {
        video->decodeType = HARD_DECODE;
        if (callJava != NULL) {
            callJava->onMediaCodecInit(MainThread, codecName, video->codecContext->width,
                                       video->codecContext->height,
                                       video->codecContext->extradata,
                                       video->codecContext->extradata_size,
                                       video->codecContext->extradata,
                                       video->codecContext->extradata_size);
        }
    } else {

        video->decodeType = SOTT_DECODE;
    }


    if (audio != NULL) {
        audio->resample();
    }
    if (video != NULL) {

        video->start();
    }

    if (dStartThread == -1) {
        pthread_create(&dStartThread, NULL, startCallBack, this);
    }


}

void LXFFmpeg::realStart() {


    pthread_mutex_lock(&pthreadMutex);
    while (playStatus != NULL && !playStatus->isExit) {


        if (playStatus->isSeek) {
            usleep(1000 * 100);
            continue;
        }

        //最多缓存100个
        if (audio->avPacketQuene->getSize() > 120) {

            usleep(1000 * 100);
            continue;
        }

        pthread_mutex_lock(&pthreadSeek);
        AVPacket *avPacket = av_packet_alloc();


        int ret = av_read_frame(formatContext, avPacket);

        if (ret == 0) {

            if (avPacket->stream_index == audio->audioStreamIndex) {

                audio->avPacketQuene->pushQueue(avPacket);
            } else if (avPacket->stream_index == video->videoStreamIndex) {

                video->avPacketQuene->pushQueue(avPacket);
            } else {
                if (avPacket) {
                    av_packet_free(&avPacket);
                    av_freep(&avPacket);
                    avPacket = NULL;
                }
            }

        } else {
            if (avPacket) {
                av_packet_free(&avPacket);
                av_freep(&avPacket);
                avPacket = NULL;
            }

            while (playStatus != NULL && !playStatus->isExit) {
                if (audio->avPacketQuene->getSize() > 0) {
                    usleep(1000 * 100);
                    continue;
                } else {
                    if (!playStatus->isSeek) {
                        if (IS_DEBUG) {
                            ALOGD("退出了");
                        }
                        usleep(1000 * 100);
                        playStatus->isExit = true;
                    }
                    break;
                }
            }


        }
        pthread_mutex_unlock(&pthreadSeek);
    }

    pthread_mutex_unlock(&pthreadMutex);

    if (playStatus != NULL) {
        playStatus->isExit = true;
    }

    if (callJava != NULL) {

        callJava->onLoad(ChildThread, false);
        callJava->onComplete(ChildThread);
    }


}


LXFFmpeg::~LXFFmpeg() {
    pthread_mutex_destroy(&pthreadMutex);
    pthread_mutex_destroy(&pthreadSeek);


}


void LXFFmpeg::play() {
    if (audio != NULL) {
        audio->play();
    }
    if (video != NULL) {
        video->play();
    }
}

void LXFFmpeg::pause() {
    if (audio != NULL) {
        audio->pause();

    }
    if (video != NULL) {
        video->pause();
    }
}

void LXFFmpeg::release() {
    if (IS_DEBUG) {
        ALOGD("开始释放资源了");
    }


    playStatus->isExit = true;
    if (dthread != -1) {
        pthread_join(dthread, NULL);
        dthread = -1;
    }
    if (dStartThread != -1) {
        pthread_join(dStartThread, NULL);
        dStartThread = -1;
    }


    int try_count = 100000;
    int ret = -1;

    while (try_count) {
        ret = pthread_mutex_trylock(&pthreadMutex);
        if (ret == 0) {
            ALOGD("获取到锁11");
            break;
        }
        try_count--;
        usleep(1000 * 10);
    }

    ALOGD("开始释放资源了11");

    if (audio != NULL) {
        audio->release();
        delete audio;
        audio = NULL;
    }
    if (video != NULL) {
        video->release();
        delete video;
        video = NULL;
    }
    ALOGD("video释放完成");
    if (callJava != NULL) {
        callJava->onProgress(ChildThread, 0, 0);
    }
    callJava = NULL;
    playStatus = NULL;


    if (formatContext) {
        avformat_close_input(&formatContext);
        avformat_free_context(formatContext);
        av_freep(&formatContext);
        formatContext = NULL;
    }
    avformat_network_deinit();

    if (openGlHelper != NULL) {
        openGlHelper = NULL;
    }


    pthread_mutex_unlock(&pthreadMutex);


}

void LXFFmpeg::seek(int sec) {

    if (duration < 0) {
        return;
    }

    if (sec < 0 || sec > duration) {
        return;
    }

    playStatus->isSeek = true;
    pthread_mutex_lock(&pthreadSeek);
    if (IS_DEBUG) {
        ALOGD("sec=>%d", sec);
    }
    auto seekTime = sec * AV_TIME_BASE;
    //-1默认的流被选择，以AVTIME_BASE;
    // int ret_=avformat_seek_file(formatContext, -1, INT64_MIN, seekTime, INT64_MAX, 0);
    int ret_ = av_seek_frame(formatContext, -1, seekTime, 0);
    if (IS_DEBUG) {
        ALOGD("seek ret==%d", ret_);
    }
    if (audio != NULL) {
        audio->avPacketQuene->clearQuene();
        audio->current = 0;
        audio->pre_time = 0;
        audio->isFinishReceive = true;
        //秒转化成时间搓
        auto realSeek = static_cast<int64_t>(sec /
                                             av_q2d(formatContext->streams[audio->audioStreamIndex]->time_base));

        pthread_mutex_lock(&audio->decodeMutex);
        avcodec_flush_buffers(audio->codecContext);

        pthread_mutex_unlock(&audio->decodeMutex);

    }

    if (video != NULL) {
        video->avPacketQuene->clearQuene();
        video->current = 0;
        pthread_mutex_lock(&video->decodeMutex);
        avcodec_flush_buffers(video->codecContext);
        pthread_mutex_unlock(&video->decodeMutex);

    }

    pthread_mutex_unlock(&pthreadSeek);
    playStatus->isSeek = false;
    if (IS_DEBUG) {
        ALOGD("seek完成了");
    }


}

int LXFFmpeg::getDuration() {
    if (audio != NULL) {
        return (int) audio->duration;
    }
    return 0;
}

void LXFFmpeg::setVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    if (audio != NULL) {
        audio->setVolume(volume);
    }
}

void LXFFmpeg::setMute(bool mute) {
    if (audio != NULL) {
        audio->setMute(mute);
    }

}

void LXFFmpeg::setChannelSolo(int channel) {
    if (audio != NULL) {
        audio->setChannelSolo(channel);
    }
}

void LXFFmpeg::setPitch(double pitch) {
    if (audio != NULL) {
        audio->setPitch(pitch);
    }

}

void LXFFmpeg::setTempPo(double temPo) {
    if (audio != NULL) {
        audio->setTempPo(temPo);
    }
}

void LXFFmpeg::setOpenGLHelper(OpenGLHelper *helper) {
    this->openGlHelper = helper;
}








