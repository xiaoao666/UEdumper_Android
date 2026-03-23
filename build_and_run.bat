@echo off
setlocal enabledelayedexpansion

:: 设置变量
set BUILD_DIR=cmake-build-arm64-v8a
set EXECUTABLE_NAME=uedumper
set NDK_TOOLCHAIN=E:\SDK\ndk\25.0.8775105\build\cmake\android.toolchain.cmake
set ANDROID_ABI=arm64-v8a
set TARGET_PATH=/data/local/tmp/%EXECUTABLE_NAME%

:: 1. 创建构建目录（如果不存在）
if not exist "%BUILD_DIR%" (
    echo Creating build directory...
    mkdir "%BUILD_DIR%"
)

:: 2. 进入构建目录
cd "%BUILD_DIR%"

:: 3. 运行CMake配置
echo Running CMake configuration...
cmake -G Ninja -DTARGET_ANDROID=ON -DANDROID_ABI=%ANDROID_ABI% -DCMAKE_TOOLCHAIN_FILE=%NDK_TOOLCHAIN% -DBUILD_SHARED_LIBS=OFF ..

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    cd ..
    exit /b %errorlevel%
)

:: 4. 编译项目
echo Building project...
ninja

if %errorlevel% neq 0 (
    echo Build failed!
    cd ..
    exit /b %errorlevel%
)

:: 5. 返回项目根目录
cd ..

:: 6. 推送可执行文件到手机
echo Pushing executable to device...
adb push "%BUILD_DIR%/%EXECUTABLE_NAME%" %TARGET_PATH%

if %errorlevel% neq 0 (
    echo Push failed!
    exit /b %errorlevel%
)

:: 7. 设置可执行权限
echo Setting executable permissions...
adb shell chmod +x %TARGET_PATH%

if %errorlevel% neq 0 (
    echo Failed to set permissions!
    exit /b %errorlevel%
)

echo ================================
echo UEDumper Build and Run Script
echo ================================

:: 8. 以root权限运行
echo Running with root privileges...
adb shell -t "su -c '/data/local/tmp/uedumper 2>&1 || echo 执行失败，退出码 $?'"

if %errorlevel% neq 0 (
    echo Execution failed!
    exit /b %errorlevel%
)

echo ================================
echo Script completed successfully!
echo ================================
endlocal