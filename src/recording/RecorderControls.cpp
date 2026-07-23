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

char const* buttonText(zaidfx::RecorderState state) {
    switch (state) {
        case zaidfx::RecorderState::Starting:
        case zaidfx::RecorderState::Recording:
            return "STOP";
        case zaidfx::RecorderState::Pending:
            return "SAVE";
        default:
            return "REC";
    }
}

ccColor3B buttonColor(zaidfx::RecorderState state) {
    switch (state) {
        case zaidfx::RecorderState::Starting:
        case zaidfx::RecorderState::Recording:
            return { 255, 82, 82 };
        case zaidfx::RecorderState::Pending:
            return { 255, 214, 82 };
        default:
            return { 225, 110, 125 };
    }
}

class RecorderOverlay final : public cocos2d::CCLayer {
public:
    static RecorderOverlay* create() {
        auto* result = new RecorderOverlay();
        if (result && result->init()) {
            result->autorelease();
            return result;
        }
        CC_SAFE_DELETE(result);
        return nullptr;
    }

    bool init() override {
        if (!CCLayer::init()) return false;

        setID("recorder-overlay"_spr);
        setAnchorPoint({ 0.0f, 0.0f });
        setPosition({ 0.0f, 0.0f });

        m_menu = cocos2d::CCMenu::create();
        if (!m_menu) return false;
        m_menu->setID("recorder-overlay-menu"_spr);
        m_menu->setZOrder(1);
        addChild(m_menu, 1);

        m_buttonSprite = ButtonSprite::create(
            buttonText(zaidfx::ScreenRecorder::get().state()),
            "goldFont.fnt",
            "GJ_button_01.png",
            0.72f
        );

        cocos2d::CCNode* visual = m_buttonSprite;
        if (!visual) {
            m_fallbackLabel = cocos2d::CCLabelBMFont::create("REC", "bigFont.fnt");
            visual = m_fallbackLabel;
        }
        if (!visual) return false;

        visual->setScale(0.44f);
        visual->setCascadeColorEnabled(true);
        visual->setColor(buttonColor(zaidfx::ScreenRecorder::get().state()));

        m_button = CCMenuItemSpriteExtra::create(
            visual,
            this,
            menu_selector(RecorderOverlay::onRecorder)
        );
        if (!m_button) return false;
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
    void refresh(bool force) {
        auto const state = zaidfx::ScreenRecorder::get().state();
        if (force || state != m_lastState) {
            if (m_buttonSprite) {
                m_buttonSprite->setString(buttonText(state));
                m_buttonSprite->setColor(buttonColor(state));
            }
            if (m_fallbackLabel) {
                m_fallbackLabel->setString(buttonText(state));
                m_fallbackLabel->setColor(buttonColor(state));
            }
            m_lastState = state;
        }

        auto* director = cocos2d::CCDirector::sharedDirector();
        if (director && m_menu) {
            auto const size = director->getWinSize();
            setContentSize(size);
            // Independent top-right overlay: it never participates in a game
            // menu layout, so it cannot move or deform existing buttons.
            m_menu->setPosition({ size.width - 34.0f, size.height - 34.0f });
        }

        float scale = 1.0f;
        if (
            state == zaidfx::RecorderState::Starting ||
            state == zaidfx::RecorderState::Recording
        ) {
            scale = 1.0f + std::sin(m_elapsed * 5.0f) * 0.045f;
        }
        if (m_button) m_button->setScale(scale);
    }

    void showStatus() {
        refresh(true);
        FLAlertLayer::create(
            "ZaidFX Recorder",
            zaidfx::ScreenRecorder::get().statusMessage(),
            "OK"
        )->show();
    }

    void onRecorder(cocos2d::CCObject*) {
        auto& recorder = zaidfx::ScreenRecorder::get();

        switch (recorder.state()) {
            case zaidfx::RecorderState::Idle:
                createQuickPopup(
                    "Internal recorder",
                    "Record the final processed image as an <cy>MP4</c>. "
                    "The floating button remains available in menus, level lists "
                    "and gameplay. This version records <co>video only</c>.",
                    "Cancel",
                    "Record",
                    [this](FLAlertLayer*, bool start) {
                        if (start) zaidfx::ScreenRecorder::get().requestStart();
                        showStatus();
                    }
                );
                break;

            case zaidfx::RecorderState::Starting:
            case zaidfx::RecorderState::Recording:
                createQuickPopup(
                    "Stop recording?",
                    "The captured frames will be finalized into a temporary MP4. "
                    "Afterward you can save or delete it.",
                    "Continue",
                    "Stop",
                    [this](FLAlertLayer*, bool stop) {
                        if (stop) zaidfx::ScreenRecorder::get().requestStop();
                        showStatus();
                    }
                );
                break;

            case zaidfx::RecorderState::Pending:
                createQuickPopup(
                    "Recording ready",
                    "Choose <cg>Save</c> to keep the MP4 in the mod recordings "
                    "folder, or <cr>Delete</c> to remove it permanently.",
                    "Delete",
                    "Save",
                    [this](FLAlertLayer*, bool save) {
                        auto& current = zaidfx::ScreenRecorder::get();
                        if (save) current.savePending();
                        else current.deletePending();
                        showStatus();
                    }
                );
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
    if (!host || host->getChildByID("recorder-overlay"_spr)) return;
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
