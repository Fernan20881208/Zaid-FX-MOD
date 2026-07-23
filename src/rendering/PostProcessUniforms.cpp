#include "PostProcessRenderer.hpp"

#include <algorithm>

namespace {

float normalized(zaidfx::Settings const& settings, zaidfx::FloatParam id) {
    return settings.get(id) * 0.01f;
}

} // namespace

namespace zaidfx {

bool PostProcessRenderer::applyLightingUniforms() {
    auto set4 = [this](char const* name, float x, float y, float z, float w) {
        return m_lightingShader.setVec4(name, x, y, z, w);
    };

    bool success = m_lightingShader.setInt("CC_Texture0", 0);
    success = m_lightingShader.setVec2(
        "u_texelSize",
        1.0f / static_cast<float>(std::max(m_textureWidth, 1)),
        1.0f / static_cast<float>(std::max(m_textureHeight, 1))
    ) && success;
    success = m_lightingShader.setFloat("u_time", m_time) && success;

    success = set4(
        "u_bloom0",
        m_settings.get(BoolParam::BloomEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::BloomIntensity),
        normalized(m_settings, FloatParam::BloomThreshold),
        normalized(m_settings, FloatParam::BloomRadius)
    ) && success;
    success = set4(
        "u_bloom1",
        normalized(m_settings, FloatParam::BloomSoftKnee),
        normalized(m_settings, FloatParam::BloomQuality),
        static_cast<float>(m_settings.qualityLevel()) / 3.0f,
        0.0f
    ) && success;

    success = set4(
        "u_emissive0",
        m_settings.get(BoolParam::EmissiveEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::EmissiveIntensity),
        normalized(m_settings, FloatParam::EmissiveThreshold),
        normalized(m_settings, FloatParam::EmissiveRadius)
    ) && success;
    success = set4(
        "u_emissive1",
        normalized(m_settings, FloatParam::EmissiveColorBoost),
        m_settings.get(BoolParam::PlayerEmissive) ? 1.0f : 0.0f,
        m_settings.get(BoolParam::ObjectEmissive) ? 1.0f : 0.0f,
        m_settings.get(BoolParam::ParticleEmissive) ? 1.0f : 0.0f
    ) && success;

    success = set4(
        "u_ao0",
        m_settings.get(BoolParam::AOEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::AOIntensity),
        normalized(m_settings, FloatParam::AORadius),
        normalized(m_settings, FloatParam::AODarkness)
    ) && success;
    success = set4(
        "u_ao1",
        normalized(m_settings, FloatParam::AOQuality),
        static_cast<float>(m_settings.qualityLevel()) / 3.0f,
        0.0f,
        0.0f
    ) && success;

    success = set4(
        "u_reflection0",
        m_settings.get(BoolParam::ReflectionsEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::ReflectionIntensity),
        normalized(m_settings, FloatParam::ReflectionOpacity),
        normalized(m_settings, FloatParam::ReflectionBlur)
    ) && success;
    success = set4(
        "u_reflection1",
        normalized(m_settings, FloatParam::ReflectionDistance),
        normalized(m_settings, FloatParam::ReflectionDistortion),
        m_settings.get(BoolParam::ReflectPlayer) ? 1.0f : 0.0f,
        m_settings.get(BoolParam::ReflectObjects) ? 1.0f : 0.0f
    ) && success;
    success = set4(
        "u_reflection2",
        m_settings.get(BoolParam::ReflectParticles) ? 1.0f : 0.0f,
        0.0f,
        0.0f,
        0.0f
    ) && success;

    success = set4(
        "u_rays0",
        m_settings.get(BoolParam::LightRaysEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::RaysIntensity),
        normalized(m_settings, FloatParam::RaysLength),
        normalized(m_settings, FloatParam::RaysBlur)
    ) && success;
    success = set4(
        "u_rays1",
        normalized(m_settings, FloatParam::RaysDecay),
        normalized(m_settings, FloatParam::RaysThreshold),
        normalized(m_settings, FloatParam::RaysCenterX),
        normalized(m_settings, FloatParam::RaysCenterY)
    ) && success;

    return success;
}

bool PostProcessRenderer::applyFinalUniforms(int width, int height) {
    auto set4 = [this](char const* name, float x, float y, float z, float w) {
        return m_finalShader.setVec4(name, x, y, z, w);
    };

    bool success = m_finalShader.setInt("CC_Texture0", 0);
    success = m_finalShader.setInt("u_lightingTexture", 1) && success;
    success = m_finalShader.setVec2(
        "u_texelSize",
        1.0f / static_cast<float>(std::max(width, 1)),
        1.0f / static_cast<float>(std::max(height, 1))
    ) && success;
    success = m_finalShader.setFloat("u_time", m_time) && success;

    auto const intensity = normalized(m_settings, FloatParam::EffectIntensity);
    success = set4(
        "u_pipelineFlags",
        m_settings.hasLightingEffects() ? 1.0f : 0.0f,
        m_settings.hasFinalEffects() ? 1.0f : 0.0f,
        intensity <= 0.0001f ? 1.0f : 0.0f,
        0.0f
    ) && success;

    success = set4(
        "u_master",
        intensity,
        static_cast<float>(m_settings.qualityLevel()) / 3.0f,
        std::clamp(m_reactivePulse, 0.0f, 1.0f),
        std::clamp(m_reactiveFlash, 0.0f, 0.35f)
    ) && success;
    success = set4(
        "u_hdr0",
        m_settings.get(BoolParam::HDREnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::DynamicRange),
        normalized(m_settings, FloatParam::HighlightCompression),
        normalized(m_settings, FloatParam::ShadowRecovery)
    ) && success;
    success = set4(
        "u_hdr1",
        normalized(m_settings, FloatParam::HDRWhitePoint),
        normalized(m_settings, FloatParam::HDRBlackPoint),
        0.0f,
        0.0f
    ) && success;
    success = set4(
        "u_color0",
        normalized(m_settings, FloatParam::Exposure),
        normalized(m_settings, FloatParam::Contrast),
        normalized(m_settings, FloatParam::Saturation),
        normalized(m_settings, FloatParam::Vibrance)
    ) && success;
    success = set4(
        "u_color1",
        normalized(m_settings, FloatParam::Gamma),
        normalized(m_settings, FloatParam::Temperature),
        normalized(m_settings, FloatParam::Tint),
        normalized(m_settings, FloatParam::Highlights)
    ) && success;
    success = set4(
        "u_color2",
        normalized(m_settings, FloatParam::Shadows),
        normalized(m_settings, FloatParam::ColorWhitePoint),
        normalized(m_settings, FloatParam::ColorBlackPoint),
        0.0f
    ) && success;
    success = set4(
        "u_local0",
        m_settings.get(BoolParam::LocalContrastEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::LocalContrastStrength),
        normalized(m_settings, FloatParam::DetailStrength),
        normalized(m_settings, FloatParam::DetailRadius)
    ) && success;
    success = set4(
        "u_specular0",
        m_settings.get(BoolParam::SpecularEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::SpecularIntensity),
        normalized(m_settings, FloatParam::SpecularSize),
        normalized(m_settings, FloatParam::SpecularThreshold)
    ) && success;
    success = set4(
        "u_specular1",
        normalized(m_settings, FloatParam::MetallicLook),
        normalized(m_settings, FloatParam::GlassLook),
        0.0f,
        0.0f
    ) && success;
    success = set4(
        "u_depth0",
        m_settings.get(BoolParam::DepthBlurEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::BackgroundBlur),
        normalized(m_settings, FloatParam::ForegroundSharpness),
        normalized(m_settings, FloatParam::DepthSeparation)
    ) && success;
    success = set4(
        "u_depth1",
        normalized(m_settings, FloatParam::BlurTransition),
        0.0f,
        0.0f,
        0.0f
    ) && success;
    success = set4(
        "u_sharpen0",
        m_settings.get(BoolParam::SharpenEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::SharpenStrength),
        normalized(m_settings, FloatParam::SharpenRadius),
        normalized(m_settings, FloatParam::SharpenThreshold)
    ) && success;
    success = set4(
        "u_finish0",
        normalized(m_settings, FloatParam::Vignette),
        normalized(m_settings, FloatParam::ChromaticAberration),
        normalized(m_settings, FloatParam::FilmGrain),
        0.0f
    ) && success;
    success = set4(
        "u_reactive0",
        m_settings.get(BoolParam::ReactiveEnabled) ? 1.0f : 0.0f,
        normalized(m_settings, FloatParam::ReactiveIntensity),
        normalized(m_settings, FloatParam::ReactiveDecay),
        normalized(m_settings, FloatParam::MusicSensitivity)
    ) && success;
    success = set4(
        "u_reactive1",
        normalized(m_settings, FloatParam::PulseStrength),
        normalized(m_settings, FloatParam::FlashStrength),
        m_smoothedMusic,
        m_gameplaySpeed
    ) && success;

    return success;
}

} // namespace zaidfx
