#include "PostProcessRenderer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/cocos/platform/CCGL.h>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

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
        "[ZaidFX][settings] initial state: enabled={}, intensity={:.2f}, brightness={:.2f}, "
        "exposure={:.2f}, contrast={:.2f}, saturation={:.2f}, gamma={:.2f}, "
        "vignette={:.2f}, sharpen={:.2f}",
        m_settings.enabled,
        m_settings.intensity,
        m_settings.brightness,
        m_settings.exposure,
        m_settings.contrast,
        m_settings.saturation,
        m_settings.gamma,
        m_settings.vignette,
        m_settings.sharpen
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
    else {
        log::warn("[ZaidFX][settings] unknown bool setting: {}", key);
        return;
    }

    ++m_settingsRevision;
    m_uniformsDirty = true;

    if (m_settings.debugLogging) {
        log::info("[ZaidFX][settings->renderer] {} = {}", key, value);
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
    else if (key == "vignette") {
        m_settings.vignette = value;
    }
    else if (key == "sharpen") {
        m_settings.sharpen = value;
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

    if (m_settings.debugLogging) {
        log::info("[ZaidFX][settings->renderer] preset = {}", m_settings.preset);
    }
}

void PostProcessRenderer::invalidatePipeline(std::string_view reason) {
    m_pipelineDirty = true;
    m_retryFrames = 0;

    if (m_settings.debugLogging) {
        log::info("[ZaidFX][pipeline] invalidated: {}", reason);
    }
}

bool PostProcessRenderer::shouldRender() const {
    return m_initialized && m_settings.enabled;
}

bool PostProcessRenderer::isCapturing() const {
    return m_capturing;
}

bool PostProcessRenderer::isPipelineReady() const {
    return m_renderTexture && m_outputSprite && m_shader.isLoaded() && !m_pipelineDirty;
}

Settings const& PostProcessRenderer::settings() const {
    return m_settings;
}

bool PostProcessRenderer::beginCapture(CCDirector* director) {
    if (!director || !shouldRender() || m_capturing) {
        return false;
    }

    if (!ensurePipeline(director)) {
        return false;
    }

    m_capturing = true;
    m_renderTexture->beginWithClear(0.0f, 0.0f, 0.0f, 1.0f);
    return true;
}

void PostProcessRenderer::endCaptureAndDraw(CCDirector* director) {
    if (!m_capturing || !m_renderTexture || !m_outputSprite || !director) {
        m_capturing = false;
        return;
    }

    m_renderTexture->end();
    m_capturing = false;

    auto const winSize = director->getWinSize();
    m_outputSprite->setPosition(winSize / 2.0f);
    m_outputSprite->setTexture(m_renderTexture->getSprite()->getTexture());
    m_outputSprite->setShaderProgram(m_shader.get());

    GLfloat previousClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glGetFloatv(GL_COLOR_CLEAR_VALUE, previousClearColor);

    director->setViewport();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader.use();
    applyUniforms();
    m_outputSprite->visit();

    glClearColor(
        previousClearColor[0],
        previousClearColor[1],
        previousClearColor[2],
        previousClearColor[3]
    );

    logRenderStateOnce();
    m_uniformsDirty = false;
}

bool PostProcessRenderer::ensurePipeline(CCDirector* director) {
    if (m_retryFrames > 0) {
        --m_retryFrames;
        return false;
    }

    auto const size = director->getWinSize();
    auto const sizeChanged =
        std::fabs(size.width - m_renderSize.width) > 0.5f ||
        std::fabs(size.height - m_renderSize.height) > 0.5f;

    if (!m_pipelineDirty && !sizeChanged && isPipelineReady()) {
        return true;
    }

    if (!rebuildPipeline(size)) {
        m_retryFrames = 120;
        return false;
    }

    return true;
}

bool PostProcessRenderer::rebuildPipeline(CCSize const& size) {
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

    auto const width = std::max(1, static_cast<int>(std::ceil(size.width)));
    auto const height = std::max(1, static_cast<int>(std::ceil(size.height)));

    m_renderTexture = CCRenderTexture::create(
        width,
        height,
        kCCTexture2DPixelFormat_RGBA8888
    );

    if (!m_renderTexture) {
        log::error("[ZaidFX][pipeline] failed to create {}x{} render texture", width, height);
        m_shader.reset();
        return false;
    }
    m_renderTexture->retain();

    auto* texture = m_renderTexture->getSprite()->getTexture();
    m_outputSprite = CCSprite::createWithTexture(texture);
    if (!m_outputSprite) {
        log::error("[ZaidFX][pipeline] failed to create fullscreen output sprite");
        destroyPipeline();
        return false;
    }

    m_outputSprite->retain();
    m_outputSprite->setAnchorPoint({ 0.5f, 0.5f });
    m_outputSprite->setPosition(size / 2.0f);
    m_outputSprite->setFlipY(true);
    m_outputSprite->setBlendFunc({ GL_ONE, GL_ONE_MINUS_SRC_ALPHA });
    m_outputSprite->setShaderProgram(m_shader.get());

    m_renderSize = size;
    m_pipelineDirty = false;
    m_uniformsDirty = true;

    log::info(
        "[ZaidFX][pipeline] ready: points={}x{}, pixels={}x{}",
        size.width,
        size.height,
        texture->getPixelsWide(),
        texture->getPixelsHigh()
    );

    return true;
}

void PostProcessRenderer::destroyPipeline() {
    if (m_outputSprite) {
        m_outputSprite->release();
        m_outputSprite = nullptr;
    }

    if (m_renderTexture) {
        m_renderTexture->release();
        m_renderTexture = nullptr;
    }

    m_shader.reset();
    m_renderSize = CCSizeMake(0.0f, 0.0f);
}

void PostProcessRenderer::applyUniforms() {
    auto* texture = m_renderTexture->getSprite()->getTexture();
    auto const width = std::max(1, static_cast<int>(texture->getPixelsWide()));
    auto const height = std::max(1, static_cast<int>(texture->getPixelsHigh()));

    struct UniformValue {
        char const* uniform;
        char const* setting;
        float value;
    };

    UniformValue const values[] = {
        { "u_intensity", "effect-intensity", m_settings.intensity },
        { "u_brightness", "brightness", m_settings.brightness },
        { "u_exposure", "exposure", m_settings.exposure },
        { "u_contrast", "contrast", m_settings.contrast },
        { "u_saturation", "saturation", m_settings.saturation },
        { "u_gamma", "gamma", m_settings.gamma },
        { "u_vignette", "vignette", m_settings.vignette },
        { "u_sharpen", "sharpen", m_settings.sharpen },
    };

    for (auto const& entry : values) {
        auto const updated = m_shader.setFloat(entry.uniform, entry.value);
        if (!updated) {
            log::error(
                "[ZaidFX][uniform] missing or invalid uniform {} for setting {}",
                entry.uniform,
                entry.setting
            );
        }
        else if (m_uniformsDirty && m_settings.debugLogging) {
            log::info(
                "[ZaidFX][uniform] {} <- {:.4f} (setting: {})",
                entry.uniform,
                entry.value,
                entry.setting
            );
        }
    }

    auto const texelX = 1.0f / static_cast<float>(width);
    auto const texelY = 1.0f / static_cast<float>(height);
    auto const texelUpdated = m_shader.setVec2("u_texelSize", texelX, texelY);

    if (!texelUpdated) {
        log::error("[ZaidFX][uniform] missing or invalid uniform u_texelSize");
    }
    else if (m_uniformsDirty && m_settings.debugLogging) {
        log::info("[ZaidFX][uniform] u_texelSize <- ({:.7f}, {:.7f})", texelX, texelY);
    }
}

void PostProcessRenderer::markSettingsChanged(std::string_view key, float value) {
    ++m_settingsRevision;
    m_uniformsDirty = true;

    if (m_settings.debugLogging) {
        log::info(
            "[ZaidFX][settings->renderer] {} requested={:.4f}, sanitized={:.4f}",
            key,
            value,
            key == "effect-intensity" ? m_settings.intensity :
            key == "brightness" ? m_settings.brightness :
            key == "exposure" ? m_settings.exposure :
            key == "contrast" ? m_settings.contrast :
            key == "saturation" ? m_settings.saturation :
            key == "gamma" ? m_settings.gamma :
            key == "vignette" ? m_settings.vignette :
            m_settings.sharpen
        );
    }
}

void PostProcessRenderer::logRenderStateOnce() {
    if (!m_settings.debugLogging || m_lastLoggedRenderRevision == m_settingsRevision) {
        return;
    }

    m_lastLoggedRenderRevision = m_settingsRevision;
    log::info(
        "[ZaidFX][render] revision={} final values: intensity={:.2f}, brightness={:.2f}, "
        "exposure={:.2f}, contrast={:.2f}, saturation={:.2f}, gamma={:.2f}, "
        "vignette={:.2f}, sharpen={:.2f}",
        m_settingsRevision,
        m_settings.intensity,
        m_settings.brightness,
        m_settings.exposure,
        m_settings.contrast,
        m_settings.saturation,
        m_settings.gamma,
        m_settings.vignette,
        m_settings.sharpen
    );
}

} // namespace zaidfx
