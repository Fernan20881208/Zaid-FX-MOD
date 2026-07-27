#include "PostProcessRenderer.hpp"

#include <Geode/binding/FMODAudioEngine.hpp>

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace {

float percent(zaidfx::Settings const& settings, zaidfx::FloatParam id) {
    return settings.get(id) / 100.0f;
}

float decayFactor(float dt, float rate) {
    return std::exp(-dt * rate);
}

} // namespace

namespace zaidfx {

void PostProcessRenderer::updateReactiveState(float dt) {
    dt = std::clamp(dt, 0.0f, 0.1f);
    m_time += dt;

    auto const decayRate =
        2.0f + percent(m_settings, FloatParam::ReactiveDecay) * 10.0f;

    m_reactivePulse *= decayFactor(dt, decayRate);
    m_reactiveFlash *= decayFactor(dt, decayRate + 4.0f);

    if (!m_settings.get(BoolParam::ReactiveEnabled)) {
        m_smoothedMusic *= decayFactor(dt, 8.0f);
        return;
    }

    auto const musicTarget =
        musicLevel() * percent(m_settings, FloatParam::MusicSensitivity);
    auto const smoothing = 1.0f - decayFactor(dt, 12.0f);

    m_smoothedMusic += (musicTarget - m_smoothedMusic) * smoothing;

    auto const pulseStrength = percent(m_settings, FloatParam::PulseStrength);

    if (m_settings.get(BoolParam::ReactMusic)) {
        m_reactivePulse = std::max(
            m_reactivePulse,
            m_smoothedMusic * pulseStrength
        );
    }

    if (m_settings.get(BoolParam::ReactSpeed)) {
        m_reactivePulse = std::max(
            m_reactivePulse,
            m_gameplaySpeed * pulseStrength * 0.65f
        );
    }
}

float PostProcessRenderer::musicLevel() const {
    auto* engine = FMODAudioEngine::get();
    if (!engine) {
        return 0.0f;
    }

    auto const level = std::max(
        engine->m_musicVisualizerPeak,
        engine->m_musicVisualizerVolume
    );
    return std::clamp(level, 0.0f, 1.0f);
}

bool PostProcessRenderer::eventEnabled(ReactiveEvent event) const {
    switch (event) {
        case ReactiveEvent::Jump:
            return m_settings.get(BoolParam::ReactJump);
        case ReactiveEvent::Orb:
            return m_settings.get(BoolParam::ReactOrbs);
        case ReactiveEvent::Portal:
            return m_settings.get(BoolParam::ReactPortals);
        case ReactiveEvent::Coin:
            return m_settings.get(BoolParam::ReactCoins);
        case ReactiveEvent::Death:
            return m_settings.get(BoolParam::ReactDeath);
    }

    return false;
}

} // namespace zaidfx
