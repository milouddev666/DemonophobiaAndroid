cmake_minimum_required(VERSION 3.22)
project(demonophobia CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

# ─── android_native_app_glue ─────────────────────────────────────────────────
# Provided by the NDK; bridges NativeActivity lifecycle to android_main()
find_path(NATIVE_APP_GLUE_DIR android_native_app_glue.h
    PATHS ${ANDROID_NDK}/sources/android/native_app_glue)

if(NOT NATIVE_APP_GLUE_DIR)
    message(FATAL_ERROR "android_native_app_glue.h not found. Check ANDROID_NDK path.")
endif()

if(NOT TARGET android_native_app_glue)
    add_library(android_native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
    target_include_directories(android_native_app_glue PUBLIC ${NATIVE_APP_GLUE_DIR})
    target_link_libraries(android_native_app_glue log android)
endif()

# ─── Raylib ──────────────────────────────────────────────────────────────────
# Raylib is fetched from GitHub during the first build (requires internet).
# To build offline: clone raylib 5.5 into app/src/main/cpp/vendor/raylib/
# and set: cmake -DFETCHCONTENT_SOURCE_DIR_RAYLIB=<path>
include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   FALSE
)

# Raylib build options for Android GLES2 + miniaudio
set(BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES      OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD  ON  CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RAUDIO   ON  CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RSHAPES  ON  CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RTEXT    ON  CACHE BOOL "" FORCE)
set(SUPPORT_MODULE_RTEXTURES ON CACHE BOOL "" FORCE)
set(PLATFORM         "Android" CACHE STRING "" FORCE)
set(OPENGL_VERSION   "ES 2.0"  CACHE STRING "" FORCE)

FetchContent_MakeAvailable(raylib)

# ─── Game source files ───────────────────────────────────────────────────────

set(GAME_SOURCES
    main.cpp

    android/AndroidBridge.cpp

    engine/Engine.cpp
    engine/AssetManager.cpp
    engine/SceneManager.cpp
    engine/SaveSystem.cpp

    gameplay/GameState.cpp
    gameplay/Player.cpp
    gameplay/Room.cpp
    gameplay/Enemy.cpp
    gameplay/Trap.cpp
    gameplay/Inventory.cpp

    gameplay/scenes/TitleScene.cpp
    gameplay/scenes/GameplayScene.cpp
    gameplay/scenes/PauseScene.cpp
    gameplay/scenes/GameOverScene.cpp

    rendering/Renderer.cpp
    rendering/SpriteSheet.cpp
    rendering/Camera.cpp

    input/TouchInput.cpp
    input/VirtualGamepad.cpp

    audio/AudioManager.cpp
)

# ─── Shared library ──────────────────────────────────────────────────────────

add_library(demonophobia SHARED ${GAME_SOURCES})

target_include_directories(demonophobia PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NATIVE_APP_GLUE_DIR}
)

# ─── Android system libraries ────────────────────────────────────────────────

find_library(lib_log     log)
find_library(lib_android android)
find_library(lib_EGL     EGL)
find_library(lib_GLES2   GLESv2)
find_library(lib_OpenSLES OpenSLES)

target_link_libraries(demonophobia
    android_native_app_glue
    raylib
    ${lib_log}
    ${lib_android}
    ${lib_EGL}
    ${lib_GLES2}
    ${lib_OpenSLES}
    dl
    m
    z      # for miniaudio's compression support
)

# ─── Compiler flags ──────────────────────────────────────────────────────────

target_compile_options(demonophobia PRIVATE
    -Wall -Wextra -Wno-unused-parameter
    -O2 -ffast-math
    -fvisibility=hidden
    -DANDROID
    -DPLATFORM_ANDROID
    -DGRAPHICS_API_OPENGL_ES2
)

if(ANDROID_ABI STREQUAL "armeabi-v7a")
    target_compile_options(demonophobia PRIVATE -mfpu=neon -mfloat-abi=softfp)
    message(STATUS "ARMv7 + NEON enabled")
elseif(ANDROID_ABI STREQUAL "arm64-v8a")
    message(STATUS "ARM64 build")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(demonophobia PRIVATE -DDEBUG_BUILD -g)
endif()

message(STATUS "ABI: ${ANDROID_ABI}")
message(STATUS "NDK: ${ANDROID_NDK}")
