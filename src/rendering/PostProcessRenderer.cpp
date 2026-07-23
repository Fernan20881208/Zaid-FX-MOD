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
    static PostProcessRenderer instance;
    return instance;
}

void PostProcessRenderer::initialize() {
    if (m_initialized) {
        return;
    }

    m_settings = Settings::read();
    m_initialized = true;
}

void PostProcessRenderer::setBool(std::string_view key, bool value) {
    if (key != "enabled") {
        log::warn("[ZaidFX] unknown boolean setting: {}", key);
        return;
    }

    m_settings.enabled = value;
    if (value) {
        invalidatePipeline();
    }
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
        log::warn("[ZaidFX] unknown numeric setting: {}", key);
        return;
    }

    m_settings.sanitize();
}

void PostProcessRenderer::invalidatePipeline() {
    m_pipelineDirty = true;
    m_retryFrames = 0;
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

void PostProcessRenderer::processPresentedFrame() {
    if (!shouldRender()) {
        return;
    }

    auto const state = captureState();
    auto const x = state.viewport[0];
    auto const y = state.viewport[1];
    auto const width = state.viewport[2];
    auto const height = state.viewport[3];

    if (width <= 0 || height <= 0) {
        restoreState(state);
        return;
    }

    if (!ensurePipeline(width, height)) {
        restoreState(state);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(state.framebuffer));
    glActiveTexture(GL_TEXTURE0);

    if (!captureCurrentFramebuffer(x, y, width, height)) {
        restoreState(state);
        invalidatePipeline();
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
    if (applyUniforms(width, height)) {
        drawFullscreenQuad();
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
        invalidatePipeline();
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
        log::error("[ZaidFX] unable to load the packaged shaders");
        return false;
    }

    GLint previousActiveTexture = GL_TEXTURE0;
    GLint previousBinding = 0;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousBinding);

    while (glGetError() != GL_NO_ERROR) {}

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
            "[ZaidFX] texture allocation failed for {}x{} (OpenGL error 0x{:X})",
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
        log::error("[ZaidFX] framebuffer copy failed (OpenGL error 0x{:X})", error);
        return false;
    }

    return true;
}

bool PostProcessRenderer::applyUniforms(int width, int height) {
    struct UniformValue final {
        char const* name;
        float value;
    };

    UniformValue const values[] = {
        { "u_intensity", m_settings.intensity },
        { "u_brightness", m_settings.brightness },
        { "u_exposure", m_settings.exposure },
        { "u_contrast", m_settings.contrast },
        { "u_saturation", m_settings.saturation },
        { "u_gamma", m_settings.gamma },
        { "u_bloom", m_settings.bloom },
        { "u_vignette", m_settings.vignette },
        { "u_sharpen", m_settings.sharpen },
        { "u_chromaticAberration", m_settings.chromaticAberration },
        { "u_tonemapping", m_settings.tonemapping },
    };

    bool success = m_shader.setInt("CC_Texture0", 0);
    for (auto const& entry : values) {
        auto const updated = m_shader.setFloat(entry.name, entry.value);
        success = updated && success;
    }

    auto const texelX = 1.0f / static_cast<float>(std::max(width, 1));
    auto const texelY = 1.0f / static_cast<float>(std::max(height, 1));
    auto const texelUpdated = m_shader.setVec2("u_texelSize", texelX, texelY);
    return texelUpdated && success;
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
        log::error("[ZaidFX] fullscreen draw failed (OpenGL error 0x{:X})", error);
        return false;
    }

    return true;
}

} // namespace zaidfx
