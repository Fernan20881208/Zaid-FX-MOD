#pragma once

namespace zaidfx {

struct Settings final {
    bool enabled = false;

    float intensity = 1.0f;
    float brightness = 0.0f;
    float exposure = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    float gamma = 1.0f;
    float bloom = 0.0f;
    float vignette = 0.0f;
    float sharpen = 0.0f;
    float chromaticAberration = 0.0f;
    float tonemapping = 0.0f;

    static Settings read();
    void sanitize();
};

} // namespace zaidfx
