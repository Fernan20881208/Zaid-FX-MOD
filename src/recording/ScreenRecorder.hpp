#pragma once

#include <Geode/cocos/platform/CCGL.h>

#include <filesystem>
#include <memory>
#include <string>

namespace zaidfx {

enum class RecorderState {
    Idle,
    Starting,
    Recording,
    Pending
};

class ScreenRecorder final {
public:
    static ScreenRecorder& get();

    ScreenRecorder(ScreenRecorder const&) = delete;
    ScreenRecorder& operator=(ScreenRecorder const&) = delete;

    bool requestStart();
    bool requestStop();
    bool savePending();
    bool deletePending();

    [[nodiscard]] RecorderState state() const;
    [[nodiscard]] bool wantsFrames() const;
    [[nodiscard]] std::string statusMessage() const;
    [[nodiscard]] std::filesystem::path outputDirectory() const;
    [[nodiscard]] std::filesystem::path lastSavedPath() const;

    // Must be called on the active OpenGL render thread after the final frame
    // has been drawn to the framebuffer that will be presented.
    void captureFrame(int x, int y, int width, int height);

private:
    ScreenRecorder();
    ~ScreenRecorder();

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace zaidfx
