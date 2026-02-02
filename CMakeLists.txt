# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(voice_cap)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC
    ${CMAKE_SOURCE_DIR}/source/*.c
    ${CMAKE_SOURCE_DIR}/source/*.cpp
)

add_executable(voice_cap ${SRC})

target_include_directories(voice_cap PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib
)

target_link_libraries(voice_cap PRIVATE 
    pthread asound rknn_api
    ${CMAKE_SOURCE_DIR}/lib/webrtc/libwebrtcvad.a
    ${CMAKE_SOURCE_DIR}/lib/logmel/liblogmel.a)
