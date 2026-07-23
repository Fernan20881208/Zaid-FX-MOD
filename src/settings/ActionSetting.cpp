#include "PresetManager.hpp"

#include <Geode/Geode.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/loader/SettingV3.hpp>

#include <utility>

using namespace geode::prelude;

namespace zaidfx {

class ActionSettingV3 final : public SettingV3 {
public:
    static Result<std::shared_ptr<SettingV3>> parse(
        std::string const& key,
        std::string const& modID,
        matjson::Value const& json
    ) {
        auto result = std::make_shared<ActionSettingV3>();
        auto root = checkJson(json, "ActionSettingV3");

        result->init(key, modID, root);
        result->parseNameAndDescription(root);
        result->parseEnableIf(root);
        root.needs("action").into(result->m_action);
        root.checkUnknownKeys();

        return root.ok(std::static_pointer_cast<SettingV3>(result));
    }

    bool load(matjson::Value const&) override {
        return true;
    }

    bool save(matjson::Value&) const override {
        return true;
    }

    bool isDefaultValue() const override {
        return true;
    }

    void reset() override {}

    SettingNodeV3* createNode(float width) override;

    [[nodiscard]] std::string const& action() const {
        return m_action;
    }

private:
    std::string m_action;
};

class ActionSettingNodeV3 final : public SettingNodeV3 {
protected:
    bool init(std::shared_ptr<ActionSettingV3> setting, float width) {
        if (!SettingNodeV3::init(setting, width)) {
            return false;
        }

        m_buttonSprite = ButtonSprite::create(
            "Run",
            "goldFont.fnt",
            "GJ_button_01.png",
            0.8f
        );
        m_buttonSprite->setScale(0.55f);

        m_button = CCMenuItemSpriteExtra::create(
            m_buttonSprite,
            this,
            menu_selector(ActionSettingNodeV3::onButton)
        );

        getButtonMenu()->addChildAtPosition(m_button, Anchor::Center);
        getButtonMenu()->setContentWidth(60.0f);
        getButtonMenu()->updateLayout();
        updateState(nullptr);
        return true;
    }

    void updateState(CCNode* invoker) override {
        SettingNodeV3::updateState(invoker);

        auto const enabled = getSetting()->shouldEnable();
        m_button->setEnabled(enabled);
        m_buttonSprite->setCascadeColorEnabled(true);
        m_buttonSprite->setCascadeOpacityEnabled(true);
        m_buttonSprite->setOpacity(enabled ? 255 : 155);
        m_buttonSprite->setColor(enabled ? ccWHITE : ccGRAY);
    }

    void onButton(CCObject*) {
        PresetManager::get().handleAction(getSetting()->action());
    }

    void onCommit() override {}
    void onResetToDefault() override {}

public:
    static ActionSettingNodeV3* create(
        std::shared_ptr<ActionSettingV3> setting,
        float width
    ) {
        auto* result = new ActionSettingNodeV3();
        if (result->init(std::move(setting), width)) {
            result->autorelease();
            return result;
        }

        delete result;
        return nullptr;
    }

    bool hasUncommittedChanges() const override {
        return false;
    }

    bool hasNonDefaultValue() const override {
        return false;
    }

    std::shared_ptr<ActionSettingV3> getSetting() const {
        return std::static_pointer_cast<ActionSettingV3>(
            SettingNodeV3::getSetting()
        );
    }

private:
    ButtonSprite* m_buttonSprite = nullptr;
    CCMenuItemSpriteExtra* m_button = nullptr;
};

SettingNodeV3* ActionSettingV3::createNode(float width) {
    return ActionSettingNodeV3::create(
        std::static_pointer_cast<ActionSettingV3>(shared_from_this()),
        width
    );
}

$on_mod(Loaded) {
    auto result = Mod::get()->registerCustomSettingType(
        "action-button",
        &ActionSettingV3::parse
    );

    if (!result.isOk()) {
        log::error("[ZaidFX] unable to register action setting: {}", result.unwrapErr());
    }
}

} // namespace zaidfx
