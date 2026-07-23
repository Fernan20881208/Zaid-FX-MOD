#include <Geode/Geode.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCEGLView.hpp>

#include "rendering/PostProcessRenderer.hpp"

#include <array>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

using namespace geode::prelude;

namespace {

struct PresetValues final {
    std::string_view id;
    float intensity;
    float brightness;
    float exposure;
    float contrast;
    float saturation;
    float gamma;
    float bloom;
    float vignette;
    float sharpen;
    float chromaticAberration;
    float tonemapping;
};

struct FloatSettingBinding final {
    std::string_view key;
    float PresetValues::* member;
};

constexpr std::array<PresetValues, 6> kPresets {{
    { "Default",   1.00f,  0.00f,  0.00f, 1.00f, 1.00f, 1.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f },
    { "Cinematic", 1.00f, -0.05f, -0.25f, 1.35f, 0.88f, 1.05f, 0.15f, 0.35f, 0.20f, 0.06f, 0.65f },
    { "Vibrant",   1.00f,  0.12f,  0.10f, 1.10f, 1.55f, 0.95f, 0.20f, 0.05f, 0.12f, 0.02f, 0.25f },
    { "Dark",      1.00f, -0.12f, -0.85f, 1.45f, 0.92f, 1.08f, 0.05f, 0.22f, 0.18f, 0.01f, 0.35f },
    { "Retro",     1.00f, -0.02f, -0.15f, 1.18f, 0.78f, 1.18f, 0.08f, 0.30f, 0.05f, 0.18f, 0.55f },
    { "Glow",      1.00f,  0.08f,  0.35f, 1.42f, 1.12f, 0.94f, 0.55f, 0.12f, 0.42f, 0.03f, 0.78f },
}};

constexpr std::array<FloatSettingBinding, 11> kFloatSettings {{
    { "effect-intensity", &PresetValues::intensity },
    { "brightness", &PresetValues::brightness },
    { "exposure", &PresetValues::exposure },
    { "contrast", &PresetValues::contrast },
    { "saturation", &PresetValues::saturation },
    { "gamma", &PresetValues::gamma },
    { "bloom", &PresetValues::bloom },
    { "vignette", &PresetValues::vignette },
    { "sharpen", &PresetValues::sharpen },
    { "chromatic-aberration", &PresetValues::chromaticAberration },
    { "tonemapping", &PresetValues::tonemapping },
}};

bool s_applyingPreset = false;
bool s_presetApplyQueued = false;
std::optional<std::string> s_pendingPreset;

PresetValues const* findPreset(std::string_view id) {
    for (auto const& preset : kPresets) {
        if (preset.id == id) {
            return &preset;
        }
    }
    return nullptr;
}

FloatSettingBinding const* findFloatSetting(std::string_view key) {
    for (auto const& setting : kFloatSettings) {
        if (setting.key == key) {
            return &setting;
        }
    }
    return nullptr;
}

void applyPresetNow(std::string_view id) {
    auto const* preset = findPreset(id);
    if (!preset) {
        log::warn("[ZaidFX] unknown preset: {}", id);
        return;
    }

    auto* mod = Mod::get();
    auto& renderer = zaidfx::PostProcessRenderer::get();

    s_applyingPreset = true;
    mod->setSettingValue<std::string>("preset", std::string(preset->id));

    for (auto const& setting : kFloatSettings) {
        auto const value = preset->*setting.member;
        mod->setSettingValue<double>(std::string(setting.key), static_cast<double>(value));
        renderer.setFloat(setting.key, value);
    }

    s_applyingPreset = false;
}

void queuePresetApply(std::string id) {
    s_pendingPreset = std::move(id);
    if (s_presetApplyQueued) {
        return;
    }

    s_presetApplyQueued = true;
    geode::queueInMainThread([] {
        s_presetApplyQueued = false;
        if (!s_pendingPreset) {
            return;
        }

        auto preset = std::move(*s_pendingPreset);
        s_pendingPreset.reset();
        applyPresetNow(preset);
    });
}

bool matchesSelectedPreset(std::string_view key, double value) {
    auto const selected = Mod::get()->getSettingValue<std::string>("preset");
    auto const* preset = findPreset(selected);
    auto const* setting = findFloatSetting(key);

    if (!preset || !setting) {
        return false;
    }

    auto const expected = preset->*setting->member;
    return std::fabs(static_cast<double>(expected) - value) < 0.0001;
}

void handleFloatSetting(std::string_view key, double value) {
    zaidfx::PostProcessRenderer::get().setFloat(key, static_cast<float>(value));

    if (s_applyingPreset || s_pendingPreset || matchesSelectedPreset(key, value)) {
        return;
    }

    auto* mod = Mod::get();
    if (mod->getSettingValue<std::string>("preset") != "Custom") {
        mod->setSettingValue<std::string>("preset", "Custom");
    }
}

void registerSettingListeners() {
    listenForSettingChanges<bool>("enabled", [](bool value) {
        zaidfx::PostProcessRenderer::get().setBool("enabled", value);
    });

    listenForSettingChanges<std::string>("preset", [](std::string value) {
        if (s_applyingPreset) {
            return;
        }

        if (value == "Custom") {
            s_pendingPreset.reset();
            return;
        }

        queuePresetApply(std::move(value));
    });

    listenForSettingChanges<double>("effect-intensity", [](double value) { handleFloatSetting("effect-intensity", value); });
    listenForSettingChanges<double>("brightness", [](double value) { handleFloatSetting("brightness", value); });
    listenForSettingChanges<double>("exposure", [](double value) { handleFloatSetting("exposure", value); });
    listenForSettingChanges<double>("contrast", [](double value) { handleFloatSetting("contrast", value); });
    listenForSettingChanges<double>("saturation", [](double value) { handleFloatSetting("saturation", value); });
    listenForSettingChanges<double>("gamma", [](double value) { handleFloatSetting("gamma", value); });
    listenForSettingChanges<double>("bloom", [](double value) { handleFloatSetting("bloom", value); });
    listenForSettingChanges<double>("vignette", [](double value) { handleFloatSetting("vignette", value); });
    listenForSettingChanges<double>("sharpen", [](double value) { handleFloatSetting("sharpen", value); });
    listenForSettingChanges<double>("chromatic-aberration", [](double value) { handleFloatSetting("chromatic-aberration", value); });
    listenForSettingChanges<double>("tonemapping", [](double value) { handleFloatSetting("tonemapping", value); });
}

} // namespace

class $modify(ZaidFXEGLView, CCEGLView) {
    void swapBuffers() {
        zaidfx::PostProcessRenderer::get().processPresentedFrame();
        CCEGLView::swapBuffers();
    }
};

$execute {
    auto& renderer = zaidfx::PostProcessRenderer::get();
    renderer.initialize();
    registerSettingListeners();

    auto selected = Mod::get()->getSettingValue<std::string>("preset");
    if (selected == "RTX") {
        selected = "Glow";
    }

    if (selected != "Custom") {
        applyPresetNow(selected);
    }
}
