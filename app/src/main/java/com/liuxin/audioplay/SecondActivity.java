package com.liuxin.audioplay;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.liuxin.videolib.LXPlayer;
import com.liuxin.videolib.OnCompleteListener;
import com.liuxin.videolib.OnDBListener;
import com.liuxin.videolib.OnErrorListener;
import com.liuxin.videolib.OnLoadListener;
import com.liuxin.videolib.OnPlayStatusListener;
import com.liuxin.videolib.OnPrepareListener;
import com.liuxin.videolib.OnProgressListener;

public class SecondActivity extends AppCompatActivity {
    private LXPlayer lxPlayer;
    private LxSurfaceView view;
    private SeekBar seekBar;
    private int duration;
    private boolean isTouch = false;
    private TextView textView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_second);


        view = findViewById(R.id.surfaceView);
        textView = findViewById(R.id.textView);
        seekBar = findViewById(R.id.seek);
        lxPlayer = LXPlayer.getInstance();
        view.setNativeOpengl(lxPlayer);


        lxPlayer.setOnPlayStatusListener(new OnPlayStatusListener() {

            @Override
            public void playStatus(final int status) {
                if (status == 1) {
                    Log.e("ffmpeg", "播放状态");
                } else if (status == 2) {
                    Log.e("ffmpeg", "暂停状态");
                } else if (status == 3) {
                    Log.e("ffmpeg", "停止状态");
                } else if (status == 0) {
                    Log.e("ffmpeg", "初始状态");
                }


            }
        });
        lxPlayer.setOnPrepareListener(new OnPrepareListener() {
            @Override
            public void prepare() {
                duration = lxPlayer.getDuration();
                lxPlayer.start();

            }
        });

        lxPlayer.setOnLoadListener(new OnLoadListener() {
            @Override
            public void onLoad(boolean isLoad) {
                //    Log.e("ffmpeg", "isLoad==>" + isLoad);
            }
        });

        lxPlayer.setOnProgressListener(new OnProgressListener() {
            @Override
            public void progress(int duration, int current) {
                if (!isTouch) {
                    final int min = duration / 60;
                    final int sec = duration % 60;
                    final int cumin = current / 60;
                    final int cusec = current % 60;
                    textView.setText(cumin + ":" + cusec + "/" + min + ":" + sec);
                    if (duration != 0) {
                        seekBar.setProgress(current * 100 / duration);
                    } else {
                        seekBar.setProgress(0);
                    }


                }

            }
        });
        lxPlayer.setOnErrorListener(new OnErrorListener() {
            @Override
            public void error(int code, String msg) {
                Log.e("ffmepg", "error code==>" + code + " msg==>" + msg + "thread=>" + Thread.currentThread().getName());
            }
        });
        lxPlayer.setOnCompleteListener(new OnCompleteListener() {
            @Override
            public void onComplete() {
                Log.e("ffmpeg", "完成");
            }
        });

        lxPlayer.setOnDBListener(new OnDBListener() {
            @Override
            public void currentDB(int db) {
                //     Log.e("ffmpeg", "当前的db==>" + db);
            }
        });


        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    final int min = duration / 60;
                    final int sec = duration % 60;
                    int current = duration * progress / seekBar.getMax();
                    final int cumin = current / 60;
                    final int cusec = current % 60;
                    textView.setText(cumin + ":" + cusec + "/" + min + ":" + sec);

                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isTouch = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isTouch = false;
                int currentSec = duration * seekBar.getProgress() / seekBar.getMax();
                lxPlayer.seek(currentSec);


            }
        });


    }

    public void start(View view) {
        //有问题的地址
        //"http://media.w3.org/2010/05/sintel/trailer.mp4"
        //"http://mirror.aarnet.edu.au/pub/TED-talks/911Mothers_2010W-480p.mp4"
       // Log.e("lx",Environment.getExternalStorageDirectory().getAbsolutePath()+"/Test2.mp4");
   lxPlayer.prepare("http://mirror.aarnet.edu.au/pub/TED-talks/911Mothers_2010W-480p.mp4");



    }

    public void release(View view) {
        duration = 0;
        lxPlayer.releaseGlobal();
    }

    @Override
    protected void onDestroy() {
        Log.e("lx","onDestory");
        lxPlayer.releaseGlobal();
        super.onDestroy();
    }

    public void pause(View view) {
        lxPlayer.pause();
    }

    public void play(View view) {
        lxPlayer.play();
    }
}
