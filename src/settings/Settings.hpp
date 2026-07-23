#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

namespace zaidfx {

enum class BoolParam : std::size_t {
    BloomEnabled,
    EmissiveEnabled,
    PlayerEmissive,
    ObjectEmissive,
    ParticleEmissive,
    AOEnabled,
    ReflectionsEnabled,
    ReflectPlayer,
    ReflectObjects,
    ReflectParticles,
    HDREnabled,
    LocalContrastEnabled,
    SpecularEnabled,
    LightRaysEnabled,
    DepthBlurEnabled,
    SharpenEnabled,
    ReactiveEnabled,
    ReactMusic,
    ReactJump,
    ReactOrbs,
    ReactPortals,
    ReactCoins,
    ReactSpeed,
    ReactDeath,
    Count
};

enum class FloatParam : std::size_t {
    EffectIntensity,

    BloomIntensity,
    BloomThreshold,
    BloomRadius,
    BloomSoftKnee,
    BloomQuality,

    EmissiveIntensity,
    EmissiveThreshold,
    EmissiveRadius,
    EmissiveColorBoost,

    AOIntensity,
    AORadius,
    AODarkness,
    AOQuality,

    ReflectionIntensity,
    ReflectionOpacity,
    ReflectionBlur,
    ReflectionDistance,
    ReflectionDistortion,

    DynamicRange,
    HighlightCompression,
    ShadowRecovery,
    HDRWhitePoint,
    HDRBlackPoint,

    Exposure,
    Contrast,
    Saturation,
    Vibrance,
    Gamma,
    Temperature,
    Tint,
    Highlights,
    Shadows,
    ColorWhitePoint,
    ColorBlackPoint,

    LocalContrastStrength,
    DetailStrength,
    DetailRadius,

    SpecularIntensity,
    SpecularSize,
    SpecularThreshold,
    MetallicLook,
    GlassLook,

    RaysIntensity,
    RaysLength,
    RaysBlur,
    RaysDecay,
    RaysThreshold,
    RaysCenterX,
    RaysCenterY,

    BackgroundBlur,
    ForegroundSharpness,
    DepthSeparation,
    BlurTransition,

    SharpenStrength,
    SharpenRadius,
    SharpenThreshold,

    Vignette,
    ChromaticAberration,
    FilmGrain,

    ReactiveIntensity,
    ReactiveDecay,
    MusicSensitivity,
    PulseStrength,
    FlashStrength,

    Count
};

struct BoolDefinition final {
    BoolParam id;
    std::string_view key;
    bool defaultValue;
};

struct FloatDefinition final {
    FloatParam id;
    std::string_view key;
    float defaultValue;
    float minValue;
    float maxValue;
};

inline constexpr std::size_t kBoolParamCount = static_cast<std::size_t>(BoolParam::Count);
inline constexpr std::size_t kFloatParamCount = static_cast<std::size_t>(FloatParam::Count);

inline constexpr std::array<BoolDefinition, kBoolParamCount> kBoolDefinitions {{
    { BoolParam::BloomEnabled, "bloom-enabled", false },
    { BoolParam::EmissiveEnabled, "emissive-enabled", false },
    { BoolParam::PlayerEmissive, "player-emissive", true },
    { BoolParam::ObjectEmissive, "object-emissive", true },
    { BoolParam::ParticleEmissive, "particle-emissive", true },
    { BoolParam::AOEnabled, "ao-enabled", false },
    { BoolParam::ReflectionsEnabled, "reflections-enabled", false },
    { BoolParam::ReflectPlayer, "reflect-player", true },
    { BoolParam::ReflectObjects, "reflect-objects", true },
    { BoolParam::ReflectParticles, "reflect-particles", true },
    { BoolParam::HDREnabled, "hdr-enabled", false },
    { BoolParam::LocalContrastEnabled, "local-contrast-enabled", false },
    { BoolParam::SpecularEnabled, "specular-enabled", false },
    { BoolParam::LightRaysEnabled, "light-rays-enabled", false },
    { BoolParam::DepthBlurEnabled, "depth-blur-enabled", false },
    { BoolParam::SharpenEnabled, "sharpen-enabled", false },
    { BoolParam::ReactiveEnabled, "reactive-enabled", false },
    { BoolParam::ReactMusic, "react-music", false },
    { BoolParam::ReactJump, "react-jump", false },
    { BoolParam::ReactOrbs, "react-orbs", false },
    { BoolParam::ReactPortals, "react-portals", false },
    { BoolParam::ReactCoins, "react-coins", false },
    { BoolParam::ReactSpeed, "react-speed", false },
    { BoolParam::ReactDeath, "react-death", false },
}};

inline constexpr std::array<FloatDefinition, kFloatParamCount> kFloatDefinitions {{
    { FloatParam::EffectIntensity, "effect-intensity", 100.0f, 0.0f, 100.0f },

    { FloatParam::BloomIntensity, "bloom-intensity", 0.0f, 0.0f, 100.0f },
    { FloatParam::BloomThreshold, "bloom-threshold", 70.0f, 0.0f, 100.0f },
    { FloatParam::BloomRadius, "bloom-radius", 25.0f, 0.0f, 100.0f },
    { FloatParam::BloomSoftKnee, "bloom-soft-knee", 35.0f, 0.0f, 100.0f },
    { FloatParam::BloomQuality, "bloom-quality", 50.0f, 0.0f, 100.0f },

    { FloatParam::EmissiveIntensity, "emissive-intensity", 0.0f, 0.0f, 100.0f },
    { FloatParam::EmissiveThreshold, "emissive-threshold", 65.0f, 0.0f, 100.0f },
    { FloatParam::EmissiveRadius, "emissive-radius", 30.0f, 0.0f, 100.0f },
    { FloatParam::EmissiveColorBoost, "emissive-color-boost", 35.0f, 0.0f, 100.0f },

    { FloatParam::AOIntensity, "ao-intensity", 0.0f, 0.0f, 100.0f },
    { FloatParam::AORadius, "ao-radius", 20.0f, 0.0f, 100.0f },
    { FloatParam::AODarkness, "ao-darkness", 25.0f, 0.0f, 100.0f },
    { FloatParam::AOQuality, "ao-quality", 50.0f, 0.0f, 100.0f },

    { FloatParam::ReflectionIntensity, "reflection-intensity", 0.0f, 0.0f, 100.0f },
    { FloatParam::ReflectionOpacity, "reflection-opacity", 0.0f, 0.0f, 100.0f },
    { FloatParam::ReflectionBlur, "reflection-blur", 25.0f, 0.0f, 100.0f },
    { FloatParam::ReflectionDistance, "reflection-distance", 15.0f, 0.0f, 100.0f },
    { FloatParam::ReflectionDistortion, "reflection-distortion", 0.0f, 0.0f, 100.0f },

    { FloatParam::DynamicRange, "dynamic-range", 50.0f, 0.0f, 100.0f },
    { FloatParam::HighlightCompression, "highlight-compression", 30.0f, 0.0f, 100.0f },
    { FloatParam::ShadowRecovery, "shadow-recovery", 0.0f, 0.0f, 100.0f },
    { FloatParam::HDRWhitePoint, "hdr-white-point", 90.0f, 1.0f, 100.0f },
    { FloatParam::HDRBlackPoint, "hdr-black-point", 5.0f, 0.0f, 99.0f },

    { FloatParam::Exposure, "exposure", 50.0f, 0.0f, 100.0f },
    { FloatParam::Contrast, "contrast", 50.0f, 0.0f, 100.0f },
    { FloatParam::Saturation, "saturation", 50.0f, 0.0f, 100.0f },
    { FloatParam::Vibrance, "vibrance", 0.0f, 0.0f, 100.0f },
    { FloatParam::Gamma, "gamma", 50.0f, 0.0f, 100.0f },
    { FloatParam::Temperature, "temperature", 50.0f, 0.0f, 100.0f },
    { FloatParam::Tint, "tint", 50.0f, 0.0f, 100.0f },
    { FloatParam::Highlights, "highlights", 50.0f, 0.0f, 100.0f },
    { FloatParam::Shadows, "shadows", 50.0f, 0.0f, 100.0f },
    { FloatParam::ColorWhitePoint, "color-white-point", 100.0f, 1.0f, 100.0f },
    { FloatParam::ColorBlackPoint, "color-black-point", 0.0f, 0.0f, 99.0f },

    { FloatParam::LocalContrastStrength, "local-contrast-strength", 0.0f, 0.0f, 100.0f },
    { FloatParam::DetailStrength, "detail-strength", 0.0f, 0.0f, 100.0f },
    { FloatParam::DetailRadius, "detail-radius", 20.0f, 0.0f, 100.0f },

    { FloatParam::SpecularIntensity, "specular-intensity", 0.0f, 0.0f, 100.0f },
    { FloatParam::SpecularSize, "specular-size", 20.0f, 0.0f, 100.0f },
    { FloatParam::SpecularThreshold, "specular-threshold", 70.0f, 0.0f, 100.0f },
    { FloatParam::MetallicLook, "metallic-look", 0.0f, 0.0f, 100.0f },
    { FloatParam::GlassLook, "glass-look", 0.0f, 0.0f, 100.0f },

    { FloatParam::RaysIntensity, "rays-intensity", 0.0f, 0.0f, 100.0f },
    { FloatParam::RaysLength, "rays-length", 20.0f, 0.0f, 100.0f },
    { FloatParam::RaysBlur, "rays-blur", 20.0f, 0.0f, 100.0f },
    { FloatParam::RaysDecay, "rays-decay", 40.0f, 0.0f, 100.0f },
    { FloatParam::RaysThreshold, "rays-threshold", 75.0f, 0.0f, 100.0f },
    { FloatParam::RaysCenterX, "rays-center-x", 50.0f, 0.0f, 100.0f },
    { FloatParam::RaysCenterY, "rays-center-y", 50.0f, 0.0f, 100.0f },

    { FloatParam::BackgroundBlur, "background-blur", 0.0f, 0.0f, 100.0f },
    { FloatParam::ForegroundSharpness, "foreground-sharpness", 50.0f, 0.0f, 100.0f },
    { FloatParam::DepthSeparation, "depth-separation", 20.0f, 0.0f, 100.0f },
    { FloatParam::BlurTransition, "blur-transition", 35.0f, 0.0f, 100.0f },

    { FloatParam::SharpenStrength, "sharpen-strength", 0.0f, 0.0f, 100.0f },
    { FloatParam::SharpenRadius, "sharpen-radius", 20.0f, 0.0f, 100.0f },
    { FloatParam::SharpenThreshold, "sharpen-threshold", 20.0f, 0.0f, 100.0f },

    { FloatParam::Vignette, "vignette", 0.0f, 0.0f, 100.0f },
    { FloatParam::ChromaticAberration, "chromatic-aberration", 0.0f, 0.0f, 100.0f },
    { FloatParam::FilmGrain, "film-grain", 0.0f, 0.0f, 100.0f },

    { FloatParam::ReactiveIntensity, "reactive-intensity", 0.0f, 0.0f, 100.0f },
    { FloatParam::ReactiveDecay, "reactive-decay", 55.0f, 0.0f, 100.0f },
    { FloatParam::MusicSensitivity, "music-sensitivity", 40.0f, 0.0f, 100.0f },
    { FloatParam::PulseStrength, "pulse-strength", 20.0f, 0.0f, 100.0f },
    { FloatParam::FlashStrength, "flash-strength", 8.0f, 0.0f, 35.0f },
}};

constexpr std::size_t index(BoolParam id) {
    return static_cast<std::size_t>(id);
}

constexpr std::size_t index(FloatParam id) {
    return static_cast<std::size_t>(id);
}

BoolDefinition const* findBoolDefinition(std::string_view key);
FloatDefinition const* findFloatDefinition(std::string_view key);

struct Settings final {
    bool enabled = false;
    std::string quality = "Medium";
    std::array<bool, kBoolParamCount> booleans {};
    std::array<float, kFloatParamCount> floats {};

    static Settings read();

    [[nodiscard]] bool get(BoolParam id) const;
    [[nodiscard]] float get(FloatParam id) const;
    [[nodiscard]] int qualityLevel() const;
    [[nodiscard]] float qualityScale() const;
    [[nodiscard]] bool hasLightingEffects() const;
    [[nodiscard]] bool hasFinalEffects() const;
    [[nodiscard]] bool hasVisibleEffects() const;

    bool set(std::string_view key, bool value);
    bool set(std::string_view key, float value);
    bool setQuality(std::string value);
    void sanitize();
};

} // namespace zaidfx
