#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCDirector.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

using namespace geode::prelude;

namespace {

constexpr std::int64_t kMinFps = 30;
constexpr std::int64_t kMaxFps = 360;
constexpr double kDefaultInterval = 1.0 / 60.0;

bool s_isApplyingOverride = false;
double s_lastGameInterval = kDefaultInterval;

bool isUsableInterval(double interval) {
    return std::isfinite(interval) && interval > 0.0 && interval <= 1.0;
}

bool bypassEnabled() {
    return Mod::get()->getSettingValue<bool>("enabled");
}

std::int64_t requestedFps() {
    auto const configuredFps =
        Mod::get()->getSettingValue<std::int64_t>("target-fps");
    return std::clamp(configuredFps, kMinFps, kMaxFps);
}

double intervalFor(std::int64_t fps) {
    return 1.0 / static_cast<double>(fps);
}

std::int64_t fpsFor(double interval) {
    return static_cast<std::int64_t>(std::lround(1.0 / interval));
}

void applySettings() {
    auto* director = cocos2d::CCDirector::sharedDirector();
    if (!director) {
        return;
    }

    auto const enabled = bypassEnabled();
    auto const fps = requestedFps();
    auto const interval = enabled ? intervalFor(fps) : s_lastGameInterval;

    s_isApplyingOverride = true;
    director->setAnimationInterval(interval);
    s_isApplyingOverride = false;

    log::info(
        "[ZaidFPS] {} at {} FPS",
        enabled ? "enabled" : "disabled",
        enabled ? fps : fpsFor(interval)
    );
}

void scheduleApply() {
    geode::queueInMainThread([] {
        applySettings();
    });
}

} // namespace

class $modify(ZaidFPSDirector, cocos2d::CCDirector) {
    void setAnimationInterval(double interval) {
        if (!s_isApplyingOverride && isUsableInterval(interval)) {
            s_lastGameInterval = interval;
        }

        auto const shouldOverride = !s_isApplyingOverride && bypassEnabled();
        cocos2d::CCDirector::setAnimationInterval(
            shouldOverride ? intervalFor(requestedFps()) : interval
        );
    }
};

$execute {
    if (auto* director = cocos2d::CCDirector::sharedDirector()) {
        auto const currentInterval = director->getAnimationInterval();
        if (isUsableInterval(currentInterval)) {
            s_lastGameInterval = currentInterval;
        }
    }

    listenForSettingChanges<bool>("enabled", [](bool) {
        scheduleApply();
    });

    listenForSettingChanges<std::int64_t>("target-fps", [](std::int64_t) {
        if (bypassEnabled()) {
            scheduleApply();
        }
    });

    scheduleApply();
}
