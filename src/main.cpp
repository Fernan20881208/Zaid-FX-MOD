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

void registerSettingListeners() {
    listenForSettingChanges<bool>("enabled", [](bool value) {
        zaidfx::PostProcessRenderer::get().setBool("enabled", value);

        if (!zaidfx::PresetManager::get().isApplyingPreset()) {
            zaidfx::PresetManager::get().markCustom();
        }
    });

    listenForSettingChanges<std::string>("quality", [](std::string value) {
        zaidfx::PostProcessRenderer::get().setString("quality", std::move(value));

        if (!zaidfx::PresetManager::get().isApplyingPreset()) {
            zaidfx::PresetManager::get().markCustom();
        }
    });

    listenForSettingChanges<std::string>("preset", [](std::string value) {
        if (
            value != "Custom" &&
            !zaidfx::PresetManager::get().isApplyingPreset()
        ) {
            // Geode commits every control in the popup separately. Applying on the
            // next main-thread update prevents stale slider values from overwriting
            // the selected preset.
            zaidfx::PresetManager::get().queuePreset(std::move(value));
        }
    });

    for (auto const& definition : zaidfx::kBoolDefinitions) {
        listenForSettingChanges<bool>(
            std::string(definition.key),
            [key = definition.key](bool value) {
                zaidfx::PostProcessRenderer::get().setBool(key, value);

                if (!zaidfx::PresetManager::get().isApplyingPreset()) {
                    zaidfx::PresetManager::get().markCustom();
                }
            }
        );
    }

    for (auto const& definition : zaidfx::kFloatDefinitions) {
        listenForSettingChanges<double>(
            std::string(definition.key),
            [key = definition.key](double value) {
                zaidfx::PostProcessRenderer::get().setFloat(
                    key,
                    static_cast<float>(value)
                );

                if (!zaidfx::PresetManager::get().isApplyingPreset()) {
                    zaidfx::PresetManager::get().markCustom();
                }
            }
        );
    }
}

} // namespace

class $modify(ZaidFXEGLView, CCEGLView) {
    void swapBuffers() {
        zaidfx::PostProcessRenderer::get().processPresentedFrame();
        CCEGLView::swapBuffers();
    }
};

class $modify(ZaidFXPlayerObject, PlayerObject) {
    struct Fields {
        float lastX = 0.0f;
        bool hasLastPosition = false;
    };

    void update(float dt) {
        PlayerObject::update(dt);

        if (!PlayLayer::get() || dt <= 0.0001f) {
            m_fields->hasLastPosition = false;
            return;
        }

        auto const x = getPositionX();
        if (m_fields->hasLastPosition) {
            auto const pixelsPerSecond =
                std::abs(x - m_fields->lastX) / std::max(dt, 0.001f);
            auto const speed = std::clamp(
                pixelsPerSecond / 900.0f,
                0.0f,
                1.0f
            );
            zaidfx::PostProcessRenderer::get().setGameplaySpeed(speed);
        }

        m_fields->lastX = x;
        m_fields->hasLastPosition = true;
    }

    bool pushButton(PlayerButton button) {
        auto const accepted = PlayerObject::pushButton(button);
        if (accepted && PlayLayer::get()) {
            zaidfx::PostProcessRenderer::get().trigger(
                zaidfx::ReactiveEvent::Jump
            );
        }
        return accepted;
    }

    void ringJump(RingObject* object, bool skipCheck) {
        PlayerObject::ringJump(object, skipCheck);
        if (PlayLayer::get()) {
            zaidfx::PostProcessRenderer::get().trigger(
                zaidfx::ReactiveEvent::Orb
            );
        }
    }

    void switchedToMode(GameObjectType type) {
        PlayerObject::switchedToMode(type);
        if (PlayLayer::get()) {
            zaidfx::PostProcessRenderer::get().trigger(
                zaidfx::ReactiveEvent::Portal
            );
        }
    }
};

class $modify(ZaidFXGameLayer, GJBaseGameLayer) {
    void pickupItem(EffectGameObject* object) {
        GJBaseGameLayer::pickupItem(object);

        if (PlayLayer::get()) {
            zaidfx::PostProcessRenderer::get().trigger(
                zaidfx::ReactiveEvent::Coin
            );
        }
    }
};

class $modify(ZaidFXPlayLayer, PlayLayer) {
    void destroyPlayer(PlayerObject* player, GameObject* object) {
        zaidfx::PostProcessRenderer::get().trigger(
            zaidfx::ReactiveEvent::Death
        );
        PlayLayer::destroyPlayer(player, object);
    }
};

$execute {
    zaidfx::PostProcessRenderer::get().initialize();
    registerSettingListeners();
    zaidfx::PresetManager::get().initialize();
}
