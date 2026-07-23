#include <Geode/Geode.hpp>
#include <Geode/cocos/platform/android/jni/JniHelper.h>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCDirector.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

using namespace geode::prelude;

namespace {

constexpr std::int64_t kMinimumFPS = 30;
constexpr std::int64_t kMaximumFPS = 360;
constexpr double kFallbackInterval = 1.0 / 60.0;
constexpr float kModeMatchTolerance = 0.75f;

bool s_applyingInterval = false;
double s_gameRequestedInterval = kFallbackInterval;

struct DisplayModeChoice {
    jint id = 0;
    jfloat refreshRate = 0.0f;
    jint width = 0;
    jint height = 0;
    bool valid = false;
};

bool isValidInterval(double interval) {
    return std::isfinite(interval) && interval > 0.0 && interval <= 1.0;
}

bool isBypassEnabled() {
    return Mod::get()->getSettingValue<bool>("enabled");
}

std::int64_t targetFPS() {
    return std::clamp(
        Mod::get()->getSettingValue<std::int64_t>("target-fps"),
        kMinimumFPS,
        kMaximumFPS
    );
}

double targetInterval() {
    return 1.0 / static_cast<double>(targetFPS());
}

bool clearJNIException(JNIEnv* env, char const* operation) {
    if (!env || !env->ExceptionCheck()) {
        return false;
    }

    env->ExceptionClear();
    log::warn("[ZaidFPS] Android rejected JNI operation: {}", operation);
    return true;
}

JNIEnv* getJNIEnv(bool& attached) {
    attached = false;

    auto* vm = cocos2d::JniHelper::getJavaVM();
    if (!vm) {
        return nullptr;
    }

    JNIEnv* env = nullptr;
    auto const status = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (status == JNI_OK) {
        return env;
    }

    if (status == JNI_EDETACHED &&
        vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
        attached = true;
        return env;
    }

    return nullptr;
}

jobject getActivity(JNIEnv* env) {
    auto activityHolderClass = cocos2d::JniHelper::getClassID(
        "com/customRobTop/BaseRobTopActivity",
        env
    );
    if (!activityHolderClass || clearJNIException(env, "find activity holder")) {
        return nullptr;
    }

    auto instanceField = env->GetStaticFieldID(
        activityHolderClass,
        "INSTANCE",
        "Lcom/customRobTop/BaseRobTopActivity;"
    );
    auto holderInstance = instanceField
        ? env->GetStaticObjectField(activityHolderClass, instanceField)
        : nullptr;
    auto getReference = env->GetMethodID(
        activityHolderClass,
        "getMe",
        "()Ljava/lang/ref/WeakReference;"
    );
    auto weakReference = holderInstance && getReference
        ? env->CallObjectMethod(holderInstance, getReference)
        : nullptr;

    if (clearJNIException(env, "get Android activity") || !weakReference) {
        if (holderInstance) env->DeleteLocalRef(holderInstance);
        env->DeleteLocalRef(activityHolderClass);
        return nullptr;
    }

    auto weakReferenceClass = env->FindClass("java/lang/ref/Reference");
    auto getObject = weakReferenceClass
        ? env->GetMethodID(weakReferenceClass, "get", "()Ljava/lang/Object;")
        : nullptr;
    auto activity = getObject
        ? env->CallObjectMethod(weakReference, getObject)
        : nullptr;

    clearJNIException(env, "dereference Android activity");

    if (weakReferenceClass) env->DeleteLocalRef(weakReferenceClass);
    env->DeleteLocalRef(weakReference);
    if (holderInstance) env->DeleteLocalRef(holderInstance);
    env->DeleteLocalRef(activityHolderClass);
    return activity;
}

jint getAndroidSDK(JNIEnv* env) {
    auto versionClass = env->FindClass("android/os/Build$VERSION");
    if (!versionClass) {
        clearJNIException(env, "find Android version");
        return 0;
    }

    auto sdkField = env->GetStaticFieldID(versionClass, "SDK_INT", "I");
    auto sdk = sdkField ? env->GetStaticIntField(versionClass, sdkField) : 0;
    clearJNIException(env, "read Android version");
    env->DeleteLocalRef(versionClass);
    return sdk;
}

DisplayModeChoice chooseDisplayMode(JNIEnv* env, jobject display, float requestedRate) {
    DisplayModeChoice choice;
    if (!display) {
        return choice;
    }

    auto displayClass = env->GetObjectClass(display);
    auto getModes = env->GetMethodID(
        displayClass,
        "getSupportedModes",
        "()[Landroid/view/Display$Mode;"
    );
    auto getCurrentMode = env->GetMethodID(
        displayClass,
        "getMode",
        "()Landroid/view/Display$Mode;"
    );

    auto modes = getModes
        ? static_cast<jobjectArray>(env->CallObjectMethod(display, getModes))
        : nullptr;
    auto currentMode = getCurrentMode
        ? env->CallObjectMethod(display, getCurrentMode)
        : nullptr;

    if (clearJNIException(env, "read display modes") || !modes) {
        if (currentMode) env->DeleteLocalRef(currentMode);
        env->DeleteLocalRef(displayClass);
        return choice;
    }

    auto modeClass = env->FindClass("android/view/Display$Mode");
    auto getModeID = modeClass
        ? env->GetMethodID(modeClass, "getModeId", "()I")
        : nullptr;
    auto getRefreshRate = modeClass
        ? env->GetMethodID(modeClass, "getRefreshRate", "()F")
        : nullptr;
    auto getWidth = modeClass
        ? env->GetMethodID(modeClass, "getPhysicalWidth", "()I")
        : nullptr;
    auto getHeight = modeClass
        ? env->GetMethodID(modeClass, "getPhysicalHeight", "()I")
        : nullptr;

    if (!modeClass || !getModeID || !getRefreshRate || !getWidth || !getHeight ||
        clearJNIException(env, "inspect display mode API")) {
        if (modeClass) env->DeleteLocalRef(modeClass);
        if (currentMode) env->DeleteLocalRef(currentMode);
        env->DeleteLocalRef(modes);
        env->DeleteLocalRef(displayClass);
        return choice;
    }

    jint currentWidth = 0;
    jint currentHeight = 0;
    if (currentMode) {
        currentWidth = env->CallIntMethod(currentMode, getWidth);
        currentHeight = env->CallIntMethod(currentMode, getHeight);
        clearJNIException(env, "read current display resolution");
    }

    float bestScore = std::numeric_limits<float>::max();
    auto const modeCount = env->GetArrayLength(modes);

    for (jsize index = 0; index < modeCount; ++index) {
        auto mode = env->GetObjectArrayElement(modes, index);
        if (!mode) {
            continue;
        }

        auto const refreshRate = env->CallFloatMethod(mode, getRefreshRate);
        auto const width = env->CallIntMethod(mode, getWidth);
        auto const height = env->CallIntMethod(mode, getHeight);
        auto const id = env->CallIntMethod(mode, getModeID);

        if (clearJNIException(env, "read supported display mode")) {
            env->DeleteLocalRef(mode);
            continue;
        }

        auto const sameResolution =
            currentWidth > 0 && currentHeight > 0 &&
            width == currentWidth && height == currentHeight;
        auto const rateDifference = std::fabs(refreshRate - requestedRate);
        auto const resolutionTieBreaker = sameResolution ? 0.0f : 1.0f;
        auto const score = rateDifference * 1000.0f + resolutionTieBreaker;

        if (!choice.valid || score < bestScore) {
            choice = { id, refreshRate, width, height, true };
            bestScore = score;
        }

        env->DeleteLocalRef(mode);
    }

    env->DeleteLocalRef(modeClass);
    if (currentMode) env->DeleteLocalRef(currentMode);
    env->DeleteLocalRef(modes);
    env->DeleteLocalRef(displayClass);
    return choice;
}

jobject getGLSurface(JNIEnv* env) {
    auto viewClass = cocos2d::JniHelper::getClassID(
        "org/cocos2dx/lib/Cocos2dxGLSurfaceView",
        env
    );
    if (!viewClass || clearJNIException(env, "find GLSurfaceView")) {
        return nullptr;
    }

    auto companionField = env->GetStaticFieldID(
        viewClass,
        "Companion",
        "Lorg/cocos2dx/lib/Cocos2dxGLSurfaceView$Companion;"
    );
    auto companion = companionField
        ? env->GetStaticObjectField(viewClass, companionField)
        : nullptr;
    auto companionClass = companion ? env->GetObjectClass(companion) : nullptr;
    auto getView = companionClass
        ? env->GetMethodID(
            companionClass,
            "getCocos2dxGLSurfaceView",
            "()Lorg/cocos2dx/lib/Cocos2dxGLSurfaceView;"
        )
        : nullptr;
    auto view = getView ? env->CallObjectMethod(companion, getView) : nullptr;

    if (clearJNIException(env, "get GLSurfaceView") || !view) {
        if (companionClass) env->DeleteLocalRef(companionClass);
        if (companion) env->DeleteLocalRef(companion);
        env->DeleteLocalRef(viewClass);
        return nullptr;
    }

    auto viewObjectClass = env->GetObjectClass(view);
    auto getHolder = env->GetMethodID(
        viewObjectClass,
        "getHolder",
        "()Landroid/view/SurfaceHolder;"
    );
    auto holder = getHolder ? env->CallObjectMethod(view, getHolder) : nullptr;
    auto holderClass = holder ? env->GetObjectClass(holder) : nullptr;
    auto getSurface = holderClass
        ? env->GetMethodID(holderClass, "getSurface", "()Landroid/view/Surface;")
        : nullptr;
    auto surface = getSurface ? env->CallObjectMethod(holder, getSurface) : nullptr;

    clearJNIException(env, "get render surface");

    if (holderClass) env->DeleteLocalRef(holderClass);
    if (holder) env->DeleteLocalRef(holder);
    env->DeleteLocalRef(viewObjectClass);
    env->DeleteLocalRef(view);
    if (companionClass) env->DeleteLocalRef(companionClass);
    if (companion) env->DeleteLocalRef(companion);
    env->DeleteLocalRef(viewClass);
    return surface;
}

float requestAndroidFrameRate(bool enabled) {
    bool attached = false;
    auto* env = getJNIEnv(attached);
    auto* vm = cocos2d::JniHelper::getJavaVM();
    if (!env || !vm) {
        log::warn("[ZaidFPS] Android JNI environment is unavailable");
        return 0.0f;
    }

    env->PushLocalFrame(96);

    auto activity = getActivity(env);
    if (!activity) {
        env->PopLocalFrame(nullptr);
        if (attached) vm->DetachCurrentThread();
        log::warn("[ZaidFPS] Android activity is unavailable");
        return 0.0f;
    }

    auto activityClass = env->GetObjectClass(activity);
    auto getWindowManager = env->GetMethodID(
        activityClass,
        "getWindowManager",
        "()Landroid/view/WindowManager;"
    );
    auto windowManager = getWindowManager
        ? env->CallObjectMethod(activity, getWindowManager)
        : nullptr;
    auto windowManagerClass = windowManager
        ? env->GetObjectClass(windowManager)
        : nullptr;
    auto getDefaultDisplay = windowManagerClass
        ? env->GetMethodID(
            windowManagerClass,
            "getDefaultDisplay",
            "()Landroid/view/Display;"
        )
        : nullptr;
    auto display = getDefaultDisplay
        ? env->CallObjectMethod(windowManager, getDefaultDisplay)
        : nullptr;

    auto const requestedRate = static_cast<float>(targetFPS());
    auto const mode = enabled
        ? chooseDisplayMode(env, display, requestedRate)
        : DisplayModeChoice {};

    auto getWindow = env->GetMethodID(
        activityClass,
        "getWindow",
        "()Landroid/view/Window;"
    );
    auto window = getWindow ? env->CallObjectMethod(activity, getWindow) : nullptr;
    auto windowClass = window ? env->GetObjectClass(window) : nullptr;
    auto getAttributes = windowClass
        ? env->GetMethodID(
            windowClass,
            "getAttributes",
            "()Landroid/view/WindowManager$LayoutParams;"
        )
        : nullptr;
    auto setAttributes = windowClass
        ? env->GetMethodID(
            windowClass,
            "setAttributes",
            "(Landroid/view/WindowManager$LayoutParams;)V"
        )
        : nullptr;
    auto attributes = getAttributes
        ? env->CallObjectMethod(window, getAttributes)
        : nullptr;
    auto attributesClass = attributes ? env->GetObjectClass(attributes) : nullptr;

    if (attributesClass) {
        auto modeField = env->GetFieldID(attributesClass, "preferredDisplayModeId", "I");
        auto rateField = env->GetFieldID(attributesClass, "preferredRefreshRate", "F");

        if (modeField) {
            env->SetIntField(attributes, modeField, enabled && mode.valid ? mode.id : 0);
        }
        if (rateField) {
            env->SetFloatField(
                attributes,
                rateField,
                enabled ? (mode.valid ? mode.refreshRate : requestedRate) : 0.0f
            );
        }

        if (setAttributes) {
            env->CallVoidMethod(window, setAttributes, attributes);
        }
        clearJNIException(env, "apply window display mode");
    }

    auto surface = getGLSurface(env);
    if (surface) {
        auto surfaceClass = env->GetObjectClass(surface);
        auto isValid = env->GetMethodID(surfaceClass, "isValid", "()Z");
        auto const valid = !isValid || env->CallBooleanMethod(surface, isValid) == JNI_TRUE;
        auto const sdk = getAndroidSDK(env);
        auto const surfaceRate = enabled
            ? (mode.valid ? mode.refreshRate : requestedRate)
            : 0.0f;

        if (valid && sdk >= 31) {
            auto setFrameRate = env->GetMethodID(surfaceClass, "setFrameRate", "(FII)V");
            if (setFrameRate) {
                env->CallVoidMethod(surface, setFrameRate, surfaceRate, 0, 1);
            }
        } else if (valid && sdk >= 30) {
            auto setFrameRate = env->GetMethodID(surfaceClass, "setFrameRate", "(FI)V");
            if (setFrameRate) {
                env->CallVoidMethod(surface, setFrameRate, surfaceRate, 0);
            }
        }

        clearJNIException(env, "apply surface frame rate");
        env->DeleteLocalRef(surfaceClass);
        env->DeleteLocalRef(surface);
    }

    auto const selectedRate = enabled
        ? (mode.valid ? mode.refreshRate : requestedRate)
        : 0.0f;

    env->PopLocalFrame(nullptr);
    if (attached) {
        vm->DetachCurrentThread();
    }

    if (enabled && mode.valid) {
        log::info(
            "[ZaidFPS] requested {} FPS; selected Android mode {}x{} @ {:.2f} Hz (mode {})",
            targetFPS(),
            mode.width,
            mode.height,
            mode.refreshRate,
            mode.id
        );
        if (std::fabs(mode.refreshRate - requestedRate) > kModeMatchTolerance) {
            log::warn(
                "[ZaidFPS] the display did not expose an exact {:.0f} Hz mode; using {:.2f} Hz",
                requestedRate,
                mode.refreshRate
            );
        }
    }

    return selectedRate;
}

void applyCurrentSetting() {
    auto* director = cocos2d::CCDirector::sharedDirector();
    if (!director) {
        return;
    }

    auto const enabled = isBypassEnabled();
    auto const interval = enabled ? targetInterval() : s_gameRequestedInterval;

    s_applyingInterval = true;
    director->setAnimationInterval(interval);
    s_applyingInterval = false;

    auto const displayRate = requestAndroidFrameRate(enabled);

    log::info(
        "[ZaidFPS] {}: game target {} FPS, Android request {:.2f} Hz",
        enabled ? "enabled" : "disabled",
        enabled
            ? targetFPS()
            : static_cast<std::int64_t>(std::lround(1.0 / interval)),
        displayRate
    );
}

void queueApply() {
    geode::queueInMainThread([] {
        applyCurrentSetting();
    });
}

} // namespace

class $modify(ZaidFPSDirector, cocos2d::CCDirector) {
    void setAnimationInterval(double interval) {
        if (!s_applyingInterval && isValidInterval(interval)) {
            s_gameRequestedInterval = interval;
        }

        auto const appliedInterval =
            !s_applyingInterval && isBypassEnabled()
                ? targetInterval()
                : interval;

        cocos2d::CCDirector::setAnimationInterval(appliedInterval);

        if (!s_applyingInterval && isBypassEnabled()) {
            requestAndroidFrameRate(true);
        }
    }
};

$execute {
    if (auto* director = cocos2d::CCDirector::sharedDirector()) {
        auto const currentInterval = director->getAnimationInterval();
        if (isValidInterval(currentInterval)) {
            s_gameRequestedInterval = currentInterval;
        }
    }

    listenForSettingChanges<bool>("enabled", [](bool) {
        queueApply();
    });

    listenForSettingChanges<std::int64_t>("target-fps", [](std::int64_t) {
        if (isBypassEnabled()) {
            queueApply();
        }
    });

    queueApply();
}
