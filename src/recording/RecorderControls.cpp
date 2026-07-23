#include "ScreenRecorder.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/FLAlertLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>

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
            return { 255, 100, 100 };
        case zaidfx::RecorderState::Pending:
            return { 255, 220, 90 };
        default:
            return { 255, 155, 155 };
    }
}

} // namespace

class $modify(ZaidFXRecorderMenu, MenuLayer) {
    struct Fields {
        ButtonSprite* sprite = nullptr;
    };

    bool init() {
        if (!MenuLayer::init()) return false;

        auto* menu = this->getChildByID("bottom-menu");
        if (!menu) return true;

        auto* sprite = ButtonSprite::create(
            buttonText(zaidfx::ScreenRecorder::get().state()),
            "goldFont.fnt",
            "GJ_button_01.png",
            0.72f
        );
        if (!sprite) return true;

        sprite->setScale(0.48f);
        sprite->setCascadeColorEnabled(true);
        sprite->setColor(buttonColor(zaidfx::ScreenRecorder::get().state()));

        auto* button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(ZaidFXRecorderMenu::onRecorder)
        );
        button->setID("screen-recorder-button"_spr);

        menu->addChild(button);
        menu->updateLayout();
        m_fields->sprite = sprite;
        return true;
    }

    void refreshRecorderButton() {
        auto* sprite = m_fields->sprite;
        if (!sprite) return;

        sprite->setString(buttonText(zaidfx::ScreenRecorder::get().state()));
        sprite->setColor(buttonColor(zaidfx::ScreenRecorder::get().state()));
    }

    void showStatus() {
        auto& recorder = zaidfx::ScreenRecorder::get();
        refreshRecorderButton();
        FLAlertLayer::create(
            "ZaidFX Recorder",
            recorder.statusMessage(),
            "OK"
        )->show();
    }

    void onRecorder(CCObject*) {
        auto& recorder = zaidfx::ScreenRecorder::get();

        switch (recorder.state()) {
            case zaidfx::RecorderState::Idle:
                createQuickPopup(
                    "Internal recorder",
                    "Record the final processed image as an <cy>MP4</c>. "
                    "Resolution and FPS follow the current ZaidFX quality. "
                    "This first version records <co>video only</c>.",
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
};
