#include "PostProcessRenderer.hpp"

#include "../recording/ScreenRecorder.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

using namespace geode::prelude;

namespace {

struct AttributeState final {
    GLint enabled = GL_FALSE;
    GLint size = 0;
    GLint stride = 0;
    GLint type = GL_FLOAT;
    GLint normalized = GL_FALSE;
    GLint buffer = 0;
    void* pointer = nullptr;
};

struct GLState final {
    GLint program = 0;
    GLint framebuffer = 0;
    GLint arrayBuffer = 0;
    GLint elementArrayBuffer = 0;
    GLint activeTexture = GL_TEXTURE0;
    GLint activeTextureBinding = 0;
    GLint texture0Binding = 0;
    GLint texture1Binding = 0;
    GLint viewport[4] = { 0, 0, 0, 0 };
    GLint scissorBox[4] = { 0, 0, 0, 0 };
    GLint blendSrcRGB = GL_ONE;
    GLint blendDstRGB = GL_ZERO;
    GLint blendSrcAlpha = GL_ONE;
    GLint blendDstAlpha = GL_ZERO;
    GLboolean blend = GL_FALSE;
    GLboolean depth = GL_FALSE;
    GLboolean stencil = GL_FALSE;
    GLboolean scissor = GL_FALSE;
    GLboolean cull = GL_FALSE;
    GLboolean depthMask = GL_TRUE;
    GLboolean colorMask[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
    AttributeState position;
    AttributeState texCoord;
};

struct BoolReset final {
    bool& value;
    ~BoolReset() { value = false; }
};

AttributeState captureAttribute(GLuint index) {
    AttributeState state;
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &state.enabled);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &state.size);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &state.stride);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &state.type);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &state.normalized);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &state.buffer);
    glGetVertexAttribPointerv(index, GL_VERTEX_ATTRIB_ARRAY_POINTER, &state.pointer);
    return state;
}

void restoreAttribute(GLuint index, AttributeState const& state) {
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(state.buffer));
    glVertexAttribPointer(
        index,
        state.size,
        static_cast<GLenum>(state.type),
        static_cast<GLboolean>(state.normalized),
        state.stride,
        state.pointer
    );

    if (state.enabled == GL_TRUE) glEnableVertexAttribArray(index);
    else glDisableVertexAttribArray(index);
}

GLState captureState() {
    GLState state;
    glGetIntegerv(GL_CURRENT_PROGRAM, &state.program);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &state.framebuffer);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &state.arrayBuffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &state.elementArrayBuffer);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &state.activeTexture);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &state.activeTextureBinding);
    glGetIntegerv(GL_VIEWPORT, state.viewport);
    glGetIntegerv(GL_SCISSOR_BOX, state.scissorBox);
    glGetIntegerv(GL_BLEND_SRC_RGB, &state.blendSrcRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &state.blendDstRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &state.blendSrcAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &state.blendDstAlpha);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &state.depthMask);
    glGetBooleanv(GL_COLOR_WRITEMASK, state.colorMask);

    state.blend = glIsEnabled(GL_BLEND);
    state.depth = glIsEnabled(GL_DEPTH_TEST);
    state.stencil = glIsEnabled(GL_STENCIL_TEST);
    state.scissor = glIsEnabled(GL_SCISSOR_TEST);
    state.cull = glIsEnabled(GL_CULL_FACE);

    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &state.texture0Binding);
    glActiveTexture(GL_TEXTURE1);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &state.texture1Binding);
    glActiveTexture(static_cast<GLenum>(state.activeTexture));

    state.position = captureAttribute(cocos2d::kCCVertexAttrib_Position);
    state.texCoord = captureAttribute(cocos2d::kCCVertexAttrib_TexCoords);
    return state;
}

void setEnabled(GLenum capability, GLboolean enabled) {
    if (enabled == GL_TRUE) glEnable(capability);
    else glDisable(capability);
}

void restoreState(GLState const& state) {
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
    glUseProgram(static_cast<GLuint>(state.program));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(state.texture0Binding));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(state.texture1Binding));
    glActiveTexture(static_cast<GLenum>(state.activeTexture));
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(state.activeTextureBinding));

    restoreAttribute(cocos2d::kCCVertexAttrib_Position, state.position);
    restoreAttribute(cocos2d::kCCVertexAttrib_TexCoords, state.texCoord);
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(state.arrayBuffer));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(state.elementArrayBuffer));

    glBlendFuncSeparate(
        static_cast<GLenum>(state.blendSrcRGB),
        static_cast<GLenum>(state.blendDstRGB),
        static_cast<GLenum>(state.blendSrcAlpha),
        static_cast<GLenum>(state.blendDstAlpha)
    );
    setEnabled(GL_BLEND, state.blend);
    setEnabled(GL_DEPTH_TEST, state.depth);
    setEnabled(GL_STENCIL_TEST, state.stencil);
    setEnabled(GL_SCISSOR_TEST, state.scissor);
    setEnabled(GL_CULL_FACE, state.cull);

    glDepthMask(state.depthMask);
    glColorMask(
        state.colorMask[0], state.colorMask[1],
        state.colorMask[2], state.colorMask[3]
    );
    glViewport(
        state.viewport[0], state.viewport[1],
        state.viewport[2], state.viewport[3]
    );
    glScissor(
        state.scissorBox[0], state.scissorBox[1],
        state.scissorBox[2], state.scissorBox[3]
    );
}

float normalized(zaidfx::Settings const& settings, zaidfx::FloatParam id) {
    return settings.get(id) * 0.01f;
}

} // namespace

namespace zaidfx {

PostProcessRenderer& PostProcessRenderer::get() {
    static PostProcessRenderer instance;
    return instance;
}

void PostProcessRenderer::initialize() {
    if (m_initialized) return;
    m_settings = Settings::read();
    m_loggedPipelineState = false;
    m_initialized = true;
}

void PostProcessRenderer::reloadSettings() {
    auto const previousQuality = m_settings.qualityLevel();
    m_settings = Settings::read();
    m_loggedPipelineState = false;
    if (previousQuality != m_settings.qualityLevel()) invalidatePipeline();
}

void PostProcessRenderer::setBool(std::string_view key, bool value) {
    if (key == "enabled") {
        m_settings.enabled = value;
        m_loggedPipelineState = false;
        if (value) invalidatePipeline();
        return;
    }
    if (!m_settings.set(key, value)) {
        log::warn("[ZaidFX] unknown boolean setting: {}", key);
        return;
    }
    m_loggedPipelineState = false;
}

void PostProcessRenderer::setFloat(std::string_view key, float value) {
    if (!m_settings.set(key, value)) {
        log::warn("[ZaidFX] unknown numeric setting: {}", key);
        return;
    }
    m_loggedPipelineState = false;
}

void PostProcessRenderer::setString(std::string_view key, std::string value) {
    if (key != "quality") {
        log::warn("[ZaidFX] unknown string setting: {}", key);
        return;
    }

    auto const previousQuality = m_settings.qualityLevel();
    if (!m_settings.setQuality(std::move(value))) {
        log::warn("[ZaidFX] invalid quality value");
        return;
    }
    m_loggedPipelineState = false;
    if (previousQuality != m_settings.qualityLevel()) invalidatePipeline();
}

void PostProcessRenderer::trigger(ReactiveEvent event) {
    if (!m_settings.get(BoolParam::ReactiveEnabled) || !eventEnabled(event)) return;

    auto const pulse = normalized(m_settings, FloatParam::PulseStrength);
    auto const flash = normalized(m_settings, FloatParam::FlashStrength);
    m_reactivePulse = std::max(m_reactivePulse, pulse);

    if (event == ReactiveEvent::Death) {
        m_reactiveFlash = std::max(m_reactiveFlash, flash);
    }
    else if (event == ReactiveEvent::Portal || event == ReactiveEvent::Coin) {
        m_reactiveFlash = std::max(m_reactiveFlash, flash * 0.55f);
    }
    else {
        m_reactiveFlash = std::max(m_reactiveFlash, flash * 0.25f);
    }
}

void PostProcessRenderer::setGameplaySpeed(float speed) {
    m_gameplaySpeed = std::clamp(speed, 0.0f, 1.0f);
}

void PostProcessRenderer::invalidatePipeline() {
    m_pipelineDirty = true;
    m_loggedPipelineState = false;
    m_retryFrames = 0;
}

bool PostProcessRenderer::shouldRender() const {
    return m_initialized && m_settings.enabled && m_settings.hasVisibleEffects();
}

bool PostProcessRenderer::isPipelineReady() const {
    return m_captureTexture != 0 && glIsTexture(m_captureTexture) == GL_TRUE &&
        m_lightingTexture != 0 && glIsTexture(m_lightingTexture) == GL_TRUE &&
        m_lightingFramebuffer != 0 && glIsFramebuffer(m_lightingFramebuffer) == GL_TRUE &&
        m_lightingShader.isValid() && m_finalShader.isValid() && !m_pipelineDirty;
}

void PostProcessRenderer::processPresentedFrame() {
    if (m_processingFrame) {
        if (!m_loggedReentry) {
            log::warn("[ZaidFX] ignored a re-entrant post-process call; the shader is applied once per presented frame");
            m_loggedReentry = true;
        }
        return;
    }

    m_processingFrame = true;
    BoolReset processingReset { m_processingFrame };

    auto& recorder = ScreenRecorder::get();
    auto const renderEffects = shouldRender();
    auto const recordFrame = recorder.wantsFrames();
    if (!renderEffects && !recordFrame) return;

    auto const state = captureState();
    auto const x = state.viewport[0];
    auto const y = state.viewport[1];
    auto const width = state.viewport[2];
    auto const height = state.viewport[3];

    if (width <= 0 || height <= 0) {
        restoreState(state);
        return;
    }

    bool renderedSuccessfully = true;
    if (renderEffects) {
        if (!ensurePipeline(width, height)) {
            renderedSuccessfully = false;
        }
        else {
            glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
            glActiveTexture(GL_TEXTURE0);
            if (!captureCurrentFramebuffer(x, y, width, height)) {
                renderedSuccessfully = false;
            }
            else {
                auto* director = cocos2d::CCDirector::sharedDirector();
                updateReactiveState(
                    director ? director->getDeltaTime() : (1.0f / 60.0f)
                );

                if (m_settings.hasLightingEffects()) {
                    renderedSuccessfully = renderLightingPass();
                }
                if (renderedSuccessfully) {
                    renderedSuccessfully = renderFinalPass(
                        state.framebuffer,
                        x,
                        y,
                        width,
                        height
                    );
                }
            }
        }

        if (!renderedSuccessfully) {
            invalidatePipeline();
            glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
            glViewport(x, y, width, height);
        }
        else if (!m_loggedPipelineState) {
            auto const preset = Mod::get()->getSettingValue<std::string>("preset");
            log::debug(
                "[ZaidFX] preset={} framebuffer={} viewport={}x{} captureTex={} lightingFbo={} lighting={}x{} lightingEffects={} finalEffects={} intensity={} exposure={} contrast={} saturation={} gamma={}",
                preset,
                state.framebuffer,
                width,
                height,
                m_captureTexture,
                m_lightingFramebuffer,
                m_lightingWidth,
                m_lightingHeight,
                m_settings.hasLightingEffects(),
                m_settings.hasFinalEffects(),
                m_settings.get(FloatParam::EffectIntensity),
                m_settings.get(FloatParam::Exposure),
                m_settings.get(FloatParam::Contrast),
                m_settings.get(FloatParam::Saturation),
                m_settings.get(FloatParam::Gamma)
            );
            m_loggedPipelineState = true;
        }
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
        glViewport(x, y, width, height);
    }

    // Capture after the optional final pass so the MP4 contains exactly the
    // image presented by ZaidFX. The recorder remains completely idle when it
    // has not been started.
    if (recorder.wantsFrames()) {
        recorder.captureFrame(x, y, width, height);
    }

    restoreState(state);
}

bool PostProcessRenderer::ensurePipeline(int width, int height) {
    if (m_retryFrames > 0) {
        --m_retryFrames;
        return false;
    }

    auto const qualityChanged = m_pipelineQuality != m_settings.qualityLevel();
    auto const sizeChanged = width != m_textureWidth || height != m_textureHeight;
    auto const contextLost =
        (m_captureTexture != 0 && glIsTexture(m_captureTexture) != GL_TRUE) ||
        (m_lightingTexture != 0 && glIsTexture(m_lightingTexture) != GL_TRUE) ||
        (m_lightingFramebuffer != 0 && glIsFramebuffer(m_lightingFramebuffer) != GL_TRUE) ||
        (m_lightingShader.isLoaded() && !m_lightingShader.isValid()) ||
        (m_finalShader.isLoaded() && !m_finalShader.isValid());

    if (contextLost || qualityChanged) invalidatePipeline();
    if (!m_pipelineDirty && !sizeChanged && isPipelineReady()) return true;

    if (!rebuildPipeline(width, height)) {
        m_retryFrames = 120;
        return false;
    }
    return true;
}

} // namespace zaidfx
