#include "Settings.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

using namespace geode::prelude;

namespace zaidfx {

BoolDefinition const* findBoolDefinition(std::string_view key) {
    for (auto const& definition : kBoolDefinitions) {
        if (definition.key == key) {
            return &definition;
        }
    }
    return nullptr;
}

FloatDefinition const* findFloatDefinition(std::string_view key) {
    for (auto const& definition : kFloatDefinitions) {
        if (definition.key == key) {
            return &definition;
        }
    }
    return nullptr;
}

Settings Settings::read() {
    Settings settings;
    auto* mod = Mod::get();

    settings.enabled = mod->getSettingValue<bool>("enabled");
    settings.quality = mod->getSettingValue<std::string>("quality");

    for (auto const& definition : kBoolDefinitions) {
        settings.booleans[index(definition.id)] =
            mod->getSettingValue<bool>(definition.key);
    }

    for (auto const& definition : kFloatDefinitions) {
        settings.floats[index(definition.id)] = static_cast<float>(
            mod->getSettingValue<double>(definition.key)
        );
    }

    settings.sanitize();
    return settings;
}

bool Settings::get(BoolParam id) const {
    return booleans[index(id)];
}

float Settings::get(FloatParam id) const {
    return floats[index(id)];
}

int Settings::qualityLevel() const {
    if (quality == "Low") {
        return 0;
    }
    if (quality == "High") {
        return 2;
    }
    if (quality == "Ultra") {
        return 3;
    }
    return 1;
}

float Settings::qualityScale() const {
    switch (qualityLevel()) {
        case 0: return 0.25f;
        case 1: return 0.375f;
        case 2: return 0.50f;
        case 3: return 0.75f;
        default: return 0.375f;
    }
}

bool Settings::hasLightingEffects() const {
    return get(BoolParam::BloomEnabled) ||
        get(BoolParam::EmissiveEnabled) ||
        get(BoolParam::AOEnabled) ||
        get(BoolParam::ReflectionsEnabled) ||
        get(BoolParam::LightRaysEnabled);
}

bool Settings::hasFinalEffects() const {
    auto const colorNeutral =
        std::abs(get(FloatParam::Exposure) - 50.0f) < 0.001f &&
        std::abs(get(FloatParam::Contrast) - 50.0f) < 0.001f &&
        std::abs(get(FloatParam::Saturation) - 50.0f) < 0.001f &&
        get(FloatParam::Vibrance) < 0.001f &&
        std::abs(get(FloatParam::Gamma) - 50.0f) < 0.001f &&
        std::abs(get(FloatParam::Temperature) - 50.0f) < 0.001f &&
        std::abs(get(FloatParam::Tint) - 50.0f) < 0.001f &&
        std::abs(get(FloatParam::Highlights) - 50.0f) < 0.001f &&
        std::abs(get(FloatParam::Shadows) - 50.0f) < 0.001f &&
        std::abs(get(FloatParam::ColorWhitePoint) - 100.0f) < 0.001f &&
        get(FloatParam::ColorBlackPoint) < 0.001f;

    return get(BoolParam::HDREnabled) ||
        get(BoolParam::LocalContrastEnabled) ||
        get(BoolParam::SpecularEnabled) ||
        get(BoolParam::DepthBlurEnabled) ||
        get(BoolParam::SharpenEnabled) ||
        get(BoolParam::ReactiveEnabled) ||
        get(FloatParam::Vignette) > 0.001f ||
        get(FloatParam::ChromaticAberration) > 0.001f ||
        get(FloatParam::FilmGrain) > 0.001f ||
        !colorNeutral;
}

bool Settings::hasVisibleEffects() const {
    return hasLightingEffects() || hasFinalEffects();
}

bool Settings::set(std::string_view key, bool value) {
    auto const* definition = findBoolDefinition(key);
    if (!definition) {
        return false;
    }

    booleans[index(definition->id)] = value;
    return true;
}

bool Settings::set(std::string_view key, float value) {
    auto const* definition = findFloatDefinition(key);
    if (!definition) {
        return false;
    }

    floats[index(definition->id)] = std::clamp(
        value,
        definition->minValue,
        definition->maxValue
    );
    return true;
}

bool Settings::setQuality(std::string value) {
    if (value != "Low" && value != "Medium" && value != "High" && value != "Ultra") {
        return false;
    }

    quality = std::move(value);
    return true;
}

void Settings::sanitize() {
    if (quality != "Low" && quality != "Medium" && quality != "High" && quality != "Ultra") {
        quality = "Medium";
    }

    for (auto const& definition : kFloatDefinitions) {
        auto& value = floats[index(definition.id)];
        value = std::clamp(value, definition.minValue, definition.maxValue);
    }

    if (get(FloatParam::ColorBlackPoint) >= get(FloatParam::ColorWhitePoint)) {
        floats[index(FloatParam::ColorBlackPoint)] =
            std::max(0.0f, get(FloatParam::ColorWhitePoint) - 1.0f);
    }

    if (get(FloatParam::HDRBlackPoint) >= get(FloatParam::HDRWhitePoint)) {
        floats[index(FloatParam::HDRBlackPoint)] =
            std::max(0.0f, get(FloatParam::HDRWhitePoint) - 1.0f);
    }
}

} // namespace zaidfx
