#include "PostProcessRenderer.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>

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

    if (state.enabled == GL_TRUE) {
        glEnableVertexAttribArray(index);
    }
    else {
        glDisableVertexAttribArray(index);
    }
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
    glActiveTexture(static_cast<GLenum>(state.activeTexture));

    state.position = captureAttribute(cocos2d::kCCVertexAttrib_Position);
    state.texCoord = captureAttribute(cocos2d::kCCVertexAttrib_TexCoords);
    return state;
}

void setEnabled(GLenum capability, GLboolean enabled) {
    if (enabled == GL_TRUE) {
        glEnable(capability);
    }
    else {
        glDisable(capability);
    }
}

void restoreState(GLState const& state) {
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
    glUseProgram(static_cast<GLuint>(state.program));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(state.texture0Binding));
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
        state.colorMask[0],
        state.colorMask[1],
        state.colorMask[2],
        state.colorMask[3]
    );
    glViewport(state.viewport[0], state.viewport[1], state.viewport[2], state.viewport[3]);
    glScissor(
        state.scissorBox[0],
        state.scissorBox[1],
        state.scissorBox[2],
        state.scissorBox[3]
    );
}

} // namespace

namespace zaidfx {

PostProcessRenderer& PostProcessRenderer::get() {
    static auto* instance = new PostProcessRenderer();
    return *instance;
}

void PostProcessRenderer::initialize() {
    if (m_initialized) {
        return;
    }

    m_settings = Settings::read();
    m_settings.sanitize();
    m_initialized = true;
    m_uniformsDirty = true;
    ++m_settingsRevision;

    log::info(
        "[ZaidFX][settings] initialized: enabled={}, red-test={}, preset={}, intensity={:.2f}, "
        "brightness={:.2f}, exposure={:.2f}, contrast={:.2f}, saturation={:.2f}, "
        "gamma={:.2f}, bloom={:.2f}, vignette={:.2f}, sharpen={:.2f}, "
        "chromatic-aberration={:.2f}, tonemapping={:.2f}",
        m_settings.enabled,
        m_settings.debugRedScreen,
        m_settings.preset,
        m_settings.intensity,
        m_settings.brightness,
        m_settings.exposure,
        m_settings.contrast,
        m_settings.saturation,
        m_settings.gamma,
        m_settings.bloom,
        m_settings.vignette,
        m_settings.sharpen,
        m_settings.chromaticAberration,
        m_settings.tonemapping
    );
}

void PostProcessRenderer::setBool(std::string_view key, bool value) {
    if (key == "enabled") {
        m_settings.enabled = value;
        if (value) {
            invalidatePipeline("effects enabled");
        }
    }
    else if (key == "debug-logging") {
        m_settings.debugLogging = value;
    }
    else if (key == "debug-red-screen") {
        m_settings.debugRedScreen = value;
    }
    else {
        log::warn("[ZaidFX][settings] unknown bool setting: {}", key);
        return;
    }

    ++m_settingsRevision;
    m_uniformsDirty = true;
    log::info("[ZaidFX][slider->renderer] {} = {}", key, value);
}

void PostProcessRenderer::setFloat(std::string_view key, float value) {
    if (key == "effect-intensity") {
        m_settings.intensity = value;
    }
    else if (key == "brightness") {
        m_settings.brightness = value;
    }
    else if (key == "exposure") {
        m_settings.exposure = value;
    }
    else if (key == "contrast") {
        m_settings.contrast = value;
    }
    else if (key == "saturation") {
        m_settings.saturation = value;
    }
    else if (key == "gamma") {
        m_settings.gamma = value;
    }
    else if (key == "bloom") {
        m_settings.bloom = value;
    }
    else if (key == "vignette") {
        m_settings.vignette = value;
    }
    else if (key == "sharpen") {
        m_settings.sharpen = value;
    }
    else if (key == "chromatic-aberration") {
        m_settings.chromaticAberration = value;
    }
    else if (key == "tonemapping") {
        m_settings.tonemapping = value;
    }
    else {
        log::warn("[ZaidFX][settings] unknown float setting: {}", key);
        return;
    }

    m_settings.sanitize();
    markSettingsChanged(key, value);
}

void PostProcessRenderer::setString(std::string_view key, std::string value) {
    if (key != "preset") {
        log::warn("[ZaidFX][settings] unknown string setting: {}", key);
        return;
    }

    m_settings.preset = std::move(value);
    ++m_settingsRevision;
    m_uniformsDirty = true;
    log::info("[ZaidFX][preset->renderer] active preset = {}", m_settings.preset);
}

void PostProcessRenderer::invalidatePipeline(std::string_view reason) {
    m_pipelineDirty = true;
    m_retryFrames = 0;
    log::info("[ZaidFX][pipeline] invalidated: {}", reason);
}

bool PostProcessRenderer::shouldRender() const {
    return m_initialized && m_settings.enabled;
}

bool PostProcessRenderer::isPipelineReady() const {
    return m_captureTexture != 0 &&
        glIsTexture(m_captureTexture) == GL_TRUE &&
        m_shader.isValid() &&
        !m_pipelineDirty;
}

Settings const& PostProcessRenderer::settings() const {
    return m_settings;
}

void PostProcessRenderer::processPresentedFrame() {
    ++m_frameCounter;

    if (!shouldRender()) {
        return;
    }

    auto const state = captureState();
    auto const x = state.viewport[0];
    auto const y = state.viewport[1];
    auto const width = state.viewport[2];
    auto const height = state.viewport[3];

    if (width <= 0 || height <= 0) {
        if (m_settings.debugLogging) {
            log::warn("[ZaidFX][render-hook] invalid viewport {}x{}", width, height);
        }
        return;
    }

    if (m_settings.debugLogging && (m_frameCounter == 1 || m_frameCounter % 300 == 0)) {
        log::info(
            "[ZaidFX][render-hook] swapBuffers frame={} framebuffer={} viewport=({}, {}, {}x{})",
            m_frameCounter,
            state.framebuffer,
            x,
            y,
            width,
            height
        );
    }

    if (!ensurePipeline(width, height)) {
        restoreState(state);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
    glActiveTexture(GL_TEXTURE0);

    if (!captureCurrentFramebuffer(x, y, width, height)) {
        restoreState(state);
        invalidatePipeline("framebuffer capture failed");
        return;
    }

    glViewport(x, y, width, height);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    m_shader.use();

    auto const uniformsReady = applyUniforms(width, height);
    auto const quadDrawn = uniformsReady && drawFullscreenQuad();

    if (quadDrawn) {
        logRenderStateOnce(state.framebuffer, width, height);
        m_uniformsDirty = false;
    }

    restoreState(state);
}

bool PostProcessRenderer::ensurePipeline(int width, int height) {
    if (m_retryFrames > 0) {
        --m_retryFrames;
        return false;
    }

    auto const sizeChanged = width != m_textureWidth || height != m_textureHeight;
    auto const contextLost =
        (m_captureTexture != 0 && glIsTexture(m_captureTexture) != GL_TRUE) ||
        (m_shader.isLoaded() && !m_shader.isValid());

    if (contextLost) {
        invalidatePipeline("OpenGL context resources were lost");
    }

    if (!m_pipelineDirty && !sizeChanged && isPipelineReady()) {
        return true;
    }

    if (!rebuildPipeline(width, height)) {
        m_retryFrames = 120;
        return false;
    }

    return true;
}

bool PostProcessRenderer::rebuildPipeline(int width, int height) {
    destroyPipeline();

    auto const resources = Mod::get()->getResourcesDir();
    auto const vertexPath = resources / "fullscreen.vert";
    auto const fragmentPath = resources / "color-grade.frag";

    if (!m_shader.loadFromFiles(vertexPath, fragmentPath)) {
        log::error(
            "[ZaidFX][pipeline] unable to load shaders from {} and {}",
            vertexPath.string(),
            fragmentPath.string()
        );
        return false;
    }

    GLint previousActiveTexture = GL_TEXTURE0;
    GLint previousBinding = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousBinding);

    glGenTextures(1, &m_captureTexture);
    glBindTexture(GL_TEXTURE_2D, m_captureTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width,
        height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    auto const error = glGetError();
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousBinding));
    glActiveTexture(static_cast<GLenum>(previousActiveTexture));

    if (m_captureTexture == 0 || error != GL_NO_ERROR) {
        log::error(
            "[ZaidFX][pipeline] texture allocation failed: texture={}, size={}x{}, glError=0x{:X}",
            m_captureTexture,
            width,
            height,
            error
        );
        destroyPipeline();
        return false;
    }

    m_textureWidth = width;
    m_textureHeight = height;
    m_pipelineDirty = false;
    m_uniformsDirty = true;

    log::info(
        "[ZaidFX][pipeline] ready: program={}, captureTexture={}, size={}x{}",
        m_shader.programID(),
        m_captureTexture,
        width,
        height
    );
    return true;
}

void PostProcessRenderer::destroyPipeline() {
    if (m_captureTexture != 0) {
        glDeleteTextures(1, &m_captureTexture);
        m_captureTexture = 0;
    }

    m_shader.reset();
    m_textureWidth = 0;
    m_textureHeight = 0;
}

bool PostProcessRenderer::captureCurrentFramebuffer(int x, int y, int width, int height) {
    while (glGetError() != GL_NO_ERROR) {}

    glBindTexture(GL_TEXTURE_2D, m_captureTexture);
    glCopyTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        x,
        y,
        width,
        height
    );

    auto const error = glGetError();
    if (error != GL_NO_ERROR) {
        log::error(
            "[ZaidFX][capture] glCopyTexSubImage2D failed: framebuffer viewport={}x{}, texture={}, glError=0x{:X}",
            width,
            height,
            m_captureTexture,
            error
        );
        return false;
    }

    return true;
}

bool PostProcessRenderer::applyUniforms(int width, int height) {
    struct UniformValue final {
        char const* uniform;
        char const* setting;
        float value;
    };

    UniformValue const values[] = {
        { "u_debugRed", "debug-red-screen", m_settings.debugRedScreen ? 1.0f : 0.0f },
        { "u_intensity", "effect-intensity", m_settings.intensity },
        { "u_brightness", "brightness", m_settings.brightness },
        { "u_exposure", "exposure", m_settings.exposure },
        { "u_contrast", "contrast", m_settings.contrast },
        { "u_saturation", "saturation", m_settings.saturation },
        { "u_gamma", "gamma", m_settings.gamma },
        { "u_bloom", "bloom", m_settings.bloom },
        { "u_vignette", "vignette", m_settings.vignette },
        { "u_sharpen", "sharpen", m_settings.sharpen },
        { "u_chromaticAberration", "chromatic-aberration", m_settings.chromaticAberration },
        { "u_tonemapping", "tonemapping", m_settings.tonemapping },
    };

    bool success = m_shader.setInt("CC_Texture0", 0);
    if (!success) {
        log::error("[ZaidFX][uniform] invalid CC_Texture0 location");
    }

    for (auto const& entry : values) {
        auto const updated = m_shader.setFloat(entry.uniform, entry.value);
        success = success && updated;

        if (!updated) {
            log::error(
                "[ZaidFX][uniform] invalid location: {} for {}",
                entry.uniform,
                entry.setting
            );
        }
        else if (m_uniformsDirty && m_settings.debugLogging) {
            log::info(
                "[ZaidFX][uniform] {} <- {:.4f} ({})",
                entry.uniform,
                entry.value,
                entry.setting
            );
        }
    }

    auto const texelX = 1.0f / static_cast<float>(std::max(width, 1));
    auto const texelY = 1.0f / static_cast<float>(std::max(height, 1));
    auto const texelUpdated = m_shader.setVec2("u_texelSize", texelX, texelY);
    success = success && texelUpdated;

    if (!texelUpdated) {
        log::error("[ZaidFX][uniform] invalid u_texelSize location");
    }
    else if (m_uniformsDirty && m_settings.debugLogging) {
        log::info("[ZaidFX][uniform] u_texelSize <- ({:.7f}, {:.7f})", texelX, texelY);
    }

    if (success && m_uniformsDirty && m_settings.debugLogging) {
        log::info("[ZaidFX][uniform] shader update confirmed for preset {}", m_settings.preset);
    }

    return success;
}

bool PostProcessRenderer::drawFullscreenQuad() {
    static GLfloat const vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_captureTexture);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(cocos2d::kCCVertexAttrib_Position);
    glEnableVertexAttribArray(cocos2d::kCCVertexAttrib_TexCoords);
    glVertexAttribPointer(
        cocos2d::kCCVertexAttrib_Position,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GLfloat) * 4,
        vertices
    );
    glVertexAttribPointer(
        cocos2d::kCCVertexAttrib_TexCoords,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(GLfloat) * 4,
        vertices + 2
    );

    while (glGetError() != GL_NO_ERROR) {}
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    auto const error = glGetError();

    if (error != GL_NO_ERROR) {
        log::error("[ZaidFX][quad] draw failed: glError=0x{:X}", error);
        return false;
    }

    if (m_uniformsDirty && m_settings.debugLogging) {
        log::info(
            "[ZaidFX][quad] final fullscreen quad drawn with program={} texture={}",
            m_shader.programID(),
            m_captureTexture
        );
    }
    return true;
}

void PostProcessRenderer::markSettingsChanged(std::string_view key, float value) {
    ++m_settingsRevision;
    m_uniformsDirty = true;

    float sanitized = m_settings.tonemapping;
    if (key == "effect-intensity") sanitized = m_settings.intensity;
    else if (key == "brightness") sanitized = m_settings.brightness;
    else if (key == "exposure") sanitized = m_settings.exposure;
    else if (key == "contrast") sanitized = m_settings.contrast;
    else if (key == "saturation") sanitized = m_settings.saturation;
    else if (key == "gamma") sanitized = m_settings.gamma;
    else if (key == "bloom") sanitized = m_settings.bloom;
    else if (key == "vignette") sanitized = m_settings.vignette;
    else if (key == "sharpen") sanitized = m_settings.sharpen;
    else if (key == "chromatic-aberration") sanitized = m_settings.chromaticAberration;

    log::info(
        "[ZaidFX][slider->renderer] {} requested={:.4f}, final={:.4f}",
        key,
        value,
        sanitized
    );
}

void PostProcessRenderer::logRenderStateOnce(GLint framebuffer, int width, int height) {
    if (!m_settings.debugLogging || m_lastLoggedRenderRevision == m_settingsRevision) {
        return;
    }

    m_lastLoggedRenderRevision = m_settingsRevision;
    log::info(
        "[ZaidFX][render] revision={} framebuffer={} program={} texture={} size={}x{} "
        "preset={} red-test={} intensity={:.2f}, brightness={:.2f}, exposure={:.2f}, "
        "contrast={:.2f}, saturation={:.2f}, gamma={:.2f}, bloom={:.2f}, vignette={:.2f}, "
        "sharpen={:.2f}, chromatic-aberration={:.2f}, tonemapping={:.2f}",
        m_settingsRevision,
        framebuffer,
        m_shader.programID(),
        m_captureTexture,
        width,
        height,
        m_settings.preset,
        m_settings.debugRedScreen,
        m_settings.intensity,
        m_settings.brightness,
        m_settings.exposure,
        m_settings.contrast,
        m_settings.saturation,
        m_settings.gamma,
        m_settings.bloom,
        m_settings.vignette,
        m_settings.sharpen,
        m_settings.chromaticAberration,
        m_settings.tonemapping
    );
}

} // namespace zaidfx
