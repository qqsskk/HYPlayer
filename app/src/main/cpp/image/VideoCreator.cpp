//
// Created by templechen on 2019-09-06.
//

#include <pthread.h>
#include "VideoCreator.h"
#include "../base/utils.h"
#include "../base/native_log.h"

VideoCreator::VideoCreator(const char *path) {
    this->path = path;
    sendMessage(kMsgVideoCreatorStart);
}

VideoCreator::~VideoCreator() {

}

AVFrame *VideoCreator::readFrame(int index) {
    double currentTime = index * 16.6667;
    if (totalMs == 0 || size == 0) {
        return nullptr;
    }
    auto currentIndex = (unsigned int) (currentTime / (totalMs / size)) % size;
    if (currentIndex < frameList.size()) {
//        ALOGD("currentIndex %d", currentIndex);
        return frameList.at(currentIndex);
    }
    return nullptr;
}

void VideoCreator::releaseFrame() {
    isDecoding = false;
    sendMessage(kMsgVideoCreatorStop);
    quit();
}

void VideoCreator::startDecode() {
    long start = javaTimeMillis();
    ic = avformat_alloc_context();
    int re = avformat_open_input(&ic, path, nullptr, nullptr);
    if (re != 0) {
        char buf[1024] = {0};
        av_strerror(re, buf, sizeof(buf));
        ALOGE("open video failed %s", buf);
        return;
    }
    ALOGD("open video success");
    if (ic->nb_streams < 1) {
        ALOGE("no video stream %d", ic->nb_streams);
        return;
    }
    re = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (re < 0) {
        char buf[1024] = {0};
        av_strerror(re, buf, sizeof(buf));
        ALOGD("av_find_best_stream failed %s", buf);
        return;
    }
    videoIndex = re;
    AVCodecParameters *codecParameters = ic->streams[videoIndex]->codecpar;
    if (ic->streams[videoIndex]->duration > 0) {
        totalMs =
                ic->streams[videoIndex]->duration * r2d(ic->streams[videoIndex]->time_base) *
                1000LL;
        size = (int) ic->streams[videoIndex]->nb_frames;
        timeBase = r2d(ic->streams[videoIndex]->time_base);
        ALOGD("video total duration is %lld %d", totalMs, size);
    }
    AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
    if (codec == nullptr) {
        ALOGE("no codec for video!");
        return;
    }
    ALOGD("video find codec success");
    codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, codecParameters);
    re = avcodec_open2(codecContext, codec, nullptr);
    if (re != 0) {
        ALOGE("video codec open error %s", path);
        return;
    }
    ALOGD("video codec open success");
    int count = 0;
    pkt = av_packet_alloc();
    while (isDecoding && av_read_frame(ic, pkt) == 0) {
        if (pkt->stream_index != videoIndex) {
            continue;
        }
        re = avcodec_send_packet(codecContext, pkt);
        if (re != 0) {
            char buf[1024] = {0};
            av_strerror(re, buf, sizeof(buf));
            ALOGE("video send packet fail %s", buf);
            return;
        }
        while (isDecoding) {
            AVFrame *pFrameYUV = av_frame_alloc();
            re = avcodec_receive_frame(codecContext, pFrameYUV);
            if (re == 0) {
                pFrameYUV->width = codecContext->width;
                pFrameYUV->height = codecContext->height;
                ALOGD("push avFrame")
                frameList.push_back(pFrameYUV);
                count++;
            } else {
                av_frame_free(&pFrameYUV);
                break;
            }
        }


    }
    ALOGD("read all video %ld %d", javaTimeMillis() - start, count);
}

void VideoCreator::handleMessage(Looper::LooperMessage *msg) {
    switch (msg->what) {
        case kMsgVideoCreatorStart : {
            isDecoding = true;
            startDecode();
            break;
        }
        case kMsgVideoCreatorStop: {
            break;
        }
        default: {
            ALOGE("unknown type for VideoCreator");
            break;
        }
    }
}

void VideoCreator::pthreadExit() {
    Looper::pthreadExit();

    avcodec_close(codecContext);
    avcodec_free_context(&codecContext);
    avformat_close_input(&ic);
    av_packet_free(&pkt);
    for (unsigned int i = 0; i < frameList.size(); ++i) {
        av_frame_free(&frameList.at(i));
    }
    frameList.clear();
    ALOGE("VideoCreator release");
}
