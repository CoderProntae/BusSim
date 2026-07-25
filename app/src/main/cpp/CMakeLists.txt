cmake_minimum_required(VERSION 3.22.1)
project(roadforge_bussim LANGUAGES CXX)

set(RF_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(RF_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(RF_GENERATED_SHADER_DIR ${RF_GENERATED_DIR}/roadforge/renderer)
set(RF_GENERATED_SHADER_HEADER ${RF_GENERATED_SHADER_DIR}/GeneratedShaders.hpp)

set(RF_GLSLC_CANDIDATES
    ${ANDROID_NDK}/shader-tools/${ANDROID_HOST_TAG}/glslc
    $ENV{ANDROID_NDK_HOME}/shader-tools/linux-x86_64/glslc
    $ENV{ANDROID_NDK_ROOT}/shader-tools/linux-x86_64/glslc
)

set(RF_GLSLC "")
foreach(candidate IN LISTS RF_GLSLC_CANDIDATES)
    if(candidate AND EXISTS "${candidate}")
        set(RF_GLSLC "${candidate}")
        break()
    endif()
endforeach()

if(NOT RF_GLSLC)
    find_program(RF_GLSLC
        NAMES glslc
        HINTS
            ${ANDROID_NDK}/shader-tools/${ANDROID_HOST_TAG}
            ${ANDROID_NDK}/shader-tools/linux-x86_64
            $ENV{ANDROID_NDK_HOME}/shader-tools/linux-x86_64
            $ENV{ANDROID_NDK_ROOT}/shader-tools/linux-x86_64
        DOC "Vulkan GLSL to SPIR-V compiler from Android NDK shader-tools"
    )
endif()

if(NOT RF_GLSLC)
    message(FATAL_ERROR "glslc not found. Install Android NDK shader-tools or ensure ANDROID_NDK points to the NDK root.")
endif()

set(RF_VERT_SPV ${CMAKE_CURRENT_BINARY_DIR}/debug_triangle.vert.spv)
set(RF_FRAG_SPV ${CMAKE_CURRENT_BINARY_DIR}/debug_triangle.frag.spv)

add_custom_command(
    OUTPUT ${RF_GENERATED_SHADER_HEADER}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RF_GENERATED_SHADER_DIR}
    COMMAND ${RF_GLSLC} -fshader-stage=vert ${RF_SHADER_DIR}/debug_triangle.vert -o ${RF_VERT_SPV}
    COMMAND ${RF_GLSLC} -fshader-stage=frag ${RF_SHADER_DIR}/debug_triangle.frag -o ${RF_FRAG_SPV}
    COMMAND ${CMAKE_COMMAND}
        -DINPUT_VERT=${RF_VERT_SPV}
        -DINPUT_FRAG=${RF_FRAG_SPV}
        -DOUTPUT=${RF_GENERATED_SHADER_HEADER}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake
    DEPENDS
        ${RF_SHADER_DIR}/debug_triangle.vert
        ${RF_SHADER_DIR}/debug_triangle.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShaders.cmake
    VERBATIM
)

add_custom_target(roadforge_shader_assets DEPENDS ${RF_GENERATED_SHADER_HEADER})

add_library(roadforge SHARED
    src/core/FrameStats.cpp
    src/core/Log.cpp
    src/core/SimulationClock.cpp
    src/engine/Engine.cpp
    src/game/GameSimulation.cpp
    src/input/InputSystem.cpp
    src/physics/NullPhysicsWorld.cpp
    src/physics/RaycastVehicleBackend.cpp
    src/platform/android/JniBridge.cpp
    src/renderer/DebugMeshAssets.cpp
    src/renderer/Material.cpp
    src/renderer/VulkanRenderer.cpp
    src/renderer/VulkanResources.cpp
    src/vehicle/VehicleController.cpp
    src/vehicle/VehicleEvents.cpp
    src/world/World.cpp
)

add_dependencies(roadforge roadforge_shader_assets)

target_include_directories(roadforge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${RF_GENERATED_DIR}
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
