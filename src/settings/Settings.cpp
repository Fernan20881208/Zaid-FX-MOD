#include "Settings.hpp"

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace zaidfx {

Settings Settings::read() {
    auto* mod = Mod::get();

    return Settings {
        .enabled = mod->getSettingValue<bool>("enabled"),
        .preset = mod->getSettingValue<std::string>("preset"),
        .exposure = static_cast<float>(mod->getSettingValue<double>("exposure")),
        .contrast = static_cast<float>(mod->getSettingValue<double>("contrast")),
        .saturation = static_cast<float>(mod->getSettingValue<double>("saturation")),
        .gamma = static_cast<float>(mod->getSettingValue<double>("gamma")),
        .vignette = static_cast<float>(mod->getSettingValue<double>("vignette")),
        .sharpen = static_cast<float>(mod->getSettingValue<double>("sharpen"))
    };
}

} // namespace zaidfx
