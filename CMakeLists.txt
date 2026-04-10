cmake_minimum_required(VERSION 3.10)
project(audvi VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tinyfiledialogs STATIC
    src/lib/tinyfiledialogs/tinyfiledialogs.c
)
target_include_directories(tinyfiledialogs PUBLIC src/lib/tinyfiledialogs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_executable(audvi src/main.cpp src/vis.cpp)

target_include_directories(audvi PRIVATE src src/lib)

target_link_libraries(audvi
    tinyfiledialogs
    raylib
    portaudio
    fftw3
    m pthread dl
)

if(WIN32)
    target_link_libraries(audvi comdlg32 ole32)
endif()