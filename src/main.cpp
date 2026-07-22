#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "settings/Settings.hpp"

using namespace geode::prelude;

namespace {

std::string statusText() {
    auto settings = zaidfx::Settings::read();
    return fmt::format(
        "<cg>Zaid-FX-MOD</c> foundation is loaded.\n\n"
        "Effects: <c{}>{}</c>\n"
        "Preset: <cy>{}</c>\n\n"
        "The first release contains the settings and GLSL API. "
        "Global framebuffer post-processing is the next milestone.",
        settings.enabled ? "g" : "r",
        settings.enabled ? "enabled" : "disabled",
        settings.preset
    );
}

} // namespace

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

        auto* icon = CCSprite::createWithSpriteFrameName("GJ_starBtn_001.png");
        icon->setScale(0.65f);

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
    log::info("Zaid-FX-MOD v0.1.0 initialized on Android");
}
