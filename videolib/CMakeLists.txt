cmake_minimum_required(VERSION 3.4.1)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")

endif()
find_library(log-lib log)
find_library(android-lib android)


add_library(native-lib
        SHARED
        src/main/cpp/native-lib.cpp
        src/main/cpp/CallJava.cpp
        src/main/cpp/LXFFmpeg.cpp
        src/main/cpp/Audio.cpp
        src/main/cpp/AvPacketQuene.cpp
        src/main/cpp/PlayStatus.cpp
        src/main/cpp/Video.cpp

        src/main/cpp/soundtouch/AAFilter.cpp
        src/main/cpp/soundtouch/FIRFilter.cpp
        src/main/cpp/soundtouch/BPMDetect.cpp
        src/main/cpp/soundtouch/PeakFinder.cpp
        src/main/cpp/soundtouch/cpu_detect_x86.cpp
        src/main/cpp/soundtouch/FIFOSampleBuffer.cpp
        src/main/cpp/soundtouch/InterpolateCubic.cpp
        src/main/cpp/soundtouch/InterpolateLinear.cpp
        src/main/cpp/soundtouch/InterpolateShannon.cpp
        src/main/cpp/soundtouch/mmx_optimized.cpp
        src/main/cpp/soundtouch/RateTransposer.cpp
        src/main/cpp/soundtouch/SoundTouch.cpp
        src/main/cpp/soundtouch/sse_optimized.cpp
        src/main/cpp/soundtouch/TDStretch.cpp

        src/main/cpp/egl/Egl_Helper.cpp
        src/main/cpp/egl/EglThread.cpp
        src/main/cpp/opengl/BaseOpenGL.cpp
        src/main/cpp/opengl/OpenGLFbo.cpp
        src/main/cpp/opengl/OpenGLCommonFilter.cpp
        src/main/cpp/opengl/OpenGLHelper.cpp
        src/main/cpp/opengl/OpenGLGreyFilter.cpp
        src/main/cpp/opengl/OpenGLYUVFilter.cpp
        )

set(JNI_LIBS_DIR ${CMAKE_SOURCE_DIR}/src/main/jniLibs)


add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavutil.so)

add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswresample.so)

add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswscale.so)

add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavcodec.so)

add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavformat.so)

add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavfilter.so)

add_library(avdevice
        SHARED
        IMPORTED)
set_target_properties(avdevice
        PROPERTIES IMPORTED_LOCATION
        ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavdevice.so)

include_directories(${JNI_LIBS_DIR}/includes)

include_directories(${JNI_LIBS_DIR}/includes/soundtouch)

target_link_libraries(native-lib
        OpenSLES
        EGL
        GLESv2
        avutil swresample swscale avcodec avformat avfilter avdevice
        ${log-lib} ${android-lib})