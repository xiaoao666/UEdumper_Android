#pragma once

#include <string>

std::string DumpFnamePath;
std::string DumpUObjectDumpPath;
std::string DumpPath;


void DumpFNames(){
    uint32_t index = 0;
    uint32_t EntryId;
    printf("Dumper FName -Start\r\n");
    FILE* pf = fopen(DumpFnamePath.c_str(),"wb");
    uint64_t NameEntry = offsets.GName + offsets.FNameEntry.FNameEntry_offset;
    uint32_t CurrentBlock = mem.Read<uint32_t>(offsets.GName + offsets.FNameEntry.CurrentBlock_offset);
    printf("CurrentBlock: %d\r\n", CurrentBlock);
    for (size_t Block = 0; Block < CurrentBlock; Block++) {
        uint64_t NameChunk = mem.ReadDword64(NameEntry + Block * 8);
        if (NameChunk == 0) {
            continue;
        }
        uint64_t NameEntryStart = NameChunk;
        uint64_t NameEntryEnd = NameChunk + BlockSizeBytes;
        while (NameEntryStart < NameEntryEnd)
        {
            //推导当前条目的offset，依据：NameEntry = NameChuck + Offset * Stride
            uint32_t Offset = (NameEntryStart - NameChunk) / Stride;
            std::string str_index = "index";
            str_index += std::to_string(index) + "-Tag";
            str_index += std::to_string(GetFNameEntryId(Block, Offset)) + " "+ "Value: ";
            int16_t EntryHeader = mem.Read<int16_t>(NameEntryStart);
            int len = EntryHeader >> 6;
            std::string name = mem.ReadString(NameEntryStart + offsets.FNameEntry.NameEntry_size, len);
            fwrite(str_index.c_str(), 1, str_index.size(), pf);
            fwrite(name.c_str(), 1, name.size(), pf);
            fwrite("\r\n", 1, strlen("\r\n"), pf);
            if (len == 0)
            {
                break;
            }
            // 使用 GetNextSize 算法计算下一个条目位置
            uint16_t nextSize = GetNextSize(EntryHeader);
            NameEntryStart += nextSize;
            index++;
        }
    }
    fclose(pf);
    printf("Dumper FName -End\r\n");
}



void DumpActorNamesFromGWorld(){
    uint64_t ULevel = mem.ReadDword64(mem.ReadDword64(offsets.GWorld) + offsets.world.PersistentLevel_offset);
    printf("ULevel: %p\n", (void*)ULevel);
    TArray<uintptr_t> Actors = mem.Read<TArray<uintptr_t>>(ULevel + offsets.ULevel.Actors_offset);
    printf("Actors: %p\n", (void*)Actors.GetData());
    int32_t count = Actors.Num();
    int32_t max = Actors.Max();
    printf("count: %d, max: %d\n", count, max);

    for (size_t i = 0; i < Actors.Num(); i++)
    {
        uintptr_t Actor = mem.ReadDword64((uintptr_t)Actors.GetData() + i * 0x8);
        uint32_t nameIndex = mem.Read<uint32_t>(Actor + offsets.UObject.NamePrivate);
        std::string ActorName = GetNameByIndex(nameIndex);
        uintptr_t ClassPtr = mem.ReadDword64(Actor + offsets.UObject.ClassPrivate);
        uint32_t ClassNameIndex = mem.Read<uint32_t>(ClassPtr + offsets.UObject.NamePrivate);
        std::string ClassName = GetNameByIndex(ClassNameIndex);
        uintptr_t SuperStructPtr = mem.ReadDword64(ClassPtr + offsets.UStruct.SuperStruct);
        while (mem.ReadDword64(SuperStructPtr + offsets.UStruct.SuperStruct) != 0 && mem.ReadDword64(SuperStructPtr + offsets.UStruct.SuperStruct) != SuperStructPtr) {
            SuperStructPtr = mem.ReadDword64(SuperStructPtr + offsets.UStruct.SuperStruct);
            uint32_t SuperClassNameIndex = mem.Read<uint32_t>(SuperStructPtr + offsets.UObject.NamePrivate);
            std::string SuperClassName = GetNameByIndex(SuperClassNameIndex);
            printf("SuperStructPtr: %p SuperStructClassName: %s\n", (void*)SuperStructPtr, SuperClassName.c_str());
            // if (SuperClassName == "GamePlayer")
            // {
            //     printf("Actor:%p\n", (void*)Actor);
            //     uintptr_t Ability = mem.ReadDword64(Actor + 0x540);
            //     printf("Ability:%p\n", (void*)Ability);
            //     Ability = mem.ReadDword64(Actor + 0x7D8);
            //     printf("Ability:%p\n", (void*)Ability);
            //
            // }
            //
            if (SuperClassName == "PlayerWeapon")
            {
                //printf("PlayerWeapon:%p\n", (void*)Actor);
                int ShootBulletCost = mem.Read<int>(Actor + 0x1398);
                int ShootBulletLaunchCount = mem.Read<int>(Actor + 0x139C);
                //printf("ShootBulletCost: %d, ShootBulletLaunchCount: %d\n", ShootBulletCost, ShootBulletLaunchCount);
                if (ShootBulletCost == 0 || ShootBulletLaunchCount == 0) {
                    continue;
                }
                mem.edit<int>(0, ((uintptr_t)Actor + 0x1398),DWORD,true);
                mem.edit<int>(50, ((uintptr_t)Actor + 0x139C),DWORD,true);
            }
        }
        printf("ClassPtr: %p ClassName: %s\n", (void*)ClassPtr, ClassName.c_str());
        printf("Actor: %p ActorName: %s\n", (void*)Actor, ActorName.c_str());


    }
}

void DumpAllUObjects(){
    int32_t MaxElements = mem.Read<int32_t>(offsets.GUObjectArray + offsets.ObjectArray.MaxElements_offset);
    int32_t NumElements = mem.Read<int32_t>(offsets.GUObjectArray + offsets.ObjectArray.NumElements_offset);
    printf("MaxElements: %d, NumElements: %d\n", MaxElements, NumElements);
    std::ofstream outFile(DumpUObjectDumpPath);
    if (!outFile) {
        printf("无法打开文件进行写入.");
        return;
    }

    for(uint32_t i=0;i<NumElements;i++){
        UE_UObject obj = GetObjectPtrByIndex(i);
        if(!obj.GetAddress()) continue;
        std::cout << "\rUObject: " << i << " / " << NumElements << std::flush;
        uint32_t nameIndex = mem.Read<uint32_t>(obj.GetAddress() + offsets.UObject.NamePrivate);
        //printf("nameIndex: %d\n", nameIndex);
        std::string name = obj.GetFullName();
        //LOGI("UObject[%u] ptr:%p nameIndex:%u",i,(void*)obj,nameIndex);
        outFile << "UObject[" << i << "] ptr:" << obj.GetAddress() << " nameIndex:" << nameIndex << " name:" << name <<std::endl;
    }
    outFile.close();
    printf("Dumper UObject -End\r\n");
}

void GatherUObjects(std::vector<std::pair<UE_UObject , std::vector<UE_UObject>>> &packages){
    int32_t NumElements = mem.Read<int32_t>(offsets.GUObjectArray + offsets.ObjectArray.NumElements_offset);
    printf("NumElements: %d\n", NumElements);
    std::ofstream outFile(DumpUObjectDumpPath);
    if (!outFile) {
        printf("无法打开文件进行写入.");
        return;
    }
    for(uint32_t i=0;i<NumElements;i++){
        UE_UObject obj = GetObjectPtrByIndex(i);
        if(!obj.GetAddress()) continue;
        //printf("UObject[%u] ptr:%p nameIndex:%u name:%s IsUEnum:%d\n", i, (void*)obj.GetAddress(), obj.GetIndex(), obj.GetFullName().c_str(), obj.IsA<UE_UEnum>());
        if (obj.IsA<UE_UFunction>() || obj.IsA<UE_UStruct>() || obj.IsA<UE_UEnum>())
        {
            bool found = false;
            auto packageObj = obj.GetPackageObject();
            for (auto &pkg : packages)
            {
                if (pkg.first == packageObj)
                {
                    pkg.second.push_back(obj);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                packages.push_back(std::make_pair(packageObj, std::vector<UE_UObject>(1, obj)));
            }
            outFile << "UObject[" << i << "] nameIndex:" << obj.GetIndex() << " name:" << obj.GetFullName() <<std::endl;
            std::cout << "\rUObject: [" << i << " / " << NumElements << "]" << std::flush;
        }
    }
    printf("\n");
    outFile.close();
}


void dump(){
    std::vector<std::pair<UE_UObject, std::vector<UE_UObject>>> packages;
    GatherUObjects(packages);
    std::ofstream outFile(DumpPath);
    if (!outFile) {
        printf("无法打开文件进行写入.");
        return;
    }
    outFile << "#pragma once\n\n#include <cstdio>\n#include <string>\n#include <cstdint>\n\n\n"<< std::endl;
    size_t i = 0;
    for (auto &pkg : packages)
    {
        i++;
        std::cout << "\rProgress: " << (int)i << "/" << (int)packages.size() << " Package: " << pkg.first.GetName() << std::flush;
        UE_UPackage package(pkg);
        package.Process();
        if (package.Classes.size() || package.Structures.size() || package.Enums.size()){
            //            aioBufferFmt.append("// Package: {}\n// Enums: {}\n// Structs: {}\n// Classes: {}\n\n",
            //                                package.GetObject().GetName(), package.Enums.size(), package.Structures.size(), package.Classes.size());
            outFile << "// Package: " << package.GetObject().GetName() << std::endl;
            outFile << "// Enums: " << package.Enums.size() << std::endl;
            outFile << "// Structs: " << package.Structures.size() << std::endl;
            outFile << "// Classes: " << package.Classes.size() << std::endl;
            outFile << std::endl;
            if (package.Enums.size()) {
                auto pkgEnums = package.Enums;
                UE_UPackage::AppendEnums(pkgEnums, outFile);
            }
            if (package.Structures.size()) {
                auto pkgStructs = package.Structures;
                UE_UPackage::AppendStructs(pkgStructs, outFile);
            }
            if (package.Classes.size()) {
                auto pkgClasses = package.Classes;
                UE_UPackage::AppendStructs(pkgClasses, outFile);
            }
        }
    }
    printf("\n");
    outFile.close();
}



//这部分是注入用的，一些游戏会将内存页设置为只写，需要注入修改内存页权限
char mapsPath[1024];
//获取指定地址所在内存页
int getMemoryPage(unsigned long addr,unsigned long* start,size_t* length) {
    int i=-1;
    FILE* maps = fopen(mapsPath, "r");
    if (maps) {
        char mapLine[1024];
        unsigned long startBuff,endBuff;
        while (!feof(maps))
        {
            fgets(mapLine, sizeof(mapLine), maps);
            sscanf(mapLine, "%lx-%lx", &startBuff, &endBuff);
            if (addr >= startBuff && addr <= endBuff) {
                *start=startBuff;
                *length = endBuff - startBuff;
                i=0;
                break;
            }
        }
        fclose(maps);
    }
    return i;
}
int setAddrVisitP(unsigned long addr,int p){
    unsigned long start;
    size_t len;
    int i = getMemoryPage(addr,&start,&len);
    if(i!=-1){
        printf("start: %p\n",start);
        if (mprotect((void*)start, len, p) == -1) {
            LOGI("mprotect failed\n");
        }
        i=0;
    }
    return i;
}