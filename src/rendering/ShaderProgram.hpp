#pragma once

#include <Geode/Geode.hpp>

#include <filesystem>
#include <string>

namespace zaidfx {

class ShaderProgram final {
public:
    ShaderProgram() = default;
    ~ShaderProgram();

    ShaderProgram(ShaderProgram const&) = delete;
    ShaderProgram& operator=(ShaderProgram const&) = delete;

    bool loadFromFiles(
        std::filesystem::path const& vertexPath,
        std::filesystem::path const& fragmentPath
    );
    bool loadFromSource(std::string const& vertexSource, std::string const& fragmentSource);

    void use() const;
    void setFloat(char const* uniformName, float value) const;
    void reset();

    [[nodiscard]] cocos2d::CCGLProgram* get() const;
    [[nodiscard]] bool isLoaded() const;

private:
    static std::string readTextFile(std::filesystem::path const& path);

    cocos2d::CCGLProgram* m_program = nullptr;
};

} // namespace zaidfx
