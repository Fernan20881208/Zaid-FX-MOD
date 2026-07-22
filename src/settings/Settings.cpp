#include "Settings.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace zaidfx {

Settings Settings::read() {
    auto* mod = Mod::get();

    Settings settings {
        .enabled = mod->getSettingValue<bool>("enabled"),
        .debugLogging = mod->getSettingValue<bool>("debug-logging"),
        .debugRedScreen = mod->getSettingValue<bool>("debug-red-screen"),
        .preset = mod->getSettingValue<std::string>("preset"),
        .intensity = static_cast<float>(mod->getSettingValue<double>("effect-intensity")),
        .brightness = static_cast<float>(mod->getSettingValue<double>("brightness")),
        .exposure = static_cast<float>(mod->getSettingValue<double>("exposure")),
        .contrast = static_cast<float>(mod->getSettingValue<double>("contrast")),
        .saturation = static_cast<float>(mod->getSettingValue<double>("saturation")),
        .gamma = static_cast<float>(mod->getSettingValue<double>("gamma")),
        .vignette = static_cast<float>(mod->getSettingValue<double>("vignette")),
        .sharpen = static_cast<float>(mod->getSettingValue<double>("sharpen"))
    };

    settings.sanitize();
    return settings;
}

void Settings::sanitize() {
    intensity = std::clamp(intensity, 0.0f, 1.0f);
    brightness = std::clamp(brightness, -1.0f, 1.0f);
    exposure = std::clamp(exposure, -2.0f, 2.0f);
    contrast = std::clamp(contrast, 0.25f, 3.0f);
    saturation = std::clamp(saturation, 0.0f, 3.0f);
    gamma = std::clamp(gamma, 0.25f, 3.0f);
    vignette = std::clamp(vignette, 0.0f, 1.0f);
    sharpen = std::clamp(sharpen, 0.0f, 1.0f);
}

} // namespace zaidfx
