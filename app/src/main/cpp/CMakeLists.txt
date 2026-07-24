cmake_minimum_required(VERSION 3.22.1)
project(roadforge_bussim LANGUAGES CXX)

add_library(roadforge SHARED
    src/core/Log.cpp
    src/core/SimulationClock.cpp
    src/engine/Engine.cpp
    src/platform/android/JniBridge.cpp
    src/renderer/VulkanRenderer.cpp
)

target_include_directories(roadforge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(roadforge PRIVATE cxx_std_20)
target_compile_options(roadforge PRIVATE
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter
)

find_library(android-lib android REQUIRED)
find_library(log-lib log REQUIRED)
find_library(vulkan-lib vulkan REQUIRED)

target_link_libraries(roadforge PRIVATE
    ${android-lib}
    ${log-lib}
    ${vulkan-lib}
)
