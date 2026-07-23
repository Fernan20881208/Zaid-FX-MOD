#include "PostProcessRenderer.hpp"

#include <Geode/binding/FMODAudioEngine.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace {

float normalized(zaidfx::Settings const& settings, zaidfx::FloatParam id) {
    return settings.get(id) * 0.01f;
}

} // namespace

namespace zaidfx {

void PostProcessRenderer::updateReactiveState(float dt) {
    dt = std::clamp(dt, 0.0f, 0.1f);
    m_time += dt;

    auto const decayControl = normalized(m_settings, FloatParam::ReactiveDecay);
    auto const decayRate = 2.0f + decayControl * 10.0f;
    auto const decay = std::exp(-dt * decayRate);

    m_reactivePulse *= decay;
    m_reactiveFlash *= std::exp(-dt * (decayRate + 4.0f));

    if (!m_settings.get(BoolParam::ReactiveEnabled)) {
        m_smoothedMusic *= std::exp(-dt * 8.0f);
        return;
    }

    auto const musicTarget = musicLevel() *
        normalized(m_settings, FloatParam::MusicSensitivity);
    auto const smoothing = 1.0f - std::exp(-dt * 12.0f);
    m_smoothedMusic += (musicTarget - m_smoothedMusic) * smoothing;

    if (m_settings.get(BoolParam::ReactMusic)) {
        m_reactivePulse = std::max(
            m_reactivePulse,
            m_smoothedMusic * normalized(m_settings, FloatParam::PulseStrength)
        );
    }

    if (m_settings.get(BoolParam::ReactSpeed)) {
        m_reactivePulse = std::max(
            m_reactivePulse,
            m_gameplaySpeed * normalized(m_settings, FloatParam::PulseStrength) * 0.65f
        );
    }
}

float PostProcessRenderer::musicLevel() const {
    auto* engine = FMODAudioEngine::get();
    if (!engine) return 0.0f;

    return std::clamp(
        std::max(engine->m_musicVisualizerPeak, engine->m_musicVisualizerVolume),
        0.0f,
        1.0f
    );
}

bool PostProcessRenderer::eventEnabled(ReactiveEvent event) const {
    switch (event) {
        case ReactiveEvent::Jump: return m_settings.get(BoolParam::ReactJump);
        case ReactiveEvent::Orb: return m_settings.get(BoolParam::ReactOrbs);
        case ReactiveEvent::Portal: return m_settings.get(BoolParam::ReactPortals);
        case ReactiveEvent::Coin: return m_settings.get(BoolParam::ReactCoins);
        case ReactiveEvent::Death: return m_settings.get(BoolParam::ReactDeath);
    }
    return false;
}

} // namespace zaidfx
