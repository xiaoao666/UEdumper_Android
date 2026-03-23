cmake_minimum_required(VERSION 3.20)
project(UEdumper_Android)

set(CMAKE_CXX_STANDARD 20)
##add_library   add_executable
add_executable(UEdumper_Android
        src/main.cpp
        src/Andoird-Memory-Debug.cpp
        src/UE_Tool.cpp
        src/UE_Dumper.h
)

find_library(log-lib log)
target_link_libraries(UEdumper_Android ${log-lib})


