# 效果图如下
![image](https://github.com/linux-liu/VideoPlay/blob/master/Screenshot_20191229_191948_com.liuxin.audioplay.jpg)

###本程序为基于ffmpeg和opengL开发的视频播放器，涉及到的知识点主要有有ffmpeg的使用，音视频的解码 openGLES纹理的渲染，vbo fbo的使用。pthread多线程的使用，音视频同步等。MediaCodec硬解码。主要用jni层来实现。

###主要功能 播放，暂停 获取时长进度，快进等

主要接口如下：
```java

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
    
    ```
    
   
