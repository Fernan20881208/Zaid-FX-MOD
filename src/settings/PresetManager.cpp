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
        { FloatParam::BloomIntensity, 32.0f },
        { FloatParam::BloomThreshold, 75.0f },
        { FloatParam::BloomRadius, 22.0f },
        { FloatParam::BloomSoftKnee, 38.0f },
        { FloatParam::BloomQuality, 48.0f },
        { FloatParam::EmissiveIntensity, 14.0f },
        { FloatParam::EmissiveThreshold, 78.0f },
        { FloatParam::EmissiveRadius, 20.0f },
        { FloatParam::EmissiveColorBoost, 10.0f },
        { FloatParam::Contrast, 52.0f },
        { FloatParam::Saturation, 52.0f },
        { FloatParam::SharpenStrength, 7.0f },
        { FloatParam::SharpenRadius, 12.0f },
        { FloatParam::SharpenThreshold, 28.0f },
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
        { FloatParam::BloomIntensity, 32.0f },
        { FloatParam::BloomThreshold, 78.0f },
        { FloatParam::BloomRadius, 22.0f },
        { FloatParam::BloomSoftKnee, 32.0f },
        { FloatParam::BloomQuality, 56.0f },
        { FloatParam::EmissiveIntensity, 34.0f },
        { FloatParam::EmissiveThreshold, 70.0f },
        { FloatParam::EmissiveRadius, 26.0f },
        { FloatParam::EmissiveColorBoost, 18.0f },
        { FloatParam::AOIntensity, 15.0f },
        { FloatParam::AORadius, 16.0f },
        { FloatParam::AODarkness, 24.0f },
        { FloatParam::AOQuality, 48.0f },
        { FloatParam::ReflectionIntensity, 10.0f },
        { FloatParam::ReflectionOpacity, 10.0f },
        { FloatParam::ReflectionBlur, 28.0f },
        { FloatParam::ReflectionDistance, 18.0f },
        { FloatParam::ReflectionDistortion, 5.0f },
        { FloatParam::DynamicRange, 56.0f },
        { FloatParam::HighlightCompression, 45.0f },
        { FloatParam::ShadowRecovery, 8.0f },
        { FloatParam::HDRWhitePoint, 92.0f },
        { FloatParam::HDRBlackPoint, 4.0f },
        { FloatParam::LocalContrastStrength, 18.0f },
        { FloatParam::DetailStrength, 14.0f },
        { FloatParam::DetailRadius, 18.0f },
        { FloatParam::SpecularIntensity, 14.0f },
        { FloatParam::SpecularSize, 16.0f },
        { FloatParam::SpecularThreshold, 78.0f },
        { FloatParam::MetallicLook, 12.0f },
        { FloatParam::GlassLook, 4.0f },
        { FloatParam::RaysIntensity, 6.0f },
        { FloatParam::RaysLength, 16.0f },
        { FloatParam::RaysBlur, 18.0f },
        { FloatParam::RaysDecay, 48.0f },
        { FloatParam::RaysThreshold, 84.0f },
        { FloatParam::RaysCenterX, 50.0f },
        { FloatParam::RaysCenterY, 52.0f },
        { FloatParam::BackgroundBlur, 3.0f },
        { FloatParam::ForegroundSharpness, 58.0f },
        { FloatParam::DepthSeparation, 20.0f },
        { FloatParam::BlurTransition, 38.0f },
        { FloatParam::SharpenStrength, 15.0f },
        { FloatParam::SharpenRadius, 16.0f },
        { FloatParam::SharpenThreshold, 28.0f },
        { FloatParam::Exposure, 52.0f },
        { FloatParam::Contrast, 58.0f },
        { FloatParam::Saturation, 54.0f },
        { FloatParam::Vibrance, 10.0f },
        { FloatParam::Gamma, 50.0f },
        { FloatParam::Temperature, 50.0f },
        { FloatParam::Tint, 50.0f },
        { FloatParam::Highlights, 60.0f },
        { FloatParam::Shadows, 48.0f },
        { FloatParam::Vignette, 7.0f },
        { FloatParam::ChromaticAberration, 1.0f },
        { FloatParam::FilmGrain, 0.0f },
    });
    m_presets.push_back(zaidLux);

    auto neon = zaidLux;
    neon.name = "ZaidLux Neon";
    neon.quality = "Ultra";
    setValues(neon, {}, {
        { FloatParam::BloomIntensity, 42.0f },
        { FloatParam::BloomThreshold, 70.0f },
        { FloatParam::BloomRadius, 28.0f },
        { FloatParam::EmissiveIntensity, 48.0f },
        { FloatParam::EmissiveThreshold, 64.0f },
        { FloatParam::EmissiveColorBoost, 32.0f },
        { FloatParam::AOIntensity, 13.0f },
        { FloatParam::ReflectionIntensity, 18.0f },
        { FloatParam::ReflectionOpacity, 14.0f },
        { FloatParam::DynamicRange, 60.0f },
        { FloatParam::LocalContrastStrength, 20.0f },
        { FloatParam::SpecularIntensity, 18.0f },
        { FloatParam::MetallicLook, 18.0f },
        { FloatParam::GlassLook, 7.0f },
        { FloatParam::RaysIntensity, 9.0f },
        { FloatParam::Exposure, 53.0f },
        { FloatParam::Contrast, 61.0f },
        { FloatParam::Saturation, 62.0f },
        { FloatParam::Vibrance, 20.0f },
        { FloatParam::Gamma, 50.0f },
        { FloatParam::Temperature, 49.0f },
        { FloatParam::Tint, 53.0f },
        { FloatParam::SharpenStrength, 14.0f },
    });
    m_presets.push_back(neon);

    auto cinematic = zaidLux;
    cinematic.name = "Cinematic";
    cinematic.quality = "High";
    setValues(cinematic, {}, {
        { FloatParam::BloomIntensity, 24.0f },
        { FloatParam::BloomThreshold, 82.0f },
        { FloatParam::EmissiveIntensity, 22.0f },
        { FloatParam::AOIntensity, 20.0f },
        { FloatParam::AODarkness, 30.0f },
        { FloatParam::ReflectionIntensity, 7.0f },
        { FloatParam::ReflectionOpacity, 7.0f },
        { FloatParam::DynamicRange, 60.0f },
        { FloatParam::HighlightCompression, 54.0f },
        { FloatParam::ShadowRecovery, 6.0f },
        { FloatParam::LocalContrastStrength, 22.0f },
        { FloatParam::SpecularIntensity, 10.0f },
        { FloatParam::RaysIntensity, 4.0f },
        { FloatParam::Exposure, 48.0f },
        { FloatParam::Contrast, 64.0f },
        { FloatParam::Saturation, 46.0f },
        { FloatParam::Vibrance, 5.0f },
        { FloatParam::Gamma, 49.0f },
        { FloatParam::Temperature, 48.0f },
        { FloatParam::Tint, 49.0f },
        { FloatParam::Highlights, 55.0f },
        { FloatParam::Shadows, 45.0f },
        { FloatParam::Vignette, 16.0f },
        { FloatParam::FilmGrain, 2.0f },
        { FloatParam::SharpenStrength, 12.0f },
    });
    m_presets.push_back(cinematic);

    auto cyberpunk = neon;
    cyberpunk.name = "Cyberpunk";
    setValues(cyberpunk, {}, {
        { FloatParam::BloomIntensity, 46.0f },
        { FloatParam::BloomThreshold, 68.0f },
        { FloatParam::EmissiveIntensity, 55.0f },
        { FloatParam::EmissiveColorBoost, 38.0f },
        { FloatParam::ReflectionIntensity, 22.0f },
        { FloatParam::ReflectionOpacity, 17.0f },
        { FloatParam::AOIntensity, 14.0f },
        { FloatParam::DynamicRange, 58.0f },
        { FloatParam::LocalContrastStrength, 20.0f },
        { FloatParam::SpecularIntensity, 22.0f },
        { FloatParam::RaysIntensity, 11.0f },
        { FloatParam::Exposure, 52.0f },
        { FloatParam::Contrast, 62.0f },
        { FloatParam::Saturation, 64.0f },
        { FloatParam::Vibrance, 24.0f },
        { FloatParam::Temperature, 46.0f },
        { FloatParam::Tint, 56.0f },
        { FloatParam::ChromaticAberration, 2.0f },
        { FloatParam::SharpenStrength, 14.0f },
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
        { FloatParam::BloomIntensity, 27.0f },
        { FloatParam::BloomThreshold, 78.0f },
        { FloatParam::BloomRadius, 18.0f },
        { FloatParam::BloomSoftKnee, 30.0f },
        { FloatParam::BloomQuality, 15.0f },
        { FloatParam::EmissiveIntensity, 25.0f },
        { FloatParam::EmissiveThreshold, 72.0f },
        { FloatParam::EmissiveRadius, 18.0f },
        { FloatParam::EmissiveColorBoost, 12.0f },
        { FloatParam::AOIntensity, 9.0f },
        { FloatParam::AOQuality, 8.0f },
        { FloatParam::DynamicRange, 52.0f },
        { FloatParam::HighlightCompression, 42.0f },
        { FloatParam::LocalContrastStrength, 13.0f },
        { FloatParam::DetailStrength, 9.0f },
        { FloatParam::SharpenStrength, 12.0f },
        { FloatParam::SharpenRadius, 13.0f },
        { FloatParam::SharpenThreshold, 30.0f },
        { FloatParam::Exposure, 51.0f },
        { FloatParam::Contrast, 56.0f },
        { FloatParam::Saturation, 53.0f },
        { FloatParam::Vibrance, 7.0f },
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
