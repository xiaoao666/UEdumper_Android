# Application.mk - 全局配置文件

# 支持的架构
APP_ABI := armeabi-v7a arm64-v8a x86 x86_64

# 使用的STL库
APP_STL := c++_static

# 支持的Android API级别
APP_PLATFORM := android-21

# C++标准版本
APP_CPPFLAGS := -std=c++20
