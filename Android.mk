#ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk NDK_APPLICATION_MK=Application.mk APP_PLATFORM=android-21 APP_STL=c++_static

LOCAL_PATH := $(call my-dir)

# 构建共享库 (so文件)
include $(CLEAR_VARS)
LOCAL_MODULE := uedumper_lib
LOCAL_SRC_FILES := \
    src/main.cpp \
    src/Andoird-Memory-Debug.cpp \
    src/UE_Tool.cpp
LOCAL_CXXFLAGS := -std=c++20
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

# 构建可执行文件
define my-dir
  $(strip $(call parent-dir,$(lastword $(MAKEFILE_LIST))))
endef

include $(CLEAR_VARS)
LOCAL_MODULE := uedumper
LOCAL_SRC_FILES := \
    src/main.cpp \
    src/Andoird-Memory-Debug.cpp \
    src/UE_Tool.cpp
LOCAL_CXXFLAGS := -std=c++20
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)


