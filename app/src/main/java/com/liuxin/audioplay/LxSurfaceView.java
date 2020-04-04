package com.liuxin.audioplay;

import android.content.Context;
import android.util.AttributeSet;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import com.liuxin.videolib.LXPlayer;

public class LxSurfaceView extends SurfaceView implements SurfaceHolder.Callback {

    private LXPlayer lxPlayer;


    public void setNativeOpengl(LXPlayer player) {
        this.lxPlayer = player;
    }

    public LxSurfaceView(Context context) {
        this(context, null);
    }

    public LxSurfaceView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public LxSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        getHolder().addCallback(this);
    }



    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        if(lxPlayer != null)
        {
            lxPlayer.setSurface(surfaceHolder.getSurface());
        }

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {
        if(lxPlayer != null)
        {
            lxPlayer.setSurfaceChange(width,height);
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        if(lxPlayer != null)
        {

            lxPlayer.destorySurface();
        }
    }


}
