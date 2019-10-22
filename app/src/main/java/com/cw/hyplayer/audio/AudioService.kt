package com.cw.hyplayer.audio

import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.Service
import android.content.Context
import android.content.Intent
import android.os.Binder
import android.os.IBinder

class AudioService : Service() {

    companion object {
        const val CHANNEL_ID = "HYAudioPlay"
        const val CHANNEL_NAME = "HYAudioService"
    }

    var audioPlayer: HYAudioPlayer? = null

    override fun onCreate() {
        super.onCreate()
        val channel =
            NotificationChannel(CHANNEL_ID, CHANNEL_NAME, NotificationManager.IMPORTANCE_DEFAULT)
        val manager = getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
        manager.createNotificationChannel(channel)
        val notification = Notification.Builder(applicationContext, CHANNEL_ID).build()
        startForeground(1, notification)
    }

    override fun onBind(intent: Intent?): IBinder? {
        return AudioBinder()
    }

    fun create() {
        if (audioPlayer == null) {
            val mediaSource = MediaSource("/sdcard/test.mp3")
//                val mediaSource = MediaSource("/sdcard/trailer111.mp4")
            audioPlayer = HYAudioPlayer(mediaSource)
        }
    }

    fun release() {
        audioPlayer?.release()
        audioPlayer = null
    }

    fun start() {
        audioPlayer?.start()
    }

    fun pause() {
        audioPlayer?.pause()
    }

    fun seek(pos: Long) {
        audioPlayer?.seek(pos)
    }

    fun totalDuration(): Long {
        audioPlayer?.let {
            return audioPlayer!!.totalDuration()
        }
        return 0L
    }

    fun currentTime(): Long {
        audioPlayer?.let {
            return audioPlayer!!.currentTime()
        }
        return 0L
    }

    inner class AudioBinder : Binder() {

        fun callCreate() {
            create()
        }

        fun callStart() {
            start()
        }

        fun callPause() {
            pause()
        }

        fun callSeek(pos: Long) {
            seek(pos)
        }

        fun callRelease() {
            release()
        }

        fun callTotalDuration(): Long {
            return totalDuration()
        }

        fun callCurrentTime(): Long {
            return currentTime()
        }
    }

}