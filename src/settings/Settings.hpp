#pragma once

#include <string>

namespace zaidfx {

struct Settings final {
    bool enabled = false;
    std::string preset = "Clean";
    float exposure = 0.0f;
    float contrast = 1.0f;
    float saturation = 1.0f;
    float gamma = 1.0f;
    float vignette = 0.0f;
    float sharpen = 0.0f;

    static Settings read();
};

} // namespace zaidfx
