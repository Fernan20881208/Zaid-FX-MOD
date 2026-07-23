#include "PostProcessRenderer.hpp"

#include <Geode/Geode.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace {

void prepareFullscreenState() {
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

} // namespace

namespace zaidfx {

bool PostProcessRenderer::rebuildPipeline(int width, int height) {
    destroyPipeline();

    auto const resources = Mod::get()->getResourcesDir();
    auto const vertexPath = resources / "fullscreen.vert";
    auto const lightingPath = resources / "lighting.frag";
    auto const finalPath = resources / "final.frag";

    if (!m_lightingShader.loadFromFiles(vertexPath, lightingPath)) {
        log::error("[ZaidFX] unable to load lighting shader");
        return false;
    }
    if (!m_finalShader.loadFromFiles(vertexPath, finalPath)) {
        log::error("[ZaidFX] unable to load final shader");
        return false;
    }

    GLint previousFramebuffer = 0;
    GLint previousActiveTexture = GL_TEXTURE0;
    GLint previousTexture = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &previousActiveTexture);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);

    while (glGetError() != GL_NO_ERROR) {}

    if (!allocateTexture(m_captureTexture, width, height)) {
        log::error("[ZaidFX] full-resolution capture texture allocation failed");
        destroyPipeline();
        return false;
    }

    auto const scale = m_settings.qualityScale();
    auto const lightingWidth = std::max(64, static_cast<int>(std::round(width * scale)));
    auto const lightingHeight = std::max(64, static_cast<int>(std::round(height * scale)));

    if (!allocateTexture(m_lightingTexture, lightingWidth, lightingHeight)) {
        log::error("[ZaidFX] reduced-resolution lighting texture allocation failed");
        destroyPipeline();
        return false;
    }

    glGenFramebuffers(1, &m_lightingFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_lightingFramebuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_lightingTexture,
        0
    );

    auto const framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    auto const error = glGetError();

    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
    glActiveTexture(static_cast<GLenum>(previousActiveTexture));

    if (
        m_lightingFramebuffer == 0 ||
        framebufferStatus != GL_FRAMEBUFFER_COMPLETE ||
        error != GL_NO_ERROR
    ) {
        log::error(
            "[ZaidFX] lighting framebuffer failed (status=0x{:X}, error=0x{:X})",
            framebufferStatus,
            error
        );
        destroyPipeline();
        return false;
    }

    m_textureWidth = width;
    m_textureHeight = height;
    m_lightingWidth = lightingWidth;
    m_lightingHeight = lightingHeight;
    m_pipelineQuality = m_settings.qualityLevel();
    m_pipelineDirty = false;
    return true;
}

void PostProcessRenderer::destroyPipeline() {
    if (m_lightingFramebuffer != 0) {
        glDeleteFramebuffers(1, &m_lightingFramebuffer);
        m_lightingFramebuffer = 0;
    }
    if (m_lightingTexture != 0) {
        glDeleteTextures(1, &m_lightingTexture);
        m_lightingTexture = 0;
    }
    if (m_captureTexture != 0) {
        glDeleteTextures(1, &m_captureTexture);
        m_captureTexture = 0;
    }

    m_lightingShader.reset();
    m_finalShader.reset();
    m_textureWidth = 0;
    m_textureHeight = 0;
    m_lightingWidth = 0;
    m_lightingHeight = 0;
    m_pipelineQuality = -1;
}

bool PostProcessRenderer::allocateTexture(GLuint& texture, int width, int height) {
    while (glGetError() != GL_NO_ERROR) {}

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
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
    if (texture == 0 || error != GL_NO_ERROR) {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
        return false;
    }
    return true;
}

bool PostProcessRenderer::captureCurrentFramebuffer(
    int x,
    int y,
    int width,
    int height
) {
    while (glGetError() != GL_NO_ERROR) {}
    glBindTexture(GL_TEXTURE_2D, m_captureTexture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, width, height);

    auto const error = glGetError();
    if (error != GL_NO_ERROR) {
        log::error("[ZaidFX] framebuffer copy failed (OpenGL error 0x{:X})", error);
        return false;
    }
    return true;
}

bool PostProcessRenderer::renderLightingPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_lightingFramebuffer);
    glViewport(0, 0, m_lightingWidth, m_lightingHeight);
    prepareFullscreenState();

    m_lightingShader.use();
    if (!applyLightingUniforms()) return false;
    return drawFullscreenQuad(m_captureTexture, m_captureTexture);
}

bool PostProcessRenderer::renderFinalPass(
    GLint targetFramebuffer,
    int x,
    int y,
    int width,
    int height
) {
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(targetFramebuffer));
    glViewport(x, y, width, height);
    prepareFullscreenState();

    m_finalShader.use();
    if (!applyFinalUniforms(width, height)) return false;

    auto const lightingSource = m_settings.hasLightingEffects()
        ? m_lightingTexture
        : m_captureTexture;
    return drawFullscreenQuad(m_captureTexture, lightingSource);
}

bool PostProcessRenderer::drawFullscreenQuad(GLuint texture0, GLuint texture1) {
    static GLfloat const vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE0);

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
