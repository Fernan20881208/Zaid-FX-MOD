#include "ScreenRecorder.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>
#include <Geode/modify/LevelSelectLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <cmath>

using namespace geode::prelude;

namespace {

zaidfx::ScreenRecorder& recorder() {
    return zaidfx::ScreenRecorder::get();
}

bool isRecording(zaidfx::RecorderState state) {
    return state == zaidfx::RecorderState::Starting ||
        state == zaidfx::RecorderState::Recording;
}

char const* buttonText(zaidfx::RecorderState state) {
    if (isRecording(state)) {
        return "STOP";
    }
    if (state == zaidfx::RecorderState::Pending) {
        return "SAVE";
    }
    return "REC";
}

ccColor3B buttonColor(zaidfx::RecorderState state) {
    if (isRecording(state)) {
        return { 255, 82, 82 };
    }
    if (state == zaidfx::RecorderState::Pending) {
        return { 255, 214, 82 };
    }
    return { 225, 110, 125 };
}

class RecorderOverlay final : public cocos2d::CCLayer {
public:
    static RecorderOverlay* create() {
        auto* overlay = new RecorderOverlay();
        if (overlay && overlay->init()) {
            overlay->autorelease();
            return overlay;
        }

        CC_SAFE_DELETE(overlay);
        return nullptr;
    }

    bool init() override {
        if (!CCLayer::init()) {
            return false;
        }

        setID("recorder-overlay"_spr);
        setAnchorPoint({ 0.0f, 0.0f });
        setPosition({ 0.0f, 0.0f });

        m_menu = cocos2d::CCMenu::create();
        if (!m_menu) {
            return false;
        }

        m_menu->setID("recorder-overlay-menu"_spr);
        m_menu->setZOrder(1);
        addChild(m_menu, 1);

        auto const state = recorder().state();
        m_buttonSprite = ButtonSprite::create(
            buttonText(state),
            "goldFont.fnt",
            "GJ_button_01.png",
            0.72f
        );

        cocos2d::CCNode* visual = m_buttonSprite;
        if (!visual) {
            m_fallbackLabel = cocos2d::CCLabelBMFont::create(
                buttonText(state),
                "bigFont.fnt"
            );
            visual = m_fallbackLabel;
        }
        if (!visual) {
            return false;
        }

        visual->setScale(0.44f);
        setVisualColor(buttonColor(state));

        m_button = CCMenuItemSpriteExtra::create(
            visual,
            this,
            menu_selector(RecorderOverlay::onRecorder)
        );
        if (!m_button) {
            return false;
        }

        m_button->setID("screen-recorder-floating-button"_spr);
        m_menu->addChild(m_button);

        refresh(true);
        scheduleUpdate();
        return true;
    }

    void update(float dt) override {
        m_elapsed += dt;
        refresh(false);
    }

private:
    void setVisualColor(ccColor3B color) {
        if (m_buttonSprite) {
            m_buttonSprite->setCascadeColorEnabled(true);
            m_buttonSprite->setColor(color);
        }
        if (m_fallbackLabel) {
            m_fallbackLabel->setColor(color);
        }
    }

    void updateAppearance(zaidfx::RecorderState state) {
        auto const* text = buttonText(state);
        auto const color = buttonColor(state);

        if (m_buttonSprite) {
            m_buttonSprite->setString(text);
        }
        if (m_fallbackLabel) {
            m_fallbackLabel->setString(text);
        }

        setVisualColor(color);
        m_lastState = state;
    }

    void updatePosition() {
        auto* director = cocos2d::CCDirector::sharedDirector();
        if (!director || !m_menu) {
            return;
        }

        auto const windowSize = director->getWinSize();
        setContentSize(windowSize);

        // Keep the recorder outside Geometry Dash menu layouts.
        m_menu->setPosition({
            windowSize.width - 34.0f,
            windowSize.height - 34.0f
        });
    }

    void updatePulse(zaidfx::RecorderState state) {
        if (!m_button) {
            return;
        }

        auto scale = 1.0f;
        if (isRecording(state)) {
            scale += std::sin(m_elapsed * 5.0f) * 0.045f;
        }

        m_button->setScale(scale);
    }

    void refresh(bool force) {
        auto const state = recorder().state();

        if (force || state != m_lastState) {
            updateAppearance(state);
        }

        updatePosition();
        updatePulse(state);
    }

    void showStatus() {
        refresh(true);
        FLAlertLayer::create(
            "ZaidFX Recorder",
            recorder().statusMessage(),
            "OK"
        )->show();
    }

    void confirmStart() {
        createQuickPopup(
            "Internal recorder",
            "Record the final processed image as an <cy>MP4</c>. "
            "The floating button remains available in menus, level lists "
            "and gameplay. This version records <co>video only</c>.",
            "Cancel",
            "Record",
            [this](FLAlertLayer*, bool confirmed) {
                if (confirmed) {
                    recorder().requestStart();
                }
                showStatus();
            }
        );
    }

    void confirmStop() {
        createQuickPopup(
            "Stop recording?",
            "The captured frames will be finalized into a temporary MP4. "
            "Afterward you can save or delete it.",
            "Continue",
            "Stop",
            [this](FLAlertLayer*, bool confirmed) {
                if (confirmed) {
                    recorder().requestStop();
                }
                showStatus();
            }
        );
    }

    void resolvePendingRecording() {
        createQuickPopup(
            "Recording ready",
            "Choose <cg>Save</c> to keep the MP4 in the mod recordings "
            "folder, or <cr>Delete</c> to remove it permanently.",
            "Delete",
            "Save",
            [this](FLAlertLayer*, bool save) {
                if (save) {
                    recorder().savePending();
                } else {
                    recorder().deletePending();
                }
                showStatus();
            }
        );
    }

    void onRecorder(cocos2d::CCObject*) {
        switch (recorder().state()) {
            case zaidfx::RecorderState::Idle:
                confirmStart();
                break;

            case zaidfx::RecorderState::Starting:
            case zaidfx::RecorderState::Recording:
                confirmStop();
                break;

            case zaidfx::RecorderState::Pending:
                resolvePendingRecording();
                break;
        }
    }

    cocos2d::CCMenu* m_menu = nullptr;
    CCMenuItemSpriteExtra* m_button = nullptr;
    ButtonSprite* m_buttonSprite = nullptr;
    cocos2d::CCLabelBMFont* m_fallbackLabel = nullptr;
    zaidfx::RecorderState m_lastState = zaidfx::RecorderState::Idle;
    float m_elapsed = 0.0f;
};

void attachRecorderOverlay(cocos2d::CCNode* host) {
    if (!host || host->getChildByID("recorder-overlay"_spr)) {
        return;
    }

    if (auto* overlay = RecorderOverlay::create()) {
        host->addChild(overlay, 10000);
    }
}

} // namespace

class $modify(ZaidFXRecorderMenu, MenuLayer) {
    void onEnter() {
        MenuLayer::onEnter();
        attachRecorderOverlay(this);
    }
};

class $modify(ZaidFXRecorderLevelBrowser, LevelBrowserLayer) {
    void onEnter() {
        LevelBrowserLayer::onEnter();
        attachRecorderOverlay(this);
    }
};

class $modify(ZaidFXRecorderLevelSelect, LevelSelectLayer) {
    void onEnter() {
        LevelSelectLayer::onEnter();
        attachRecorderOverlay(this);
    }
};

class $modify(ZaidFXRecorderPlay, PlayLayer) {
    void onEnter() {
        PlayLayer::onEnter();
        attachRecorderOverlay(this);
    }
};
