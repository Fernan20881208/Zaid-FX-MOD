#pragma once

#include "Settings.hpp"

#include <array>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace zaidfx {

struct Preset final {
    std::string name;
    bool enabled = true;
    std::string quality = "High";
    std::array<bool, kBoolParamCount> booleans {};
    std::array<float, kFloatParamCount> floats {};
};

class PresetManager final {
public:
    static PresetManager& get();

    void initialize();
    void queuePreset(std::string presetName);
    void applyPreset(std::string_view presetName);
    void markCustom();

    void resetAll();
    void resetCurrentSection();
    void saveCustomPreset();
    void loadCustomPreset();

    void handleAction(std::string_view action);
    [[nodiscard]] bool isApplyingPreset() const;

private:
    PresetManager();

    Preset makeBasePreset(std::string name) const;
    Preset const* findPreset(std::string_view name) const;
    void writePreset(Preset const& preset, bool updatePresetName);
    void reloadRenderer();
    void resetKeys(
        std::initializer_list<BoolParam> bools,
        std::initializer_list<FloatParam> floats
    );

    std::vector<Preset> m_presets;
    bool m_applyingPreset = false;
};

} // namespace zaidfx
