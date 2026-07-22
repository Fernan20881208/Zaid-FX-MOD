#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "rendering/PostProcessRenderer.hpp"

#include <optional>
#include <string_view>

using namespace geode::prelude;

namespace {

bool s_applyingPreset = false;

struct PresetValues final {
    double intensity;
    double brightness;
    double exposure;
    double contrast;
    double saturation;
    double gamma;
    double vignette;
    double sharpen;
};

std::optional<PresetValues> presetValues(std::string_view preset) {
    if (preset == "Clean") {
        return PresetValues { 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.05 };
    }
    if (preset == "OLED") {
        return PresetValues { 1.0, -0.03, 0.05, 1.20, 1.08, 0.92, 0.12, 0.15 };
    }
    if (preset == "Vibrant") {
        return PresetValues { 1.0, 0.03, 0.10, 1.12, 1.45, 0.95, 0.05, 0.18 };
    }
    if (preset == "Cinematic") {
        return PresetValues { 0.95, -0.02, -0.05, 1.25, 0.90, 1.05, 0.28, 0.10 };
    }
    if (preset == "RTX Fake") {
        return PresetValues { 1.0, 0.02, 0.18, 1.30, 1.30, 0.92, 0.18, 0.42 };
    }
    if (preset == "Competitive") {
        return PresetValues { 1.0, 0.02, 0.04, 1.12, 1.05, 1.0, 0.0, 0.55 };
    }
    return std::nullopt;
}

void applyPreset(std::string_view preset) {
    auto values = presetValues(preset);
    if (!values) {
        return;
    }

    auto* mod = Mod::get();
    s_applyingPreset = true;

    mod->setSettingValue<double>("effect-intensity", values->intensity);
    mod->setSettingValue<double>("brightness", values->brightness);
    mod->setSettingValue<double>("exposure", values->exposure);
    mod->setSettingValue<double>("contrast", values->contrast);
    mod->setSettingValue<double>("saturation", values->saturation);
    mod->setSettingValue<double>("gamma", values->gamma);
    mod->setSettingValue<double>("vignette", values->vignette);
    mod->setSettingValue<double>("sharpen", values->sharpen);

    s_applyingPreset = false;
    log::info("[ZaidFX][preset] applied {} to the live renderer settings", preset);
}

void markPresetCustom() {
    if (s_applyingPreset) {
        return;
    }

    auto* mod = Mod::get();
    if (mod->getSettingValue<std::string>("preset") != "Custom") {
        mod->setSettingValue<std::string>("preset", "Custom");
    }
}

void handleSlider(std::string_view key, double value) {
    log::info("[ZaidFX][slider] {} generated {:.4f}", key, value);
    zaidfx::PostProcessRenderer::get().setFloat(key, static_cast<float>(value));
    markPresetCustom();
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
        "Vignette: {:.2f}\nSharpen: {:.2f}",
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
        settings.vignette,
        settings.sharpen
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
        log::info("[ZaidFX][slider] preset generated {}", value);
        zaidfx::PostProcessRenderer::get().setString("preset", value);
        if (value != "Custom") {
            applyPreset(value);
        }
    });

    listenForSettingChanges<double>("effect-intensity", [](double value) {
        handleSlider("effect-intensity", value);
    });
    listenForSettingChanges<double>("brightness", [](double value) {
        handleSlider("brightness", value);
    });
    listenForSettingChanges<double>("exposure", [](double value) {
        handleSlider("exposure", value);
    });
    listenForSettingChanges<double>("contrast", [](double value) {
        handleSlider("contrast", value);
    });
    listenForSettingChanges<double>("saturation", [](double value) {
        handleSlider("saturation", value);
    });
    listenForSettingChanges<double>("gamma", [](double value) {
        handleSlider("gamma", value);
    });
    listenForSettingChanges<double>("vignette", [](double value) {
        handleSlider("vignette", value);
    });
    listenForSettingChanges<double>("sharpen", [](double value) {
        handleSlider("sharpen", value);
    });
}

} // namespace

class $modify(ZaidFXEGLView, CCEGLView) {
    void swapBuffers() {
        // Geometry Dash has already drawn the complete menu or gameplay frame.
        // Process that exact framebuffer before the platform presents it.
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
    log::info("Zaid-FX-MOD final-frame shader pipeline initialized on Android");
}
