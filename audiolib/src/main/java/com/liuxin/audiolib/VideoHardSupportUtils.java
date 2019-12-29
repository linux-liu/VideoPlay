package com.liuxin.audiolib;

import android.media.MediaCodecList;
import android.text.TextUtils;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

public class VideoHardSupportUtils {

    private static Map<String, String> supportMapList = new HashMap<>();

    static {

        supportMapList.put("h264", "video/avc");
        supportMapList.put("h265", "video/hevc");


    }

    public static String getHardType(String name) {

        if (supportMapList.containsKey(name)) {
            return supportMapList.get(name);
        }

        return "";
    }


    public static boolean isSupportHardCode(String ffmpeg_name) {
        Log.e("ffmpeg","isSupport=>"+ffmpeg_name);
        int count = MediaCodecList.getCodecCount();

        String type=getHardType(ffmpeg_name);

        if(!TextUtils.isEmpty(type)){
            for (int i = 0; i < count; i++) {
                String[] supportType = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
                if (supportType != null) {
                    for (int j = 0; j < supportType.length; j++) {
                         if(type.equals(supportType[j])){
                             return true;
                         }
                    }
                }


            }
        }

        return false;

    }
}
