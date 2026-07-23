#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCDirector.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>

using namespace geode::prelude;

namespace {

constexpr std::int64_t kMinimumFPS = 30;
constexpr std::int64_t kMaximumFPS = 360;
constexpr double kFallbackInterval = 1.0 / 60.0;

bool s_applyingInterval = false;
double s_gameRequestedInterval = kFallbackInterval;

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

void applyCurrentSetting() {
    auto* director = cocos2d::CCDirector::sharedDirector();
    if (!director) {
        return;
    }

    auto const interval = isBypassEnabled()
        ? targetInterval()
        : s_gameRequestedInterval;

    s_applyingInterval = true;
    director->setAnimationInterval(interval);
    s_applyingInterval = false;

    log::info(
        "[ZaidFPS] {} at {} FPS",
        isBypassEnabled() ? "enabled" : "disabled",
        isBypassEnabled()
            ? targetFPS()
            : static_cast<std::int64_t>(std::lround(1.0 / interval))
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
