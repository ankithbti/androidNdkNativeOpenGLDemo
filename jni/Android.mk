LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := TestAndroidApp

LOCAL_CFLAGS    := -Wall

LOCAL_SRC_FILES := TestAndroidApp.cpp \

## For Native Support
#######
LOCAL_LDLIBS:= -landroid -llog -lEGL -lGLESv1_CM -lGLESv2
LOCAL_STATIC_LIBRARIES:= android_native_app_glue
#######

include $(BUILD_SHARED_LIBRARY)

## For Native Support
#######
$(call import-module,android/native_app_glue)
#######
