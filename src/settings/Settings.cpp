#include "Settings.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <utility>

using namespace geode::prelude;

namespace {

constexpr float kEffectEpsilon = 0.001f;
constexpr std::array<std::string_view, 4> kQualityNames {
    "Low",
    "Medium",
    "High",
    "Ultra"
};
constexpr std::array<float, 4> kQualityScales {
    0.25f,
    0.375f,
    0.50f,
    0.75f
};

template <class Definition, std::size_t Size>
Definition const* findDefinition(
    std::array<Definition, Size> const& definitions,
    std::string_view key
) {
    auto const match = std::find_if(
        definitions.begin(),
        definitions.end(),
        [key](Definition const& definition) {
            return definition.key == key;
        }
    );

    return match == definitions.end() ? nullptr : &*match;
}

bool isKnownQuality(std::string_view quality) {
    return std::find(kQualityNames.begin(), kQualityNames.end(), quality) !=
        kQualityNames.end();
}

bool isNear(float value, float expected) {
    return std::abs(value - expected) < kEffectEpsilon;
}

} // namespace

namespace zaidfx {

BoolDefinition const* findBoolDefinition(std::string_view key) {
    return findDefinition(kBoolDefinitions, key);
}

FloatDefinition const* findFloatDefinition(std::string_view key) {
    return findDefinition(kFloatDefinitions, key);
}

Settings Settings::read() {
    Settings settings;
    auto* mod = Mod::get();

    settings.enabled = mod->getSettingValue<bool>("enabled");
    settings.quality = mod->getSettingValue<std::string>("quality");

    for (auto const& setting : kBoolDefinitions) {
        settings.booleans[index(setting.id)] =
            mod->getSettingValue<bool>(setting.key);
    }

    for (auto const& setting : kFloatDefinitions) {
        settings.floats[index(setting.id)] = static_cast<float>(
            mod->getSettingValue<double>(setting.key)
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
    auto const match = std::find(kQualityNames.begin(), kQualityNames.end(), quality);
    if (match == kQualityNames.end()) {
        return 1;
    }

    return static_cast<int>(std::distance(kQualityNames.begin(), match));
}

float Settings::qualityScale() const {
    return kQualityScales[static_cast<std::size_t>(qualityLevel())];
}

bool Settings::hasLightingEffects() const {
    return get(BoolParam::BloomEnabled) ||
        get(BoolParam::EmissiveEnabled) ||
        get(BoolParam::AOEnabled) ||
        get(BoolParam::ReflectionsEnabled) ||
        get(BoolParam::LightRaysEnabled);
}

bool Settings::hasFinalEffects() const {
    auto const colorIsNeutral =
        isNear(get(FloatParam::Exposure), 50.0f) &&
        isNear(get(FloatParam::Contrast), 50.0f) &&
        isNear(get(FloatParam::Saturation), 50.0f) &&
        isNear(get(FloatParam::Vibrance), 0.0f) &&
        isNear(get(FloatParam::Gamma), 50.0f) &&
        isNear(get(FloatParam::Temperature), 50.0f) &&
        isNear(get(FloatParam::Tint), 50.0f) &&
        isNear(get(FloatParam::Highlights), 50.0f) &&
        isNear(get(FloatParam::Shadows), 50.0f) &&
        isNear(get(FloatParam::ColorWhitePoint), 100.0f) &&
        isNear(get(FloatParam::ColorBlackPoint), 0.0f);

    return get(BoolParam::HDREnabled) ||
        get(BoolParam::LocalContrastEnabled) ||
        get(BoolParam::SpecularEnabled) ||
        get(BoolParam::DepthBlurEnabled) ||
        get(BoolParam::SharpenEnabled) ||
        get(BoolParam::ReactiveEnabled) ||
        get(FloatParam::Vignette) > kEffectEpsilon ||
        get(FloatParam::ChromaticAberration) > kEffectEpsilon ||
        get(FloatParam::FilmGrain) > kEffectEpsilon ||
        !colorIsNeutral;
}

bool Settings::hasVisibleEffects() const {
    return hasLightingEffects() || hasFinalEffects();
}

bool Settings::set(std::string_view key, bool value) {
    auto const* setting = findBoolDefinition(key);
    if (!setting) {
        return false;
    }

    booleans[index(setting->id)] = value;
    return true;
}

bool Settings::set(std::string_view key, float value) {
    auto const* setting = findFloatDefinition(key);
    if (!setting) {
        return false;
    }

    floats[index(setting->id)] = std::clamp(
        value,
        setting->minValue,
        setting->maxValue
    );
    return true;
}

bool Settings::setQuality(std::string value) {
    if (!isKnownQuality(value)) {
        return false;
    }

    quality = std::move(value);
    return true;
}

void Settings::sanitize() {
    if (!isKnownQuality(quality)) {
        quality = "Medium";
    }

    for (auto const& setting : kFloatDefinitions) {
        auto& value = floats[index(setting.id)];
        value = std::clamp(value, setting.minValue, setting.maxValue);
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
