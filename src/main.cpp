#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "rendering/PostProcessRenderer.hpp"

#include <cmath>
#include <optional>
#include <string_view>

using namespace geode::prelude;

namespace {

bool s_applyingPreset = false;

struct PresetValues final {
    char const* id;
    double intensity;
    double brightness;
    double exposure;
    double contrast;
    double saturation;
    double gamma;
    double bloom;
    double vignette;
    double sharpen;
    double chromaticAberration;
    double tonemapping;
};

std::optional<PresetValues> presetValues(std::string_view preset) {
    if (preset == "Default") {
        return PresetValues { "Default", 1.00, 0.00, 0.00, 1.00, 1.00, 1.00, 0.00, 0.00, 0.00, 0.00, 0.00 };
    }
    if (preset == "Cinematic") {
        return PresetValues { "Cinematic", 1.00, -0.05, -0.25, 1.35, 0.88, 1.05, 0.15, 0.35, 0.20, 0.06, 0.65 };
    }
    if (preset == "Vibrant") {
        return PresetValues { "Vibrant", 1.00, 0.12, 0.10, 1.10, 1.55, 0.95, 0.20, 0.05, 0.12, 0.02, 0.25 };
    }
    if (preset == "Dark") {
        return PresetValues { "Dark", 1.00, -0.12, -0.85, 1.45, 0.92, 1.08, 0.05, 0.22, 0.18, 0.01, 0.35 };
    }
    if (preset == "Retro") {
        return PresetValues { "Retro", 1.00, -0.02, -0.15, 1.18, 0.78, 1.18, 0.08, 0.30, 0.05, 0.18, 0.55 };
    }
    if (preset == "RTX") {
        return PresetValues { "RTX", 1.00, 0.08, 0.35, 1.42, 1.12, 0.94, 0.55, 0.12, 0.42, 0.03, 0.78 };
    }
    return std::nullopt;
}

bool presetValueMatches(std::string_view key, double value) {
    auto const selected = Mod::get()->getSettingValue<std::string>("preset");
    auto const values = presetValues(selected);
    if (!values) {
        return false;
    }

    double expected = value;
    if (key == "effect-intensity") expected = values->intensity;
    else if (key == "brightness") expected = values->brightness;
    else if (key == "exposure") expected = values->exposure;
    else if (key == "contrast") expected = values->contrast;
    else if (key == "saturation") expected = values->saturation;
    else if (key == "gamma") expected = values->gamma;
    else if (key == "bloom") expected = values->bloom;
    else if (key == "vignette") expected = values->vignette;
    else if (key == "sharpen") expected = values->sharpen;
    else if (key == "chromatic-aberration") expected = values->chromaticAberration;
    else if (key == "tonemapping") expected = values->tonemapping;
    else return false;

    return std::fabs(expected - value) < 0.0001;
}

void assignPresetValue(std::string_view key, double value) {
    Mod::get()->setSettingValue<double>(std::string(key), value);
    zaidfx::PostProcessRenderer::get().setFloat(key, static_cast<float>(value));
    log::info("[ZaidFX][preset] slider {} <- {:.4f}", key, value);
}

void applyPreset(std::string_view preset) {
    auto const values = presetValues(preset);
    if (!values) {
        log::warn("[ZaidFX][preset] unknown preset text/id: {}", preset);
        return;
    }

    auto& renderer = zaidfx::PostProcessRenderer::get();
    s_applyingPreset = true;

    renderer.setString("preset", std::string(values->id));
    log::info("[ZaidFX][preset] selected={} internal-id={}", preset, values->id);
    log::info(
        "[ZaidFX][preset] loaded values intensity={:.2f}, brightness={:.2f}, exposure={:.2f}, "
        "contrast={:.2f}, saturation={:.2f}, gamma={:.2f}, bloom={:.2f}, vignette={:.2f}, "
        "sharpen={:.2f}, chromatic-aberration={:.2f}, tonemapping={:.2f}",
        values->intensity,
        values->brightness,
        values->exposure,
        values->contrast,
        values->saturation,
        values->gamma,
        values->bloom,
        values->vignette,
        values->sharpen,
        values->chromaticAberration,
        values->tonemapping
    );

    assignPresetValue("effect-intensity", values->intensity);
    assignPresetValue("brightness", values->brightness);
    assignPresetValue("exposure", values->exposure);
    assignPresetValue("contrast", values->contrast);
    assignPresetValue("saturation", values->saturation);
    assignPresetValue("gamma", values->gamma);
    assignPresetValue("bloom", values->bloom);
    assignPresetValue("vignette", values->vignette);
    assignPresetValue("sharpen", values->sharpen);
    assignPresetValue("chromatic-aberration", values->chromaticAberration);
    assignPresetValue("tonemapping", values->tonemapping);

    s_applyingPreset = false;
    log::info("[ZaidFX][preset] shader refresh queued immediately for {}", values->id);
}

void markPresetCustom() {
    if (s_applyingPreset) {
        return;
    }

    auto* mod = Mod::get();
    if (mod->getSettingValue<std::string>("preset") != "Custom") {
        mod->setSettingValue<std::string>("preset", "Custom");
        zaidfx::PostProcessRenderer::get().setString("preset", "Custom");
        log::info("[ZaidFX][preset] manual slider change -> Custom");
    }
}

void handleSlider(std::string_view key, double value) {
    log::info("[ZaidFX][slider] {} generated {:.4f}", key, value);
    zaidfx::PostProcessRenderer::get().setFloat(key, static_cast<float>(value));

    if (!s_applyingPreset && !presetValueMatches(key, value)) {
        markPresetCustom();
    }
}

std::string statusText() {
    auto const& renderer = zaidfx::PostProcessRenderer::get();
    auto const& settings = renderer.settings();

    return fmt::format(
        "<cg>Zaid-FX-MOD</c> final-frame post-processing\n\n"
        "Effects: <c{}>{}</c>\n"
        "Pipeline: <c{}>{}</c>\n"
        "Red test: <c{}>{}</c>\n"
        "Preset: <cy>{}</c>\n\n"
        "Intensity: {:.2f}\nBrightness: {:.2f}\nExposure: {:.2f}\n"
        "Contrast: {:.2f}\nSaturation: {:.2f}\nGamma: {:.2f}\n"
        "Bloom: {:.2f}\nVignette: {:.2f}\nSharpen: {:.2f}\n"
        "Chromatic aberration: {:.2f}\nTonemapping: {:.2f}",
        settings.enabled ? "g" : "r",
        settings.enabled ? "enabled" : "disabled",
        renderer.isPipelineReady() ? "g" : "y",
        renderer.isPipelineReady() ? "ready" : "waiting for a presented frame",
        settings.debugRedScreen ? "r" : "g",
        settings.debugRedScreen ? "enabled" : "disabled",
        settings.preset,
        settings.intensity,
        settings.brightness,
        settings.exposure,
        settings.contrast,
        settings.saturation,
        settings.gamma,
        settings.bloom,
        settings.vignette,
        settings.sharpen,
        settings.chromaticAberration,
        settings.tonemapping
    );
}

void registerSettingListeners() {
    listenForSettingChanges<bool>("enabled", [](bool value) {
        log::info("[ZaidFX][slider] enabled generated {}", value);
        zaidfx::PostProcessRenderer::get().setBool("enabled", value);
    });

    listenForSettingChanges<bool>("debug-logging", [](bool value) {
        log::info("[ZaidFX][slider] debug-logging generated {}", value);
        zaidfx::PostProcessRenderer::get().setBool("debug-logging", value);
    });

    listenForSettingChanges<bool>("debug-red-screen", [](bool value) {
        log::info("[ZaidFX][slider] debug-red-screen generated {}", value);
        zaidfx::PostProcessRenderer::get().setBool("debug-red-screen", value);
    });

    listenForSettingChanges<std::string>("preset", [](std::string value) {
        log::info("[ZaidFX][preset] selector generated {}", value);
        zaidfx::PostProcessRenderer::get().setString("preset", value);
        if (value != "Custom") {
            applyPreset(value);
        }
    });

    listenForSettingChanges<double>("effect-intensity", [](double value) { handleSlider("effect-intensity", value); });
    listenForSettingChanges<double>("brightness", [](double value) { handleSlider("brightness", value); });
    listenForSettingChanges<double>("exposure", [](double value) { handleSlider("exposure", value); });
    listenForSettingChanges<double>("contrast", [](double value) { handleSlider("contrast", value); });
    listenForSettingChanges<double>("saturation", [](double value) { handleSlider("saturation", value); });
    listenForSettingChanges<double>("gamma", [](double value) { handleSlider("gamma", value); });
    listenForSettingChanges<double>("bloom", [](double value) { handleSlider("bloom", value); });
    listenForSettingChanges<double>("vignette", [](double value) { handleSlider("vignette", value); });
    listenForSettingChanges<double>("sharpen", [](double value) { handleSlider("sharpen", value); });
    listenForSettingChanges<double>("chromatic-aberration", [](double value) { handleSlider("chromatic-aberration", value); });
    listenForSettingChanges<double>("tonemapping", [](double value) { handleSlider("tonemapping", value); });
}

} // namespace

class $modify(ZaidFXEGLView, CCEGLView) {
    void swapBuffers() {
        zaidfx::PostProcessRenderer::get().processPresentedFrame();
        CCEGLView::swapBuffers();
    }
};

class $modify(ZaidFXMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto* menu = this->getChildByID("bottom-menu");
        if (!menu) {
            log::warn("Zaid-FX-MOD could not find bottom-menu");
            return true;
        }

        auto* icon = CCSprite::createWithSpriteFrameName("ZaidFXLogo.png"_spr);
        if (!icon) {
            log::warn("Zaid-FX-MOD logo missing; using safe fallback icon");
            icon = CCSprite::createWithSpriteFrameName("GJ_starBtn_001.png");
        }
        icon->setScale(0.25f);

        auto* button = CCMenuItemSpriteExtra::create(
            icon,
            this,
            menu_selector(ZaidFXMenuLayer::onZaidFX)
        );
        button->setID("open-fx-status"_spr);

        menu->addChild(button);
        menu->updateLayout();
        return true;
    }

    void onZaidFX(CCObject*) {
        FLAlertLayer::create("Zaid-FX-MOD", statusText(), "OK")->show();
    }
};

$execute {
    zaidfx::PostProcessRenderer::get().initialize();
    registerSettingListeners();

    auto const selected = Mod::get()->getSettingValue<std::string>("preset");
    if (selected != "Custom") {
        applyPreset(selected);
    }

    log::info("Zaid-FX-MOD v0.2.1 preset and final-frame shader pipeline initialized on Android");
}
