#pragma once

#include "ShaderProgram.hpp"
#include "../settings/Settings.hpp"

#include <Geode/cocos/misc_nodes/CCRenderTexture.h>
#include <Geode/cocos/sprite_nodes/CCSprite.h>

#include <cstdint>
#include <string_view>

namespace zaidfx {

class PostProcessRenderer final {
public:
    static PostProcessRenderer& get();

    void initialize();

    void setBool(std::string_view key, bool value);
    void setFloat(std::string_view key, float value);
    void setString(std::string_view key, std::string value);

    void invalidatePipeline(std::string_view reason);

    [[nodiscard]] bool shouldRender() const;
    [[nodiscard]] bool isCapturing() const;
    [[nodiscard]] bool isPipelineReady() const;
    [[nodiscard]] Settings const& settings() const;

    bool beginCapture(cocos2d::CCDirector* director);
    void endCaptureAndDraw(cocos2d::CCDirector* director);

private:
    PostProcessRenderer() = default;

    bool ensurePipeline(cocos2d::CCDirector* director);
    bool rebuildPipeline(cocos2d::CCSize const& size);
    void destroyPipeline();
    void applyUniforms();
    void markSettingsChanged(std::string_view key, float value);
    void logRenderStateOnce();

    Settings m_settings;
    ShaderProgram m_shader;
    cocos2d::CCRenderTexture* m_renderTexture = nullptr;
    cocos2d::CCSprite* m_outputSprite = nullptr;
    cocos2d::CCSize m_renderSize = { 0.0f, 0.0f };

    bool m_initialized = false;
    bool m_capturing = false;
    bool m_pipelineDirty = true;
    bool m_uniformsDirty = true;
    int m_retryFrames = 0;

    std::uint64_t m_settingsRevision = 0;
    std::uint64_t m_lastLoggedRenderRevision = static_cast<std::uint64_t>(-1);
};

} // namespace zaidfx
