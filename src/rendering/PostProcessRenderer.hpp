#pragma once

#include "ShaderProgram.hpp"
#include "../settings/Settings.hpp"

#include <Geode/cocos/platform/CCGL.h>

#include <cstdint>
#include <string>
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
    [[nodiscard]] bool isPipelineReady() const;
    [[nodiscard]] Settings const& settings() const;

    // Called from CCEGLView::swapBuffers after Geometry Dash has finished
    // drawing the frame and immediately before it is presented.
    void processPresentedFrame();

private:
    PostProcessRenderer() = default;

    bool ensurePipeline(int width, int height);
    bool rebuildPipeline(int width, int height);
    void destroyPipeline();
    bool captureCurrentFramebuffer(int x, int y, int width, int height);
    bool applyUniforms(int width, int height);
    bool drawFullscreenQuad();
    void markSettingsChanged(std::string_view key, float value);
    void logRenderStateOnce(GLint framebuffer, int width, int height);

    Settings m_settings;
    ShaderProgram m_shader;
    GLuint m_captureTexture = 0;
    int m_textureWidth = 0;
    int m_textureHeight = 0;

    bool m_initialized = false;
    bool m_pipelineDirty = true;
    bool m_uniformsDirty = true;
    int m_retryFrames = 0;

    std::uint64_t m_frameCounter = 0;
    std::uint64_t m_settingsRevision = 0;
    std::uint64_t m_lastLoggedRenderRevision = static_cast<std::uint64_t>(-1);
};

} // namespace zaidfx
