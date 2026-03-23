#include <iostream>
#include <fstream>
#include <android/log.h>
#include <sys/stat.h>
#include <sstream>
#include "vector"
#include "tools.h"
#include "Android-Memory-Debug.hpp"
#include "UE_Offset.h"
#include "UE_Tool.h"
#include "UPackageGenerator.hpp"
#include "UE_Dumper.h"




//可执行文件的主函数
int main() {
    mem.setPackageName("com.xxx.xxx");
    offsets.UE_base = mem.getModuleBase("libUE4.so");
    printf("UE_base: %p\n", (void*)offsets.UE_base);
    offsets.GName = offsets.UE_base + offsets.GName_offset;
    printf("GName: %p\n", (void*)offsets.GName);
    //setAddrVisitP(offsets.GName,PROT_READ|PROT_WRITE);
    printf("None:%s\n",GetNameByIndex(0).c_str());
    offsets.GUObjectArray = offsets.UE_base + offsets.GUObjectArray_offset;
    printf("GUObjectArray: %p\n", (void*)offsets.GUObjectArray);
    offsets.GWorld = offsets.UE_base + offsets.GWorld_offset;
    printf("GWorld: %p\n", (void*)offsets.GWorld);
    //DumpActorNamesFromGWorld();
    //setAddrVisitP(offsets.GUObjectArray,PROT_READ|PROT_WRITE);
    //DumpAllUObjects();
    DumpFNames();
    dump();
    // UE_UObject obj = GetObjectPtrByIndex(5644);
    // printf("obj: %p\n", (void*)obj.GetAddress());
    // printf("obj name: %s\n", obj.GetFullName().c_str());
    // printf("obj class: %s\n", obj.GetClass().GetFullName().c_str());
    // printf("obj package: %s\n", obj.GetPackageObject().GetName().c_str());
    // printf("obj is class: %d\n", obj.IsA<UE_UStruct>());



    return 0;
}
//注入用的代码
// __attribute__((constructor))
// void lib_main() {
//
//     mem.setPackageName("com.dragonli.projectsnow.lhm");
//     offsets.UE_base = mem.getModuleBase("libUE4.so");
//     LOGI("UE_base: %p\n", (void*)offsets.UE_base);
//     offsets.GName = offsets.UE_base + offsets.GName_offset;
//     LOGI("GName: %p\n", (void*)offsets.GName);
//     //setAddrVisitP(offsets.GName,PROT_READ|PROT_WRITE);
//     LOGI("None:%s\n",GetNameByIndex(0).c_str());
//     offsets.GUObjectArray = offsets.UE_base + offsets.GUObjectArray_offset;
//     LOGI("GUObjectArray: %p\n", (void*)offsets.GUObjectArray);
//     offsets.GWorld = offsets.UE_base + offsets.GWorld_offset;
//     LOGI("GWorld: %p\n", (void*)offsets.GWorld);
//     IsJumping = (bool(*)(void*))(offsets.UE_base + 0x5E46680);
//     //IsJumping = (bool(*)(void*))(offsets.UE_base + 0x687d4dc);
//     LOGI("IsJumping: %p\n", (void*)IsJumping);
//     //IsInFight = (bool(*)(void*))(offsets.UE_base + 0x687bc40);
//     //LOGI("IsInFight: %p\n", (void*)IsInFight);
//     DumpActorNamesFromGWorld();
//
// }