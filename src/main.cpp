#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "rendering/PostProcessRenderer.hpp"
#include "settings/PresetManager.hpp"
#include "settings/Settings.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

using namespace geode::prelude;

namespace {

zaidfx::PostProcessRenderer& renderer() {
    return zaidfx::PostProcessRenderer::get();
}

zaidfx::PresetManager& presets() {
    return zaidfx::PresetManager::get();
}

void markPresetAsCustom() {
    if (!presets().isApplyingPreset()) {
        presets().markCustom();
    }
}

void triggerWhilePlaying(zaidfx::ReactiveEvent event) {
    if (PlayLayer::get()) {
        renderer().trigger(event);
    }
}

void registerSettingListeners() {
    listenForSettingChanges<bool>("enabled", [](bool enabled) {
        renderer().setBool("enabled", enabled);
        markPresetAsCustom();
    });

    listenForSettingChanges<std::string>("quality", [](std::string quality) {
        renderer().setString("quality", std::move(quality));
        markPresetAsCustom();
    });

    listenForSettingChanges<std::string>("preset", [](std::string preset) {
        if (preset == "Custom" || presets().isApplyingPreset()) {
            return;
        }

        // The settings popup commits its controls one by one. Deferring the
        // preset keeps old slider values from overwriting the new selection.
        presets().queuePreset(std::move(preset));
    });

    for (auto const& setting : zaidfx::kBoolDefinitions) {
        listenForSettingChanges<bool>(
            std::string(setting.key),
            [key = setting.key](bool value) {
                renderer().setBool(key, value);
                markPresetAsCustom();
            }
        );
    }

    for (auto const& setting : zaidfx::kFloatDefinitions) {
        listenForSettingChanges<double>(
            std::string(setting.key),
            [key = setting.key](double value) {
                renderer().setFloat(key, static_cast<float>(value));
                markPresetAsCustom();
            }
        );
    }
}

} // namespace

class $modify(ZaidFXEGLView, CCEGLView) {
    void swapBuffers() {
        renderer().processPresentedFrame();
        CCEGLView::swapBuffers();
    }
};

class $modify(ZaidFXPlayerObject, PlayerObject) {
    struct Fields {
        float previousX = 0.0f;
        bool hasPreviousPosition = false;
    };

    void update(float dt) {
        PlayerObject::update(dt);

        if (!PlayLayer::get() || dt <= 0.0001f) {
            m_fields->hasPreviousPosition = false;
            return;
        }

        auto const currentX = getPositionX();
        if (m_fields->hasPreviousPosition) {
            auto const distance = std::abs(currentX - m_fields->previousX);
            auto const pixelsPerSecond = distance / std::max(dt, 0.001f);
            auto const normalizedSpeed = std::clamp(
                pixelsPerSecond / 900.0f,
                0.0f,
                1.0f
            );

            renderer().setGameplaySpeed(normalizedSpeed);
        }

        m_fields->previousX = currentX;
        m_fields->hasPreviousPosition = true;
    }

    bool pushButton(PlayerButton button) {
        auto const accepted = PlayerObject::pushButton(button);
        if (accepted) {
            triggerWhilePlaying(zaidfx::ReactiveEvent::Jump);
        }
        return accepted;
    }

    void ringJump(RingObject* object, bool skipCheck) {
        PlayerObject::ringJump(object, skipCheck);
        triggerWhilePlaying(zaidfx::ReactiveEvent::Orb);
    }

    void switchedToMode(GameObjectType type) {
        PlayerObject::switchedToMode(type);
        triggerWhilePlaying(zaidfx::ReactiveEvent::Portal);
    }
};

class $modify(ZaidFXGameLayer, GJBaseGameLayer) {
    void pickupItem(EffectGameObject* object) {
        GJBaseGameLayer::pickupItem(object);
        triggerWhilePlaying(zaidfx::ReactiveEvent::Coin);
    }
};

class $modify(ZaidFXPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        renderer().trigger(zaidfx::ReactiveEvent::Death);
        PlayLayer::destroyPlayer(player, object);
    }
};

$execute {
    renderer().initialize();
    registerSettingListeners();
    presets().initialize();
}
