//
// Created by templechen on 2019-04-28.
//

#ifndef FFMPEG_PLAYER_GL_LOOPER_H
#define FFMPEG_PLAYER_GL_LOOPER_H


#include <decode/circle_av_frame_queue.h>
#include "GLVideoPlayer.h"
#include "../base/Looper.h"

class GLVideoLooper : public Looper {

public:

    enum {
        kMsgSurfaceCreated,
        kMsgSurfaceChanged,
        kMsgSurfaceDestroyed,
        kMsgSurfaceStart,
        kMsgSurfacePause,
        kMsgSurfaceSeek,
        kMsgSurfaceDoFrame
    };

    GLVideoLooper(circle_av_frame_queue *frameQueue);

    virtual ~GLVideoLooper();

    virtual void handleMessage(LooperMessage *msg) override;

    long getCurrentPos();

private:
    GLVideoPlayer *glVideoPlayer;

    bool destroyed;

    circle_av_frame_queue *frameQueue;
};

#endif //FFMPEG_PLAYER_GL_LOOPER_H
