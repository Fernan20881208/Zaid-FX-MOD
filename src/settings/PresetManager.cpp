#include "PresetManager.hpp"

#include "../rendering/PostProcessRenderer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/FLAlertLayer.hpp>

#include <algorithm>
#include <initializer_list>
#include <utility>

using namespace geode::prelude;

namespace zaidfx {
namespace {

using BoolValue = std::pair<BoolParam, bool>;
using FloatValue = std::pair<FloatParam, float>;

void setValues(
    Preset& preset,
    std::initializer_list<BoolValue> booleans,
    std::initializer_list<FloatValue> floats
) {
    for (auto const& [id, value] : booleans) {
        preset.booleans[index(id)] = value;
    }
    for (auto const& [id, value] : floats) {
        preset.floats[index(id)] = value;
    }
}

std::string customKey(std::string_view kind, std::string_view key) {
    return fmt::format("custom-{}-{}", kind, key);
}

} // namespace

PresetManager& PresetManager::get() {
    static PresetManager instance;
    return instance;
}

PresetManager::PresetManager() {
    auto defaults = makeBasePreset("Default");
    defaults.enabled = false;
    defaults.quality = "Medium";
    m_presets.push_back(defaults);

    auto glow = makeBasePreset("Glow");
    glow.quality = "High";
    setValues(glow, {
        { BoolParam::BloomEnabled, true },
        { BoolParam::EmissiveEnabled, true },
        { BoolParam::SharpenEnabled, true },
    }, {
        { FloatParam::BloomIntensity, 48.0f },
        { FloatParam::BloomThreshold, 62.0f },
        { FloatParam::BloomRadius, 34.0f },
        { FloatParam::BloomSoftKnee, 45.0f },
        { FloatParam::BloomQuality, 55.0f },
        { FloatParam::EmissiveIntensity, 25.0f },
        { FloatParam::EmissiveThreshold, 68.0f },
        { FloatParam::EmissiveRadius, 28.0f },
        { FloatParam::EmissiveColorBoost, 22.0f },
        { FloatParam::Contrast, 54.0f },
        { FloatParam::Saturation, 56.0f },
        { FloatParam::SharpenStrength, 10.0f },
        { FloatParam::SharpenRadius, 14.0f },
        { FloatParam::SharpenThreshold, 24.0f },
    });
    m_presets.push_back(glow);

    auto zaidLux = makeBasePreset("ZaidLux");
    zaidLux.quality = "High";
    setValues(zaidLux, {
        { BoolParam::BloomEnabled, true },
        { BoolParam::EmissiveEnabled, true },
        { BoolParam::AOEnabled, true },
        { BoolParam::ReflectionsEnabled, true },
        { BoolParam::HDREnabled, true },
        { BoolParam::LocalContrastEnabled, true },
        { BoolParam::SpecularEnabled, true },
        { BoolParam::LightRaysEnabled, true },
        { BoolParam::DepthBlurEnabled, true },
        { BoolParam::SharpenEnabled, true },
    }, {
        { FloatParam::BloomIntensity, 54.0f },
        { FloatParam::BloomThreshold, 72.0f },
        { FloatParam::BloomRadius, 28.0f },
        { FloatParam::BloomSoftKnee, 38.0f },
        { FloatParam::BloomQuality, 66.0f },
        { FloatParam::EmissiveIntensity, 68.0f },
        { FloatParam::EmissiveThreshold, 63.0f },
        { FloatParam::EmissiveRadius, 38.0f },
        { FloatParam::EmissiveColorBoost, 42.0f },
        { FloatParam::AOIntensity, 25.0f },
        { FloatParam::AORadius, 18.0f },
        { FloatParam::AODarkness, 30.0f },
        { FloatParam::AOQuality, 58.0f },
        { FloatParam::ReflectionIntensity, 28.0f },
        { FloatParam::ReflectionOpacity, 22.0f },
        { FloatParam::ReflectionBlur, 34.0f },
        { FloatParam::ReflectionDistance, 18.0f },
        { FloatParam::ReflectionDistortion, 8.0f },
        { FloatParam::DynamicRange, 65.0f },
        { FloatParam::HighlightCompression, 35.0f },
        { FloatParam::ShadowRecovery, 18.0f },
        { FloatParam::HDRWhitePoint, 82.0f },
        { FloatParam::HDRBlackPoint, 12.0f },
        { FloatParam::LocalContrastStrength, 32.0f },
        { FloatParam::DetailStrength, 25.0f },
        { FloatParam::DetailRadius, 20.0f },
        { FloatParam::SpecularIntensity, 28.0f },
        { FloatParam::SpecularSize, 18.0f },
        { FloatParam::SpecularThreshold, 72.0f },
        { FloatParam::MetallicLook, 18.0f },
        { FloatParam::GlassLook, 12.0f },
        { FloatParam::RaysIntensity, 14.0f },
        { FloatParam::RaysLength, 20.0f },
        { FloatParam::RaysBlur, 24.0f },
        { FloatParam::RaysDecay, 40.0f },
        { FloatParam::RaysThreshold, 78.0f },
        { FloatParam::RaysCenterX, 50.0f },
        { FloatParam::RaysCenterY, 54.0f },
        { FloatParam::BackgroundBlur, 5.0f },
        { FloatParam::ForegroundSharpness, 75.0f },
        { FloatParam::DepthSeparation, 18.0f },
        { FloatParam::BlurTransition, 35.0f },
        { FloatParam::SharpenStrength, 29.0f },
        { FloatParam::SharpenRadius, 18.0f },
        { FloatParam::SharpenThreshold, 22.0f },
        { FloatParam::Exposure, 56.0f },
        { FloatParam::Contrast, 68.0f },
        { FloatParam::Saturation, 61.0f },
        { FloatParam::Vibrance, 26.0f },
        { FloatParam::Gamma, 47.0f },
        { FloatParam::Temperature, 50.0f },
        { FloatParam::Tint, 50.0f },
        { FloatParam::Highlights, 84.0f },
        { FloatParam::Shadows, 38.0f },
        { FloatParam::Vignette, 11.0f },
        { FloatParam::ChromaticAberration, 2.0f },
        { FloatParam::FilmGrain, 1.0f },
    });
    m_presets.push_back(zaidLux);

    auto neon = zaidLux;
    neon.name = "ZaidLux Neon";
    neon.quality = "Ultra";
    setValues(neon, {}, {
        { FloatParam::BloomIntensity, 65.0f },
        { FloatParam::BloomThreshold, 64.0f },
        { FloatParam::BloomRadius, 36.0f },
        { FloatParam::EmissiveIntensity, 82.0f },
        { FloatParam::EmissiveThreshold, 55.0f },
        { FloatParam::EmissiveColorBoost, 68.0f },
        { FloatParam::AOIntensity, 22.0f },
        { FloatParam::ReflectionIntensity, 34.0f },
        { FloatParam::ReflectionOpacity, 28.0f },
        { FloatParam::DynamicRange, 72.0f },
        { FloatParam::LocalContrastStrength, 36.0f },
        { FloatParam::SpecularIntensity, 35.0f },
        { FloatParam::MetallicLook, 28.0f },
        { FloatParam::GlassLook, 24.0f },
        { FloatParam::RaysIntensity, 18.0f },
        { FloatParam::Exposure, 57.0f },
        { FloatParam::Contrast, 72.0f },
        { FloatParam::Saturation, 72.0f },
        { FloatParam::Vibrance, 44.0f },
        { FloatParam::Gamma, 46.0f },
        { FloatParam::Temperature, 47.0f },
        { FloatParam::Tint, 54.0f },
        { FloatParam::SharpenStrength, 26.0f },
    });
    m_presets.push_back(neon);

    auto cinematic = zaidLux;
    cinematic.name = "Cinematic";
    setValues(cinematic, {}, {
        { FloatParam::BloomIntensity, 38.0f },
        { FloatParam::BloomThreshold, 78.0f },
        { FloatParam::EmissiveIntensity, 42.0f },
        { FloatParam::AOIntensity, 38.0f },
        { FloatParam::AODarkness, 42.0f },
        { FloatParam::ReflectionIntensity, 18.0f },
        { FloatParam::ReflectionOpacity, 14.0f },
        { FloatParam::DynamicRange, 74.0f },
        { FloatParam::HighlightCompression, 48.0f },
        { FloatParam::ShadowRecovery, 14.0f },
        { FloatParam::LocalContrastStrength, 40.0f },
        { FloatParam::SpecularIntensity, 20.0f },
        { FloatParam::RaysIntensity, 9.0f },
        { FloatParam::Exposure, 48.0f },
        { FloatParam::Contrast, 74.0f },
        { FloatParam::Saturation, 48.0f },
        { FloatParam::Vibrance, 15.0f },
        { FloatParam::Gamma, 45.0f },
        { FloatParam::Temperature, 46.0f },
        { FloatParam::Tint, 48.0f },
        { FloatParam::Vignette, 22.0f },
        { FloatParam::FilmGrain, 3.0f },
        { FloatParam::SharpenStrength, 24.0f },
    });
    m_presets.push_back(cinematic);

    auto cyberpunk = neon;
    cyberpunk.name = "Cyberpunk";
    setValues(cyberpunk, {}, {
        { FloatParam::BloomIntensity, 62.0f },
        { FloatParam::BloomThreshold, 60.0f },
        { FloatParam::EmissiveIntensity, 86.0f },
        { FloatParam::EmissiveColorBoost, 75.0f },
        { FloatParam::ReflectionIntensity, 45.0f },
        { FloatParam::ReflectionOpacity, 32.0f },
        { FloatParam::AOIntensity, 28.0f },
        { FloatParam::DynamicRange, 68.0f },
        { FloatParam::LocalContrastStrength, 35.0f },
        { FloatParam::SpecularIntensity, 44.0f },
        { FloatParam::RaysIntensity, 22.0f },
        { FloatParam::Exposure, 55.0f },
        { FloatParam::Contrast, 70.0f },
        { FloatParam::Saturation, 76.0f },
        { FloatParam::Vibrance, 50.0f },
        { FloatParam::Temperature, 43.0f },
        { FloatParam::Tint, 61.0f },
        { FloatParam::ChromaticAberration, 4.0f },
        { FloatParam::SharpenStrength, 27.0f },
    });
    m_presets.push_back(cyberpunk);

    auto performance = makeBasePreset("ZaidLux Performance");
    performance.quality = "Low";
    setValues(performance, {
        { BoolParam::BloomEnabled, true },
        { BoolParam::EmissiveEnabled, true },
        { BoolParam::AOEnabled, true },
        { BoolParam::HDREnabled, true },
        { BoolParam::LocalContrastEnabled, true },
        { BoolParam::SharpenEnabled, true },
    }, {
        { FloatParam::BloomIntensity, 48.0f },
        { FloatParam::BloomThreshold, 70.0f },
        { FloatParam::BloomRadius, 24.0f },
        { FloatParam::BloomSoftKnee, 34.0f },
        { FloatParam::BloomQuality, 20.0f },
        { FloatParam::EmissiveIntensity, 55.0f },
        { FloatParam::EmissiveThreshold, 65.0f },
        { FloatParam::EmissiveRadius, 22.0f },
        { FloatParam::AOIntensity, 15.0f },
        { FloatParam::AOQuality, 10.0f },
        { FloatParam::DynamicRange, 55.0f },
        { FloatParam::LocalContrastStrength, 24.0f },
        { FloatParam::DetailStrength, 16.0f },
        { FloatParam::SharpenStrength, 22.0f },
        { FloatParam::SharpenRadius, 15.0f },
        { FloatParam::Exposure, 54.0f },
        { FloatParam::Contrast, 64.0f },
        { FloatParam::Saturation, 59.0f },
        { FloatParam::Vibrance, 20.0f },
    });
    m_presets.push_back(performance);
}

Preset PresetManager::makeBasePreset(std::string name) const {
    Preset preset;
    preset.name = std::move(name);
    for (auto const& definition : kBoolDefinitions) {
        preset.booleans[index(definition.id)] = definition.defaultValue;
    }
    for (auto const& definition : kFloatDefinitions) {
        preset.floats[index(definition.id)] = definition.defaultValue;
    }
    return preset;
}

void PresetManager::initialize() {
    auto selected = Mod::get()->getSettingValue<std::string>("preset");
    if (selected == "RTX") selected = "ZaidLux";
    else if (selected == "RTX Neon") selected = "ZaidLux Neon";
    else if (selected == "Performance RTX") selected = "ZaidLux Performance";

    if (selected != Mod::get()->getSettingValue<std::string>("preset")) {
        Mod::get()->setSettingValue<std::string>("preset", selected);
    }

    if (selected == "Custom") reloadRenderer();
    else applyPreset(selected);
}

void PresetManager::queuePreset(std::string presetName) {
    geode::queueInMainThread([presetName = std::move(presetName)] {
        PresetManager::get().applyPreset(presetName);
    });
}

void PresetManager::applyPreset(std::string_view presetName) {
    auto const* preset = findPreset(presetName);
    if (!preset) {
        log::warn("[ZaidFX] unknown preset: {}", presetName);
        return;
    }
    writePreset(*preset, true);
}

void PresetManager::markCustom() {
    if (m_applyingPreset) return;
    auto* mod = Mod::get();
    if (mod->getSettingValue<std::string>("preset") != "Custom") {
        mod->setSettingValue<std::string>("preset", "Custom");
    }
}

void PresetManager::resetAll() {
    applyPreset("Default");
}

void PresetManager::resetCurrentSection() {
    auto const section = Mod::get()->getSettingValue<std::string>("reset-section-target");

    if (section == "Bloom") resetKeys(
        { BoolParam::BloomEnabled },
        { FloatParam::BloomIntensity, FloatParam::BloomThreshold, FloatParam::BloomRadius,
          FloatParam::BloomSoftKnee, FloatParam::BloomQuality }
    );
    else if (section == "Emissive Lighting") resetKeys(
        { BoolParam::EmissiveEnabled, BoolParam::PlayerEmissive,
          BoolParam::ObjectEmissive, BoolParam::ParticleEmissive },
        { FloatParam::EmissiveIntensity, FloatParam::EmissiveThreshold,
          FloatParam::EmissiveRadius, FloatParam::EmissiveColorBoost }
    );
    else if (section == "Ambient Occlusion") resetKeys(
        { BoolParam::AOEnabled },
        { FloatParam::AOIntensity, FloatParam::AORadius,
          FloatParam::AODarkness, FloatParam::AOQuality }
    );
    else if (section == "Reflections") resetKeys(
        { BoolParam::ReflectionsEnabled, BoolParam::ReflectPlayer,
          BoolParam::ReflectObjects, BoolParam::ReflectParticles },
        { FloatParam::ReflectionIntensity, FloatParam::ReflectionOpacity,
          FloatParam::ReflectionBlur, FloatParam::ReflectionDistance,
          FloatParam::ReflectionDistortion }
    );
    else if (section == "HDR") resetKeys(
        { BoolParam::HDREnabled },
        { FloatParam::DynamicRange, FloatParam::HighlightCompression,
          FloatParam::ShadowRecovery, FloatParam::HDRWhitePoint,
          FloatParam::HDRBlackPoint }
    );
    else if (section == "Color Grading") resetKeys({}, {
        FloatParam::Exposure, FloatParam::Contrast, FloatParam::Saturation,
        FloatParam::Vibrance, FloatParam::Gamma, FloatParam::Temperature,
        FloatParam::Tint, FloatParam::Highlights, FloatParam::Shadows,
        FloatParam::ColorWhitePoint, FloatParam::ColorBlackPoint,
        FloatParam::Vignette, FloatParam::ChromaticAberration, FloatParam::FilmGrain
    });
    else if (section == "Local Contrast") resetKeys(
        { BoolParam::LocalContrastEnabled },
        { FloatParam::LocalContrastStrength, FloatParam::DetailStrength,
          FloatParam::DetailRadius }
    );
    else if (section == "Specular") resetKeys(
        { BoolParam::SpecularEnabled },
        { FloatParam::SpecularIntensity, FloatParam::SpecularSize,
          FloatParam::SpecularThreshold, FloatParam::MetallicLook,
          FloatParam::GlassLook }
    );
    else if (section == "Light Rays") resetKeys(
        { BoolParam::LightRaysEnabled },
        { FloatParam::RaysIntensity, FloatParam::RaysLength,
          FloatParam::RaysBlur, FloatParam::RaysDecay,
          FloatParam::RaysThreshold, FloatParam::RaysCenterX,
          FloatParam::RaysCenterY }
    );
    else if (section == "Depth Blur") resetKeys(
        { BoolParam::DepthBlurEnabled },
        { FloatParam::BackgroundBlur, FloatParam::ForegroundSharpness,
          FloatParam::DepthSeparation, FloatParam::BlurTransition }
    );
    else if (section == "Sharpen") resetKeys(
        { BoolParam::SharpenEnabled },
        { FloatParam::SharpenStrength, FloatParam::SharpenRadius,
          FloatParam::SharpenThreshold }
    );
    else if (section == "Reactive Lighting") resetKeys(
        { BoolParam::ReactiveEnabled, BoolParam::ReactMusic, BoolParam::ReactJump,
          BoolParam::ReactOrbs, BoolParam::ReactPortals, BoolParam::ReactCoins,
          BoolParam::ReactSpeed, BoolParam::ReactDeath },
        { FloatParam::ReactiveIntensity, FloatParam::ReactiveDecay,
          FloatParam::MusicSensitivity, FloatParam::PulseStrength,
          FloatParam::FlashStrength }
    );
    else {
        resetAll();
        return;
    }

    FLAlertLayer::create("ZaidFX", fmt::format("{} reset to defaults.", section), "OK")->show();
}

void PresetManager::saveCustomPreset() {
    auto* mod = Mod::get();
    mod->setSavedValue<bool>("custom-preset-exists", true);
    mod->setSavedValue<bool>("custom-master-enabled", mod->getSettingValue<bool>("enabled"));
    mod->setSavedValue<std::string>("custom-quality", mod->getSettingValue<std::string>("quality"));

    for (auto const& definition : kBoolDefinitions) {
        mod->setSavedValue<bool>(
            customKey("bool", definition.key),
            mod->getSettingValue<bool>(definition.key)
        );
    }
    for (auto const& definition : kFloatDefinitions) {
        mod->setSavedValue<double>(
            customKey("float", definition.key),
            mod->getSettingValue<double>(definition.key)
        );
    }

    FLAlertLayer::create(
        "Custom preset saved",
        "All toggles, sliders and quality values were saved.",
        "OK"
    )->show();
}

void PresetManager::loadCustomPreset() {
    auto* mod = Mod::get();
    if (!mod->getSavedValue<bool>("custom-preset-exists", false)) {
        FLAlertLayer::create("No custom preset", "Save a custom preset first.", "OK")->show();
        return;
    }

    m_applyingPreset = true;
    mod->setSettingValue<bool>(
        "enabled",
        mod->getSavedValue<bool>("custom-master-enabled", true)
    );
    mod->setSettingValue<std::string>(
        "quality",
        mod->getSavedValue<std::string>("custom-quality", "Medium")
    );
    for (auto const& definition : kBoolDefinitions) {
        mod->setSettingValue<bool>(
            definition.key,
            mod->getSavedValue<bool>(
                customKey("bool", definition.key),
                definition.defaultValue
            )
        );
    }
    for (auto const& definition : kFloatDefinitions) {
        mod->setSettingValue<double>(
            definition.key,
            mod->getSavedValue<double>(
                customKey("float", definition.key),
                definition.defaultValue
            )
        );
    }
    mod->setSettingValue<std::string>("preset", "Custom");
    m_applyingPreset = false;
    reloadRenderer();

    FLAlertLayer::create(
        "Custom preset loaded",
        "The saved values are active immediately.",
        "OK"
    )->show();
}

void PresetManager::handleAction(std::string_view action) {
    if (action == "reset-section") resetCurrentSection();
    else if (action == "reset-all") resetAll();
    else if (action == "save-custom") saveCustomPreset();
    else if (action == "load-custom") loadCustomPreset();
}

bool PresetManager::isApplyingPreset() const {
    return m_applyingPreset;
}

Preset const* PresetManager::findPreset(std::string_view name) const {
    auto const found = std::find_if(
        m_presets.begin(),
        m_presets.end(),
        [name](Preset const& preset) { return preset.name == name; }
    );
    return found == m_presets.end() ? nullptr : &*found;
}

void PresetManager::writePreset(Preset const& preset, bool updatePresetName) {
    auto* mod = Mod::get();
    m_applyingPreset = true;

    mod->setSettingValue<bool>("enabled", preset.enabled);
    mod->setSettingValue<std::string>("quality", preset.quality);
    for (auto const& definition : kBoolDefinitions) {
        mod->setSettingValue<bool>(
            definition.key,
            preset.booleans[index(definition.id)]
        );
    }
    for (auto const& definition : kFloatDefinitions) {
        mod->setSettingValue<double>(
            definition.key,
            static_cast<double>(preset.floats[index(definition.id)])
        );
    }
    if (updatePresetName) {
        mod->setSettingValue<std::string>("preset", preset.name);
    }

    m_applyingPreset = false;
    reloadRenderer();
}

void PresetManager::reloadRenderer() {
    PostProcessRenderer::get().reloadSettings();
}

void PresetManager::resetKeys(
    std::initializer_list<BoolParam> booleans,
    std::initializer_list<FloatParam> floats
) {
    auto* mod = Mod::get();
    m_applyingPreset = true;

    for (auto const id : booleans) {
        auto const& definition = kBoolDefinitions[index(id)];
        mod->setSettingValue<bool>(definition.key, definition.defaultValue);
    }
    for (auto const id : floats) {
        auto const& definition = kFloatDefinitions[index(id)];
        mod->setSettingValue<double>(definition.key, definition.defaultValue);
    }
    mod->setSettingValue<std::string>("preset", "Custom");

    m_applyingPreset = false;
    reloadRenderer();
}

} // namespace zaidfx
