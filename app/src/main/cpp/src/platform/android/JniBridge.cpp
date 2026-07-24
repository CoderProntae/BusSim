#include "roadforge/engine/Engine.hpp"
#include "roadforge/core/Log.hpp"

#include <android/native_window_jni.h>
#include <jni.h>

#include <cstdint>
#include <new>

namespace {

using roadforge::engine::Engine;

Engine* fromHandle(jlong handle) {
    return reinterpret_cast<Engine*>(handle);
}

} // namespace

extern "C" JNIEXPORT jlong JNICALL
Java_com_roadforge_bussim_MainActivity_nativeCreate(JNIEnv* env, jclass clazz) {
    (void)env;
    (void)clazz;
    try {
        return reinterpret_cast<jlong>(new Engine());
    } catch (const std::bad_alloc&) {
        RF_LOGE("Failed to allocate Engine");
        return 0;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeDestroy(JNIEnv* env, jclass clazz, jlong handle) {
    (void)env;
    (void)clazz;
    delete fromHandle(handle);
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeSurfaceCreated(JNIEnv* env, jclass clazz, jlong handle, jobject surface) {
    (void)clazz;
    Engine* engine = fromHandle(handle);
    if (engine == nullptr || surface == nullptr) {
        return;
    }

    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        RF_LOGE("ANativeWindow_fromSurface returned null");
        return;
    }

    // Engine bu referansın sahipliğini alır ve surfaceDestroyed/destroy anında release eder.
    engine->onSurfaceCreated(window);
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeSurfaceChanged(JNIEnv* env, jclass clazz, jlong handle, jint width, jint height) {
    (void)env;
    (void)clazz;
    Engine* engine = fromHandle(handle);
    if (engine != nullptr) {
        engine->onSurfaceChanged(static_cast<int32_t>(width), static_cast<int32_t>(height));
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeSurfaceDestroyed(JNIEnv* env, jclass clazz, jlong handle) {
    (void)env;
    (void)clazz;
    Engine* engine = fromHandle(handle);
    if (engine != nullptr) {
        engine->onSurfaceDestroyed();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeOnPause(JNIEnv* env, jclass clazz, jlong handle) {
    (void)env;
    (void)clazz;
    Engine* engine = fromHandle(handle);
    if (engine != nullptr) {
        engine->onPause();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeOnResume(JNIEnv* env, jclass clazz, jlong handle) {
    (void)env;
    (void)clazz;
    Engine* engine = fromHandle(handle);
    if (engine != nullptr) {
        engine->onResume();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeTouch(JNIEnv* env, jclass clazz, jlong handle, jint action, jfloat x, jfloat y, jint pointerCount) {
    (void)env;
    (void)clazz;
    Engine* engine = fromHandle(handle);
    if (engine != nullptr) {
        engine->onTouch(static_cast<int32_t>(action), x, y, static_cast<int32_t>(pointerCount));
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_roadforge_bussim_MainActivity_nativeFrame(JNIEnv* env, jclass clazz, jlong handle, jlong frameTimeNanos) {
    (void)env;
    (void)clazz;
    Engine* engine = fromHandle(handle);
    if (engine != nullptr) {
        engine->frame(static_cast<int64_t>(frameTimeNanos));
    }
}
