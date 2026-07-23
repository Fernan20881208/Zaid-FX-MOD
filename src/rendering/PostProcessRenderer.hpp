#pragma once

#include "ShaderProgram.hpp"
#include "../settings/Settings.hpp"

#include <Geode/cocos/platform/CCGL.h>

#include <string>
#include <string_view>

namespace zaidfx {

enum class ReactiveEvent {
    Jump,
    Orb,
    Portal,
    Coin,
    Death
};

class PostProcessRenderer final {
public:
    static PostProcessRenderer& get();

    void initialize();
    void reloadSettings();
    void setBool(std::string_view key, bool value);
    void setFloat(std::string_view key, float value);
    void setString(std::string_view key, std::string value);

    void trigger(ReactiveEvent event);
    void setGameplaySpeed(float speed);

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

    bool allocateTexture(GLuint& texture, int width, int height);
    bool captureCurrentFramebuffer(int x, int y, int width, int height);
    bool renderLightingPass();
    bool renderFinalPass(GLint targetFramebuffer, int x, int y, int width, int height);
    bool applyLightingUniforms();
    bool applyFinalUniforms(int width, int height);
    bool drawFullscreenQuad(GLuint texture0, GLuint texture1);

    void updateReactiveState(float dt);
    [[nodiscard]] float musicLevel() const;
    [[nodiscard]] bool eventEnabled(ReactiveEvent event) const;

    Settings m_settings;
    ShaderProgram m_lightingShader;
    ShaderProgram m_finalShader;

    GLuint m_captureTexture = 0;
    GLuint m_lightingTexture = 0;
    GLuint m_lightingFramebuffer = 0;

    int m_textureWidth = 0;
    int m_textureHeight = 0;
    int m_lightingWidth = 0;
    int m_lightingHeight = 0;
    int m_pipelineQuality = -1;

    bool m_initialized = false;
    bool m_pipelineDirty = true;
    bool m_processingFrame = false;
    bool m_loggedPipelineState = false;
    bool m_loggedReentry = false;
    int m_retryFrames = 0;

    float m_time = 0.0f;
    float m_reactivePulse = 0.0f;
    float m_reactiveFlash = 0.0f;
    float m_gameplaySpeed = 0.0f;
    float m_smoothedMusic = 0.0f;
};

} // namespace zaidfx
