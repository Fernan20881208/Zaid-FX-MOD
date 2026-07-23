#include "ScreenRecorder.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/string.hpp>

#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>
#include <media/NdkMediaMuxer.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <fcntl.h>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace {

constexpr int32_t kColorFormatYuv420Flexible = 0x7F420888;
constexpr std::size_t kMaximumQueuedFrames = 2;

std::uint8_t clampByte(int value) {
    return static_cast<std::uint8_t>(std::clamp(value, 0, 255));
}

std::string pathText(std::filesystem::path const& path) {
    return geode::utils::string::pathToString(path);
}

struct RecordingProfile final {
    int maximumHeight = 720;
    int framesPerSecond = 24;
    int bitrateMultiplier = 13;
};

RecordingProfile profileForQuality(std::string const& quality) {
    if (quality == "Low") return { 480, 15, 10 };
    if (quality == "High") return { 720, 30, 15 };
    if (quality == "Ultra") return { 1080, 30, 16 };
    return { 720, 24, 13 };
}

std::string timestampName() {
    auto const now = std::chrono::system_clock::now();
    auto const time = std::chrono::system_clock::to_time_t(now);
    std::tm local {};
    localtime_r(&time, &local);

    std::ostringstream stream;
    stream << "ZaidFX-" << std::put_time(&local, "%Y%m%d-%H%M%S") << ".mp4";
    return stream.str();
}

} // namespace

namespace zaidfx {

struct ScreenRecorder::Impl final {
    struct Frame final {
        int width = 0;
        int height = 0;
        std::int64_t presentationTimeUs = 0;
        std::vector<std::uint8_t> rgba;
    };

    mutable std::mutex mutex;
    std::condition_variable condition;
    std::deque<Frame> frames;
    std::thread worker;

    std::atomic<RecorderState> state { RecorderState::Idle };
    bool stopWorker = false;
    bool encoderFailed = false;

    int sourceWidth = 0;
    int sourceHeight = 0;
    int outputWidth = 0;
    int outputHeight = 0;
    int framesPerSecond = 24;
    std::int64_t frameIndex = 0;
    std::chrono::steady_clock::time_point nextCapture {};

    AMediaCodec* codec = nullptr;
    AMediaMuxer* muxer = nullptr;
    int fileDescriptor = -1;
    int trackIndex = -1;
    bool muxerStarted = false;

    std::filesystem::path directory;
    std::filesystem::path pendingPath;
    std::filesystem::path savedPath;
    std::string message = "Recorder ready.";

    Impl() {
        directory = Mod::get()->getSaveDir() / "recordings";
        pendingPath = directory / "pending-recording.mp4";

        std::error_code error;
        std::filesystem::create_directories(directory, error);
        if (!error && std::filesystem::exists(pendingPath, error)) {
            state.store(RecorderState::Pending);
            message = "A recording is waiting to be saved or deleted.";
        }
    }

    ~Impl() {
        stopRecording();
        cleanupMedia();
    }

    void setMessage(std::string value) {
        std::lock_guard lock(mutex);
        message = std::move(value);
    }

    std::string getMessage() const {
        std::lock_guard lock(mutex);
        return message;
    }

    bool requestStart() {
        auto const current = state.load();
        if (current == RecorderState::Recording || current == RecorderState::Starting) {
            setMessage("A recording is already running.");
            return false;
        }
        if (current == RecorderState::Pending) {
            setMessage("Save or delete the previous recording first.");
            return false;
        }

        state.store(RecorderState::Starting);
        setMessage("Recording will start on the next rendered frame.");
        return true;
    }

    bool requestStop() {
        auto const current = state.load();
        if (current == RecorderState::Starting) {
            state.store(RecorderState::Idle);
            setMessage("Recording start was cancelled.");
            return true;
        }
        if (current != RecorderState::Recording) {
            setMessage("There is no active recording.");
            return false;
        }

        stopRecording();
        return state.load() == RecorderState::Pending;
    }

    bool begin(int width, int height) {
        sourceWidth = width;
        sourceHeight = height;

        auto const quality = Mod::get()->getSettingValue<std::string>("quality");
        auto const profile = profileForQuality(quality);
        framesPerSecond = profile.framesPerSecond;

        auto const scale = std::min(
            1.0,
            static_cast<double>(profile.maximumHeight) /
                static_cast<double>(std::max(height, 1))
        );
        outputWidth = std::max(2, static_cast<int>(width * scale) & ~1);
        outputHeight = std::max(2, static_cast<int>(height * scale) & ~1);

        std::error_code fsError;
        std::filesystem::create_directories(directory, fsError);
        if (fsError) {
            state.store(RecorderState::Idle);
            setMessage("Unable to create the recordings directory.");
            return false;
        }
        std::filesystem::remove(pendingPath, fsError);

        codec = AMediaCodec_createEncoderByType("video/avc");
        if (!codec) {
            state.store(RecorderState::Idle);
            setMessage("Android did not provide an H.264 encoder.");
            return false;
        }

        auto* format = AMediaFormat_new();
        AMediaFormat_setString(format, AMEDIAFORMAT_KEY_MIME, "video/avc");
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_WIDTH, outputWidth);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_HEIGHT, outputHeight);
        AMediaFormat_setInt32(
            format,
            AMEDIAFORMAT_KEY_BIT_RATE,
            std::clamp(
                outputWidth * outputHeight * framesPerSecond *
                    profile.bitrateMultiplier / 100,
                1'500'000,
                14'000'000
            )
        );
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_FRAME_RATE, framesPerSecond);
        AMediaFormat_setInt32(format, AMEDIAFORMAT_KEY_I_FRAME_INTERVAL, 2);
        AMediaFormat_setInt32(
            format,
            AMEDIAFORMAT_KEY_COLOR_FORMAT,
            kColorFormatYuv420Flexible
        );

        auto const configured = AMediaCodec_configure(
            codec,
            format,
            nullptr,
            nullptr,
            AMEDIACODEC_CONFIGURE_FLAG_ENCODE
        );
        AMediaFormat_delete(format);

        if (configured != AMEDIA_OK || AMediaCodec_start(codec) != AMEDIA_OK) {
            cleanupMedia();
            state.store(RecorderState::Idle);
            setMessage("The Android H.264 encoder could not be started.");
            return false;
        }

        auto const path = pathText(pendingPath);
        fileDescriptor = open(path.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0600);
        if (fileDescriptor < 0) {
            cleanupMedia();
            state.store(RecorderState::Idle);
            setMessage("Unable to create the temporary MP4 file.");
            return false;
        }

        muxer = AMediaMuxer_new(fileDescriptor, AMEDIAMUXER_OUTPUT_FORMAT_MPEG_4);
        if (!muxer) {
            cleanupMedia();
            state.store(RecorderState::Idle);
            setMessage("Unable to create the Android MP4 muxer.");
            return false;
        }

        {
            std::lock_guard lock(mutex);
            frames.clear();
            stopWorker = false;
            encoderFailed = false;
        }
        frameIndex = 0;
        nextCapture = std::chrono::steady_clock::now();
        state.store(RecorderState::Recording);
        worker = std::thread([this] { workerLoop(); });

        setMessage(fmt::format(
            "Recording {}x{} at {} FPS. Audio is not captured.",
            outputWidth,
            outputHeight,
            framesPerSecond
        ));
        return true;
    }

    void capture(int x, int y, int width, int height) {
        if (state.load() == RecorderState::Starting && !begin(width, height)) {
            return;
        }
        if (state.load() != RecorderState::Recording) return;

        auto const now = std::chrono::steady_clock::now();
        if (now < nextCapture) return;

        auto const interval = std::chrono::microseconds(
            1'000'000 / std::max(framesPerSecond, 1)
        );
        nextCapture += interval;
        if (nextCapture + interval < now) nextCapture = now + interval;

        {
            std::lock_guard lock(mutex);
            if (frames.size() >= kMaximumQueuedFrames || stopWorker) return;
        }

        Frame frame;
        frame.width = width;
        frame.height = height;
        frame.presentationTimeUs =
            frameIndex++ * 1'000'000LL / std::max(framesPerSecond, 1);
        frame.rgba.resize(
            static_cast<std::size_t>(width) *
            static_cast<std::size_t>(height) * 4u
        );

        GLint previousPackAlignment = 4;
        glGetIntegerv(GL_PACK_ALIGNMENT, &previousPackAlignment);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        while (glGetError() != GL_NO_ERROR) {}
        glReadPixels(
            x,
            y,
            width,
            height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            frame.rgba.data()
        );
        auto const glError = glGetError();
        glPixelStorei(GL_PACK_ALIGNMENT, previousPackAlignment);

        if (glError != GL_NO_ERROR) {
            setMessage(fmt::format(
                "Frame capture failed with OpenGL error 0x{:X}.",
                glError
            ));
            return;
        }

        {
            std::lock_guard lock(mutex);
            if (frames.size() >= kMaximumQueuedFrames || stopWorker) return;
            frames.push_back(std::move(frame));
        }
        condition.notify_one();
    }

    void workerLoop() {
        while (true) {
            Frame frame;
            {
                std::unique_lock lock(mutex);
                condition.wait(lock, [this] {
                    return stopWorker || !frames.empty();
                });

                if (frames.empty() && stopWorker) break;
                frame = std::move(frames.front());
                frames.pop_front();
            }

            if (!encodeFrame(frame)) {
                std::lock_guard lock(mutex);
                encoderFailed = true;
                frames.clear();
                stopWorker = true;
            }
        }

        if (!encoderFailed) {
            queueEndOfStream();
            drainEncoder(true);
        }

        auto const failed = encoderFailed;
        cleanupMedia();

        if (failed) {
            std::error_code error;
            std::filesystem::remove(pendingPath, error);
            state.store(RecorderState::Idle);
            setMessage("The recording encoder failed; the incomplete file was deleted.");
        }
        else {
            state.store(RecorderState::Pending);
            setMessage("Recording stopped. Choose Save or Delete.");
        }
    }

    void stopRecording() {
        if (state.load() != RecorderState::Recording) return;

        {
            std::lock_guard lock(mutex);
            stopWorker = true;
        }
        condition.notify_all();

        if (worker.joinable() && worker.get_id() != std::this_thread::get_id()) {
            worker.join();
        }
    }

    bool encodeFrame(Frame const& frame) {
        auto yuv = convertToI420(frame);
        if (yuv.empty()) return false;

        for (int attempt = 0; attempt < 8; ++attempt) {
            drainEncoder(false);
            auto const inputIndex = AMediaCodec_dequeueInputBuffer(codec, 2'000);
            if (inputIndex < 0) continue;

            std::size_t capacity = 0;
            auto* input = AMediaCodec_getInputBuffer(
                codec,
                static_cast<std::size_t>(inputIndex),
                &capacity
            );
            if (!input || capacity < yuv.size()) return false;

            std::copy(yuv.begin(), yuv.end(), input);
            return AMediaCodec_queueInputBuffer(
                codec,
                static_cast<std::size_t>(inputIndex),
                0,
                yuv.size(),
                frame.presentationTimeUs,
                0
            ) == AMEDIA_OK;
        }

        return false;
    }

    std::vector<std::uint8_t> convertToI420(Frame const& frame) const {
        auto const ySize = static_cast<std::size_t>(outputWidth) * outputHeight;
        auto const uvWidth = outputWidth / 2;
        auto const uvHeight = outputHeight / 2;
        auto const uvSize = static_cast<std::size_t>(uvWidth) * uvHeight;

        std::vector<std::uint8_t> output(ySize + uvSize * 2u);
        auto* yPlane = output.data();
        auto* uPlane = yPlane + ySize;
        auto* vPlane = uPlane + uvSize;

        auto sample = [&frame, this](int x, int y) {
            auto const sourceX = std::clamp(
                x * frame.width / std::max(outputWidth, 1),
                0,
                frame.width - 1
            );
            auto const scaledY = std::clamp(
                y * frame.height / std::max(outputHeight, 1),
                0,
                frame.height - 1
            );
            auto const sourceY = frame.height - 1 - scaledY;
            auto const offset = (
                static_cast<std::size_t>(sourceY) * frame.width + sourceX
            ) * 4u;
            return std::array<int, 3> {
                frame.rgba[offset],
                frame.rgba[offset + 1],
                frame.rgba[offset + 2]
            };
        };

        for (int y = 0; y < outputHeight; ++y) {
            for (int x = 0; x < outputWidth; ++x) {
                auto const rgb = sample(x, y);
                auto const luma = ((66 * rgb[0] + 129 * rgb[1] + 25 * rgb[2] + 128) >> 8) + 16;
                yPlane[static_cast<std::size_t>(y) * outputWidth + x] = clampByte(luma);
            }
        }

        for (int y = 0; y < outputHeight; y += 2) {
            for (int x = 0; x < outputWidth; x += 2) {
                int red = 0;
                int green = 0;
                int blue = 0;
                for (int oy = 0; oy < 2; ++oy) {
                    for (int ox = 0; ox < 2; ++ox) {
                        auto const rgb = sample(x + ox, y + oy);
                        red += rgb[0];
                        green += rgb[1];
                        blue += rgb[2];
                    }
                }
                red /= 4;
                green /= 4;
                blue /= 4;

                auto const chromaU = ((-38 * red - 74 * green + 112 * blue + 128) >> 8) + 128;
                auto const chromaV = ((112 * red - 94 * green - 18 * blue + 128) >> 8) + 128;
                auto const uvIndex = static_cast<std::size_t>(y / 2) * uvWidth + x / 2;
                uPlane[uvIndex] = clampByte(chromaU);
                vPlane[uvIndex] = clampByte(chromaV);
            }
        }

        return output;
    }

    void queueEndOfStream() {
        for (int attempt = 0; attempt < 40; ++attempt) {
            drainEncoder(false);
            auto const inputIndex = AMediaCodec_dequeueInputBuffer(codec, 5'000);
            if (inputIndex < 0) continue;

            AMediaCodec_queueInputBuffer(
                codec,
                static_cast<std::size_t>(inputIndex),
                0,
                0,
                frameIndex * 1'000'000LL / std::max(framesPerSecond, 1),
                AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM
            );
            return;
        }
        encoderFailed = true;
    }

    void drainEncoder(bool waitForEnd) {
        if (!codec) return;

        int idleCount = 0;
        while (true) {
            AMediaCodecBufferInfo info {};
            auto const outputIndex = AMediaCodec_dequeueOutputBuffer(
                codec,
                &info,
                waitForEnd ? 10'000 : 0
            );

            if (outputIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
                if (!waitForEnd || ++idleCount > 100) return;
                continue;
            }
            if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
                if (muxerStarted) {
                    encoderFailed = true;
                    return;
                }
                auto* format = AMediaCodec_getOutputFormat(codec);
                trackIndex = AMediaMuxer_addTrack(muxer, format);
                AMediaFormat_delete(format);
                if (trackIndex < 0 || AMediaMuxer_start(muxer) != AMEDIA_OK) {
                    encoderFailed = true;
                    return;
                }
                muxerStarted = true;
                continue;
            }
            if (outputIndex < 0) continue;

            std::size_t capacity = 0;
            auto* data = AMediaCodec_getOutputBuffer(
                codec,
                static_cast<std::size_t>(outputIndex),
                &capacity
            );

            if (
                data &&
                info.size > 0 &&
                muxerStarted &&
                (info.flags & AMEDIACODEC_BUFFER_FLAG_CODEC_CONFIG) == 0
            ) {
                if (
                    AMediaMuxer_writeSampleData(
                        muxer,
                        static_cast<std::size_t>(trackIndex),
                        data,
                        &info
                    ) != AMEDIA_OK
                ) {
                    encoderFailed = true;
                }
            }

            auto const endOfStream =
                (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) != 0;
            AMediaCodec_releaseOutputBuffer(
                codec,
                static_cast<std::size_t>(outputIndex),
                false
            );

            if (endOfStream || encoderFailed) return;
        }
    }

    void cleanupMedia() {
        if (codec) {
            AMediaCodec_stop(codec);
            AMediaCodec_delete(codec);
            codec = nullptr;
        }
        if (muxer) {
            if (muxerStarted) AMediaMuxer_stop(muxer);
            AMediaMuxer_delete(muxer);
            muxer = nullptr;
        }
        muxerStarted = false;
        trackIndex = -1;

        if (fileDescriptor >= 0) {
            close(fileDescriptor);
            fileDescriptor = -1;
        }
    }

    bool savePending() {
        if (state.load() != RecorderState::Pending) {
            setMessage("There is no stopped recording to save.");
            return false;
        }

        auto destination = directory / timestampName();
        std::error_code error;
        std::filesystem::rename(pendingPath, destination, error);
        if (error) {
            setMessage(fmt::format("Unable to save recording: {}", error.message()));
            return false;
        }

        savedPath = std::move(destination);
        state.store(RecorderState::Idle);
        setMessage(fmt::format("Saved to {}", pathText(savedPath)));
        return true;
    }

    bool deletePending() {
        if (state.load() == RecorderState::Recording) {
            setMessage("Stop the recording before deleting it.");
            return false;
        }
        if (state.load() != RecorderState::Pending) {
            setMessage("There is no stopped recording to delete.");
            return false;
        }

        std::error_code error;
        auto const removed = std::filesystem::remove(pendingPath, error);
        if (error || !removed) {
            setMessage("Unable to delete the temporary recording.");
            return false;
        }

        state.store(RecorderState::Idle);
        setMessage("The temporary recording was deleted.");
        return true;
    }
};

ScreenRecorder& ScreenRecorder::get() {
    static ScreenRecorder instance;
    return instance;
}

ScreenRecorder::ScreenRecorder()
  : m_impl(std::make_unique<Impl>()) {}

ScreenRecorder::~ScreenRecorder() = default;

bool ScreenRecorder::requestStart() {
    return m_impl->requestStart();
}

bool ScreenRecorder::requestStop() {
    return m_impl->requestStop();
}

bool ScreenRecorder::savePending() {
    return m_impl->savePending();
}

bool ScreenRecorder::deletePending() {
    return m_impl->deletePending();
}

RecorderState ScreenRecorder::state() const {
    return m_impl->state.load();
}

bool ScreenRecorder::wantsFrames() const {
    auto const current = state();
    return current == RecorderState::Starting || current == RecorderState::Recording;
}

std::string ScreenRecorder::statusMessage() const {
    return m_impl->getMessage();
}

std::filesystem::path ScreenRecorder::outputDirectory() const {
    return m_impl->directory;
}

std::filesystem::path ScreenRecorder::lastSavedPath() const {
    return m_impl->savedPath;
}

void ScreenRecorder::captureFrame(int x, int y, int width, int height) {
    m_impl->capture(x, y, width, height);
}

} // namespace zaidfx
