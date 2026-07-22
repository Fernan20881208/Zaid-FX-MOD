#pragma once

#include <Geode/cocos/shaders/CCGLProgram.h>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace zaidfx {

class ShaderProgram final {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    ShaderProgram(ShaderProgram const&) = delete;
    ShaderProgram& operator=(ShaderProgram const&) = delete;
    ShaderProgram(ShaderProgram&&) = delete;
    ShaderProgram& operator=(ShaderProgram&&) = delete;

    bool loadFromFiles(
        std::filesystem::path const& vertexPath,
        std::filesystem::path const& fragmentPath
    );
    bool loadFromSource(std::string const& vertexSource, std::string const& fragmentSource);

    void use() const;
    bool setFloat(char const* uniformName, float value) const;
    bool setVec2(char const* uniformName, float x, float y) const;
    void reset();

    [[nodiscard]] cocos2d::CCGLProgram* get() const;
    [[nodiscard]] bool isLoaded() const;

private:
    static std::string readTextFile(std::filesystem::path const& path);
    int uniformLocation(char const* uniformName) const;

    cocos2d::CCGLProgram* m_program = nullptr;
    mutable std::unordered_map<std::string, int> m_uniformLocations;
};

} // namespace zaidfx
