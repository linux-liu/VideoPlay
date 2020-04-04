//
// Created by liuxin on 19-11-30.
//

#include "Video.h"

Video::Video(PlayStatus *status, CallJava *callJava) {
    this->playStatus = status;
    this->callJava = callJava;
    this->avPacketQuene = new AvPacketQuene();


    decodeType = SOTT_DECODE;
    decode_thread = -1;
    pthread_mutex_init(&decodeMutex, NULL);

}


static void *decode_thread_call_back(void *data) {
    Video *video = (Video *) data;
    while (video->playStatus != NULL && !video->playStatus->isExit) {

        if (video->playStatus->isSeek) {
            video->callJava->onLoad(ChildThread, true);
            usleep(1000 * 100);
            continue;
        }

        if (video->isPause) {
            usleep(1000 * 100);
            continue;
        }
        if (video->avPacketQuene->getSize() == 0) {
            video->callJava->onLoad(ChildThread, true);

            usleep(1000 * 100);
            continue;
        }


        video->callJava->onLoad(ChildThread, false);
        AVPacket *avPacket = av_packet_alloc();
        if (video->avPacketQuene->pullQueue(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_freep(&avPacket);
            avPacket = NULL;
            continue;
        }

        if (video->decodeType == HARD_DECODE) {
            if (av_bsf_send_packet(video->avbsfContext, avPacket) != 0) {
                av_packet_free(&avPacket);
                av_freep(&avPacket);
                avPacket = NULL;
                continue;
            }

            while (av_bsf_receive_packet(video->avbsfContext, avPacket) == 0) {
                ALOGD("开始硬解码");
                double diff = video->getDifferTimetamp(NULL, avPacket);
                usleep(video->getDelayTime(diff) * 1000000);
                video->callJava->onMediaCodeDecode(ChildThread, avPacket->data, avPacket->size);
                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }

            avPacket = NULL;
        } else {
            pthread_mutex_lock(&video->decodeMutex);

             if (avcodec_send_packet(video->codecContext, avPacket) != 0) {
                   if (IS_DEBUG) {
                       ALOGE("发送视频帧失败");
                   }
                   av_packet_free(&avPacket);
                   av_freep(&avPacket);
                   avPacket = NULL;
                   pthread_mutex_unlock(&video->decodeMutex);
                   continue;
               }


               AVFrame *avFrame = av_frame_alloc();

               if (avcodec_receive_frame(video->codecContext, avFrame) != 0) {
                   if (IS_DEBUG) {
                             ALOGE("接收视频帧失败");
                   }
                   av_frame_free(&avFrame);
                   av_freep(&avFrame);
                   avFrame = NULL;

                   av_packet_free(&avPacket);
                   av_freep(&avPacket);
                   avPacket = NULL;
                   pthread_mutex_unlock(&video->decodeMutex);
                   continue;
               }


            if (IS_DEBUG) {
                ALOGD("avformat=>%d", avFrame->format);
                ALOGD("解码视频帧成功");
            }
            if (avFrame->format == AV_PIX_FMT_YUV420P) {

                if (video->openGlHelper != NULL) {
                    double diff = video->getDifferTimetamp(avFrame, NULL);
                    usleep(video->getDelayTime(diff) * 1000000);

                    video->openGlHelper->setYUVPixels(avFrame->data[0], avFrame->data[1],
                                                      avFrame->data[2],
                                                      avFrame->linesize[0],
                                                      avFrame->height);
                }

            } else {

                AVFrame *destFrame = av_frame_alloc();


                int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                          video->codecContext->width,
                                                          video->codecContext->height, 16);

                auto buffer = static_cast<uint8_t *>(av_malloc(bufferSize * sizeof(uint8_t)));

                av_image_fill_arrays(destFrame->data, destFrame->linesize, buffer,
                                     AV_PIX_FMT_YUV420P,
                                     video->codecContext->width, video->codecContext->height,
                                     16);

                SwsContext *swsContext = sws_getContext(video->codecContext->width,
                                                        video->codecContext->height,
                                                        video->codecContext->pix_fmt,
                                                        video->codecContext->width,
                                                        video->codecContext->height,
                                                        AV_PIX_FMT_YUV420P,
                                                        SWS_BILINEAR,
                                                        NULL,
                                                        NULL,
                                                        NULL);


                sws_scale(swsContext, avFrame->data, avFrame->linesize, 0,
                          video->codecContext->height, destFrame->data, destFrame->linesize);

                if (video->openGlHelper != NULL) {
                    double diff = video->getDifferTimetamp(avFrame, NULL);
                    usleep(video->getDelayTime(diff) * 1000000);
                    video->openGlHelper->setYUVPixels(destFrame->data[0], destFrame->data[1],
                                                      destFrame->data[2],
                                                      video->codecContext->width,
                                                      video->codecContext->height);

                }

                sws_freeContext(swsContext);
                swsContext = NULL;
                av_freep(&buffer);
                buffer = NULL;
                av_frame_free(&destFrame);
                av_freep(&destFrame);
                destFrame = NULL;
            }

            av_frame_free(&avFrame);
            av_freep(&avFrame);
            avFrame = NULL;
            av_packet_free(&avPacket);
            av_freep(&avPacket);
            avPacket = NULL;
            pthread_mutex_unlock(&video->decodeMutex);

        }


    }
    ALOGE("销毁解码线程");

    return 0;
}

void Video::start() {
    if (decode_thread == -1)
        pthread_create(&decode_thread, NULL, decode_thread_call_back, this);

}

void Video::release() {
    playStatus->isExit = true;
    if (decode_thread != -1) {
        pthread_join(decode_thread, NULL);
    }

    decode_thread = -1;


    if (avPacketQuene) {
        avPacketQuene->clearQuene();
    }
    if (avPacketQuene) {
        delete avPacketQuene;
        avPacketQuene = NULL;
    }

    if (avbsfContext) {
        ALOGE("avbsContext 释放");
        av_bsf_free(&avbsfContext);
        avbsfContext = NULL;
    }

    if (codecContext) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        av_freep(&codecContext);
        codecContext = NULL;
        ALOGE("释放codecContext");
    }
    if (openGlHelper != NULL) {

        this->openGlHelper = NULL;
    }


    if (callJava) {
        callJava = NULL;
    }
    if (playStatus) {
        playStatus = NULL;
    }
    if (audio) {
        audio = NULL;
    }
}

Video::~Video() {

    pthread_mutex_unlock(&decodeMutex);

}

double Video::getDifferTimetamp(AVFrame *avFrame, AVPacket *avPacket) {

    double pts = 0;

    if (avFrame != NULL) {
        pts = avFrame->pts;
    }

    if (avPacket != NULL) {
        pts = avPacket->pts;
    }

    if (pts == AV_NOPTS_VALUE) {
        pts = 0;
    }

    //ALOGE("avstream timebase %d,%d",time_base.num,time_base.den);
    //ALOGE("avcode timebase %d,%d",codecContext->time_base.num,codecContext->time_base.den);
    pts = pts * av_q2d(time_base);
    if (pts > 0) {
        current = pts;
    }

    double diff = audio->current - current;

    return diff;
}

double Video::getDelayTime(double diff) {

    //  ALOGE("delayTime=>%lf,defaultDelayTime=>%lf",delayTime,defaultDelayTime);
    //音频较快
    if (diff > 0.003) {
        delayTime = delayTime * 2 / 3;

        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }


    } else if (diff < -0.003) {
        delayTime = delayTime * 3 / 2;
        if (delayTime < defaultDelayTime / 2) {
            delayTime = defaultDelayTime * 2 / 3;
        } else if (delayTime > defaultDelayTime * 2) {
            delayTime = defaultDelayTime * 2;
        }
    }


    if (diff >= 0.5) {
        delayTime = 0;
    } else if (diff <= -0.5) {
        delayTime = defaultDelayTime * 2;
    }

    if (fabs(diff) >= 10) {
        delayTime = defaultDelayTime;
    }
    return delayTime;


}

void Video::setOpenGLHelper(OpenGLHelper *helper) {
    this->openGlHelper = helper;
}

void Video::pause() {
    isPause = true;

}

void Video::play() {
    isPause = false;

}











