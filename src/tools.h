#pragma once

#define TAG "Mod_Menu"
enum LogType {
    oDEBUG = 3,
    oERROR = 6,
    oINFO  = 4,
    oWARN  = 5
};
#define LOGD(...) ((void)__android_log_print(oDEBUG, TAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(oERROR, TAG, __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(oINFO,  TAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(oWARN,  TAG, __VA_ARGS__))


