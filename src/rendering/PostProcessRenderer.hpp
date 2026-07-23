#pragma once

#include "ShaderProgram.hpp"
#include "../settings/Settings.hpp"

#include <Geode/cocos/platform/CCGL.h>

#include <string_view>

namespace zaidfx {

class PostProcessRenderer final {
public:
    static PostProcessRenderer& get();

    void initialize();
    void setBool(std::string_view key, bool value);
    void setFloat(std::string_view key, float value);

    // Called immediately before Android presents the completed frame.
    void processPresentedFrame();

private:
    PostProcessRenderer() = default;

    void invalidatePipeline();
    [[nodiscard]] bool shouldRender() const;
    [[nodiscard]] bool isPipelineReady() const;
    bool ensurePipeline(int width, int height);
    bool rebuildPipeline(int width, int height);
    void destroyPipeline();
    bool captureCurrentFramebuffer(int x, int y, int width, int height);
    bool applyUniforms(int width, int height);
    bool drawFullscreenQuad();

    Settings m_settings;
    ShaderProgram m_shader;
    GLuint m_captureTexture = 0;
    int m_textureWidth = 0;
    int m_textureHeight = 0;

    bool m_initialized = false;
    bool m_pipelineDirty = true;
    int m_retryFrames = 0;
};

} // namespace zaidfx
