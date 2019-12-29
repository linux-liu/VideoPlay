package com.liuxin.audiolib;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v4.app.NavUtils;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;

import java.io.IOException;
import java.nio.ByteBuffer;


public class LXPlayer {


    private volatile boolean isSwitch = false;

    private OnPrepareListener onPrepareListener;

    private OnProgressListener onProgressListener;

    private OnPlayStatusListener onPlayStatusListener;

    private OnCompleteListener onCompleteListener;

    private OnErrorListener onErrorListener;

    private OnDBListener onDBListener;

    private OnLoadListener onLoadListener;

    private static LXPlayer lxPlayer;

    private String url;


    private MediaCodec mediaCodec;
    private MediaFormat mediaFormat;
    private MediaCodec.BufferInfo info;

    private int picWidth;
    private int picHeight;
    private int yuvFormat=0; //0为yuv420p 1为nv12;


    private final int MSG_PREPARE = 0x123;
    private final int MSG_PROGRESS = 0x124;
    private final int MSG_PLAYSTATUS = 0x125;
    private final int MSG_COMPLETE = 0x126;
    private final int MSG_ERROR = 0x127;
    private final int MSG_CALL_SWITCH = 0x128;
    private final int MSG_DB = 0x129;
    private final int MSG_LOAD = 0x130;
    private static final int DEFAULT_TIMEOUT = 3 * 1000;


    private Handler mHandler = new Handler(Looper.getMainLooper()) {

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_PREPARE:
                    if (onPrepareListener != null) {
                        onPrepareListener.prepare();
                    }
                    break;
                case MSG_PROGRESS:
                    if (onProgressListener != null) {
                        onProgressListener.progress(msg.arg1, msg.arg2);
                    }
                    break;
                case MSG_PLAYSTATUS:
                    if (onPlayStatusListener != null) {
                        onPlayStatusListener.playStatus(msg.arg1);
                    }
                    break;
                case MSG_COMPLETE:
                    releaseGlobal();
                    if (onCompleteListener != null) {
                        onCompleteListener.onComplete();
                    }
                    break;
                case MSG_ERROR:
                    releaseGlobal();
                    if (onErrorListener != null) {
                        onErrorListener.error(msg.arg1, (String) msg.obj);
                    }
                    break;

                case MSG_CALL_SWITCH:
                    if (isSwitch) {
                        isSwitch = false;
                        prepare(url);
                    }
                    break;
                case MSG_DB:
                    if (onDBListener != null) {
                        onDBListener.currentDB(msg.arg1);
                    }
                    break;

                case MSG_LOAD:
                    if (onLoadListener != null) {
                        onLoadListener.onLoad((boolean) msg.obj);
                    }
                    break;
            }
        }
    };


    private LXPlayer() {

    }

    public static LXPlayer getInstance() {
        if (lxPlayer == null) {
            lxPlayer = new LXPlayer();
        }
        return lxPlayer;
    }


    public void setOnPrepareListener(OnPrepareListener listener) {
        this.onPrepareListener = listener;
    }

    public void setOnProgressListener(OnProgressListener listener) {
        this.onProgressListener = listener;
    }

    public void setOnPlayStatusListener(OnPlayStatusListener listener) {
        this.onPlayStatusListener = listener;
    }

    public void setOnCompleteListener(OnCompleteListener listener) {
        this.onCompleteListener = listener;
    }

    public void setOnErrorListener(OnErrorListener listener) {
        this.onErrorListener = listener;
    }

    public void setOnDBListener(OnDBListener listener) {
        this.onDBListener = listener;
    }


    public void setOnLoadListener(OnLoadListener listener) {
        this.onLoadListener = listener;
    }

    static {
        System.loadLibrary("native-lib");

    }


    /**
     * jni调用准备好了的方法
     */
    private void JniCallPrepare() {
        Message message = Message.obtain();
        message.what = MSG_PREPARE;
        mHandler.sendMessage(message);

    }


    private void JniCallProgress(int duration, int current) {
        Message message = Message.obtain();
        message.what = MSG_PROGRESS;
        message.arg1 = duration;
        message.arg2 = current;
        mHandler.sendMessage(message);


    }

    private void JniCallPlayStatus(int status) {
        Message message = Message.obtain();
        message.what = MSG_PLAYSTATUS;
        message.arg1 = status;

        mHandler.sendMessage(message);

    }

    private void JniCallComplete() {
        Message message = Message.obtain();
        message.what = MSG_COMPLETE;
        mHandler.sendMessage(message);
    }

    private void JniCallError(int code, String msg) {

        Message message = Message.obtain();
        message.what = MSG_ERROR;
        message.arg1 = code;
        message.obj = msg;
        mHandler.sendMessage(message);

    }

    private void JniCallSwitch() {

        Message message = Message.obtain();
        message.what = MSG_CALL_SWITCH;
        mHandler.sendMessage(message);

    }

    /**
     * 当前音乐的分贝值
     *
     * @param db
     */
    private void JniCallDB(int db) {
        Message message = Message.obtain();
        message.what = MSG_DB;
        message.arg1 = db;
        mHandler.sendMessage(message);
    }


    private void JniCallLoad(boolean isLoad) {
        Message message = Message.obtain();
        message.what = MSG_LOAD;
        message.obj = isLoad;
        mHandler.sendMessage(message);
    }


    /**
     * 是否支持硬编解码
     *
     * @param name
     * @return
     */
    private boolean JniCallSupportHard(String name) {
        boolean is = VideoHardSupportUtils.isSupportHardCode(name);
        Log.e("ffmpeg", "is=>" + is);
        return is;
    }


    /**
     * 初始化mediaCodec
     *
     * @param name
     * @param width
     * @param height
     * @param csd_0
     * @param csd_1
     */
    private void JniCallMeadCodecInit(String name, int width, int height, byte[] csd_0, byte[] csd_1) {


        if (mediaCodec == null) {
            try {

                this.picWidth = width;
                this.picHeight = height;
                String type = VideoHardSupportUtils.getHardType(name);
                mediaCodec = MediaCodec.createDecoderByType(type);
                mediaFormat = MediaFormat.createVideoFormat(type, width, height);
                mediaFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                mediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);
                info = new MediaCodec.BufferInfo();

                mediaCodec.configure(mediaFormat, null, null, 0);
                mediaCodec.start();


            } catch (IOException e) {
                Log.e("lx", "io异常");
                e.printStackTrace();
            }

        }


    }


    private void JNICallDecodeAvPacket(byte[] data, int size) {
        if (mediaCodec != null && data != null && size != 0) {

            try {
                int intputBufferIndex = mediaCodec.dequeueInputBuffer(DEFAULT_TIMEOUT);
                if (intputBufferIndex >= 0) {
                    ByteBuffer byteBuffer = null;

                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {

                        byteBuffer = mediaCodec.getInputBuffer(intputBufferIndex);
                    } else {
                        byteBuffer = mediaCodec.getInputBuffers()[intputBufferIndex];
                    }
                    if (byteBuffer != null) {
                        byteBuffer.clear();
                        byteBuffer.put(data);

                        mediaCodec.queueInputBuffer(intputBufferIndex, 0, size, 0, 0);
                    }

                }
                int outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, DEFAULT_TIMEOUT);
                ByteBuffer outBuffer = null;

                if(outputBufferIndex==MediaCodec.INFO_OUTPUT_FORMAT_CHANGED){
                    MediaFormat mediaFormat = mediaCodec.getOutputFormat();
                    switch (mediaFormat.getInteger(MediaFormat.KEY_COLOR_FORMAT)) {
                        case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV411Planar:
                            Log.e("ffmpeg","COLOR_FormatYUV411Plana");
                            break;
                        case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV411PackedPlanar:
                            Log.e("ffmpeg","FormatYUV411PackedPlanar");
                            break;
                        case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedPlanar:
                            Log.e("ffmpeg","COLOR_FormatYUV420PackedPlana");
                            break;
                        case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar:
                            Log.e("ffmpeg","COLOR_FormatYUV420SemiPlan");
                            yuvFormat=1;
                            break;
                        case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420PackedSemiPlanar:
                            Log.e("ffmpeg","COLOR_FormatYUV420PackedSemiPlanar");
                            break;
                        case MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar:
                            yuvFormat=0;
                            Log.e("ffmpeg","COLOR_FormatYUV420Planar");
                        default:
                            break;
                    }


                }else {
                    while (outputBufferIndex >= 0) {
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {

                            outBuffer = mediaCodec.getOutputBuffer(outputBufferIndex);
                        } else {
                            outBuffer = mediaCodec.getOutputBuffers()[outputBufferIndex];
                        }
                        if (outBuffer != null) {


                            outBuffer.position(info.offset);
                            outBuffer.limit(info.offset + info.size);
                            byte[] yuvData = new byte[outBuffer.remaining()];
                            outBuffer.get(yuvData);

                            mediaCodec.getOutputFormat();

                            setYUVData(yuvData, picWidth, picHeight,yuvFormat);

                            mediaCodec.releaseOutputBuffer(outputBufferIndex, false);
                            outBuffer.clear();
                        }
                        outputBufferIndex = mediaCodec.dequeueOutputBuffer(info, DEFAULT_TIMEOUT);
                    }
                }

            } catch (Exception e) {
                e.printStackTrace();
            }


        }


    }


    public void releaseGlobal() {
        release();
        releaseMediaCodec();

    }

    private void releaseMediaCodec() {
        try {
            if (mediaCodec != null) {
                mediaCodec.flush();
                mediaCodec.stop();
                mediaCodec.release();

            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        picWidth = 0;
        picHeight = 0;
        info = null;
        mediaFormat = null;
        mediaCodec = null;

    }


    /**
     * 所以公有方法在主线程中调用
     *
     * @param url
     */
    public void nextOrPre(String url) {
        if (TextUtils.isEmpty(url)) return;
        this.url = url;
        isSwitch = true;

        releaseGlobal();

    }


    public native void setSurface(Surface surface);

    public native void setYUVData(byte[]data, int picWidth, int picHeight,int yuvFormat);

    public native void destorySurface();

    public native void setSurfaceChange(int width, int height);

    public native void prepare(String url);

    public native void start();

    public native void pause();

    public native void play();

    public native void seek(int sec);

    public native void release();

    public native int getDuration();

    /**
     * 0到100 0最小 100最大
     *
     * @param volume
     */
    public native void setVolume(int volume);

    public native void setMute(boolean mute);

    /**
     * @param channel 0左声道 1右声道 2立体声
     */
    public native void setChannelSolo(int channel);

    //变调
    public native void setPitch(double pitch);

    //变速
    public native void setTemPo(double temPo);




}
