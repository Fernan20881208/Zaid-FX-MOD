#include "Settings.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace zaidfx {

Settings Settings::read() {
    auto* mod = Mod::get();

    Settings settings {
        .enabled = mod->getSettingValue<bool>("enabled"),
        .intensity = static_cast<float>(mod->getSettingValue<double>("effect-intensity")),
        .brightness = static_cast<float>(mod->getSettingValue<double>("brightness")),
        .exposure = static_cast<float>(mod->getSettingValue<double>("exposure")),
        .contrast = static_cast<float>(mod->getSettingValue<double>("contrast")),
        .saturation = static_cast<float>(mod->getSettingValue<double>("saturation")),
        .gamma = static_cast<float>(mod->getSettingValue<double>("gamma")),
        .bloom = static_cast<float>(mod->getSettingValue<double>("bloom")),
        .vignette = static_cast<float>(mod->getSettingValue<double>("vignette")),
        .sharpen = static_cast<float>(mod->getSettingValue<double>("sharpen")),
        .chromaticAberration = static_cast<float>(mod->getSettingValue<double>("chromatic-aberration")),
        .tonemapping = static_cast<float>(mod->getSettingValue<double>("tonemapping"))
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
    bloom = std::clamp(bloom, 0.0f, 1.0f);
    vignette = std::clamp(vignette, 0.0f, 1.0f);
    sharpen = std::clamp(sharpen, 0.0f, 1.0f);
    chromaticAberration = std::clamp(chromaticAberration, 0.0f, 1.0f);
    tonemapping = std::clamp(tonemapping, 0.0f, 1.0f);
}

} // namespace zaidfx
