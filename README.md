# UEdumper_Android

## 项目简介
UEdumper_Android 是一个专为 Android 平台设计的 Unreal Engine 游戏内存转储工具，用于提取游戏中的 FName、UObject 等信息，帮助开发者和研究人员分析游戏数据结构。

## 功能特性
- 转储 FName 表，获取游戏中所有命名对象的名称
- 转储 UObject 表，获取游戏中所有对象的信息
- 从 GWorld 转储 Actor 名称和类信息
- 支持修改内存页权限，处理游戏的内存保护机制
- 生成包含游戏类、结构体和枚举定义的头文件

## 环境要求
- Android SDK 和 NDK (推荐 NDK 25.0 或更高版本)
- CMake (3.18 或更高版本)
- Ninja 构建系统
- Android 设备（需要 root 权限）
- Windows 操作系统（用于运行构建脚本）

## 构建和运行

### 1. 配置环境变量
编辑 `build_and_run.bat` 文件，设置正确的 NDK 路径：
```batch
set NDK_TOOLCHAIN=E:\SDK\ndk\25.0.8775105\build\cmake\android.toolchain.cmake
```

### 2. 构建项目
运行 `build_and_run.bat` 脚本，该脚本会：
- 创建构建目录
- 运行 CMake 配置
- 编译项目
- 推送可执行文件到设备
- 设置可执行权限
- 以 root 权限运行

```batch
./build_and_run.bat
```

## 输出文件
工具运行后会在设备的 `/sdcard/Download/` 目录生成以下文件：
- `Name.txt` - FName 表转储
- `UObject.txt` - UObject 表转储
- `Dump.cpp` - 包含游戏类、结构体和枚举定义的头文件

## 配置
在 `main.cpp` 文件中，你可以修改以下配置：

```cpp
// 目标包名
mem.setPackageName("com.xxx.xxx");

// 输出文件路径
DumpFnamePath = "/sdcard/Download/Name.txt";
DumpUObjectDumpPath = "/sdcard/Download/UObject.txt";
DumpPath = "/sdcard/Download/Dump.cpp";
```

## 注意事项
- 工具需要在 root 权限下运行，因为需要访问游戏进程的内存
- 不同版本的 Unreal Engine 可能需要调整偏移量，在 `UE_Offset.h` 中配置
- 工具仅用于学习和研究目的，请勿用于非法用途

## 技术原理
- 使用 Android 内存调试工具读取游戏进程内存
- 根据 Unreal Engine 的内存布局，定位 GName、GUObjectArray、GWorld 等关键数据结构
- 解析内存中的数据结构，提取有用信息
- 生成结构化的输出文件

## Credits & Thanks
* [UE4Dumper-4.25](https://github.com/guttir14/UnrealDumper-4.25)
* [Il2cppDumper](https://github.com/Perfare/Il2CppDumper)
* [Dumper-7](https://github.com/Encryqed/Dumper-7)
* [UEDumper](https://github.com/Spuckwaffel/UEDumper)
* [AndUEDumper](https://github.com/MJx0/AndUEDumper)
* [Android-MemoryDebug-Tools](https://github.com/oobbb/Android-MemoryDebug-Tools)
