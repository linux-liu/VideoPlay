package com.liuxin.videolib;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

public class VideoHardSupportUtils {

    private static Map<String, String> supportMapList = new HashMap<>();

    static {
        supportMapList.put("h264", "video/avc");
        supportMapList.put("h265", "video/hevc");
        supportMapList.put("aac","audio/mp4a-latm");
        supportMapList.put("mp3","audio/mpeg");

    }

    public static String getHardType(String name) {

        if (supportMapList.containsKey(name)) {
            return supportMapList.get(name);
        }

        return "";
    }


    public static boolean isSupportHardCode(String ffmpeg_name) {

       String type=getHardType(ffmpeg_name);
        MediaCodecList mediaCodecList=new MediaCodecList(MediaCodecList.REGULAR_CODECS);
        MediaCodecInfo[] mediaCodecInfos=mediaCodecList.getCodecInfos();

        if(mediaCodecInfos!=null&&mediaCodecInfos.length>0){
            for(int i=0;i<mediaCodecInfos.length;i++){
                String []supportType=mediaCodecInfos[i].getSupportedTypes();
                for(int j=0;j<supportType.length;j++){

                    if(type.equals(supportType[j])){
                        return true;
                    }
                }
            }
        }

        return false;

    }
}
