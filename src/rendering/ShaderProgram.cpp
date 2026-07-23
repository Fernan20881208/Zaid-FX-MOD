#include "ShaderProgram.hpp"

#include <Geode/Geode.hpp>
#include <Geode/cocos/platform/CCGL.h>
#include <Geode/utils/file.hpp>
#include <Geode/utils/string.hpp>

#include <utility>

using namespace geode::prelude;

namespace zaidfx {

ShaderProgram::~ShaderProgram() {
    reset();
}

std::string ShaderProgram::readTextFile(std::filesystem::path const& path) {
    auto result = geode::utils::file::readString(path);
    if (!result.isOk()) {
        log::error(
            "[ZaidFX] unable to read shader {}: {}",
            geode::utils::string::pathToString(path),
            result.unwrapErr()
        );
        return {};
    }

    return std::move(result).unwrap();
}

bool ShaderProgram::loadFromFiles(
    std::filesystem::path const& vertexPath,
    std::filesystem::path const& fragmentPath
) {
    auto vertexSource = readTextFile(vertexPath);
    auto fragmentSource = readTextFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }

    return loadFromSource(vertexSource, fragmentSource);
}

bool ShaderProgram::loadFromSource(
    std::string const& vertexSource,
    std::string const& fragmentSource
) {
    reset();

    auto* program = new cocos2d::CCGLProgram();
    if (!program->initWithVertexShaderByteArray(vertexSource.c_str(), fragmentSource.c_str())) {
        log::error("[ZaidFX] vertex shader error: {}", program->vertexShaderLog());
        log::error("[ZaidFX] fragment shader error: {}", program->fragmentShaderLog());
        program->release();
        return false;
    }

    program->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
    program->addAttribute(kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);

    if (!program->link()) {
        log::error("[ZaidFX] shader link error: {}", program->programLog());
        program->release();
        return false;
    }

    program->updateUniforms();
    m_program = program;
    m_uniformLocations.clear();

    if (!isValid()) {
        log::error("[ZaidFX] linked shader program is invalid");
        reset();
        return false;
    }

    return true;
}

void ShaderProgram::use() const {
    if (m_program) {
        m_program->use();
    }
}

bool ShaderProgram::setInt(char const* uniformName, int value) const {
    if (!m_program || !uniformName) {
        return false;
    }

    auto const location = uniformLocation(uniformName);
    if (location < 0) {
        return false;
    }

    m_program->setUniformLocationWith1i(location, value);
    return true;
}

bool ShaderProgram::setFloat(char const* uniformName, float value) const {
    if (!m_program || !uniformName) {
        return false;
    }

    auto const location = uniformLocation(uniformName);
    if (location < 0) {
        return false;
    }

    m_program->setUniformLocationWith1f(location, value);
    return true;
}

bool ShaderProgram::setVec2(char const* uniformName, float x, float y) const {
    if (!m_program || !uniformName) {
        return false;
    }

    auto const location = uniformLocation(uniformName);
    if (location < 0) {
        return false;
    }

    m_program->setUniformLocationWith2f(location, x, y);
    return true;
}

void ShaderProgram::reset() {
    m_uniformLocations.clear();

    if (m_program) {
        m_program->release();
        m_program = nullptr;
    }
}

cocos2d::CCGLProgram* ShaderProgram::get() const {
    return m_program;
}

unsigned int ShaderProgram::programID() const {
    return m_program ? m_program->getProgram() : 0u;
}

bool ShaderProgram::isLoaded() const {
    return m_program != nullptr;
}

bool ShaderProgram::isValid() const {
    auto const id = programID();
    return id != 0u && glIsProgram(id) == GL_TRUE;
}

int ShaderProgram::uniformLocation(char const* uniformName) const {
    auto const found = m_uniformLocations.find(uniformName);
    if (found != m_uniformLocations.end()) {
        return found->second;
    }

    auto const location = m_program->getUniformLocationForName(uniformName);
    m_uniformLocations.emplace(uniformName, location);

    if (location < 0) {
        log::error("[ZaidFX] shader uniform not found: {}", uniformName);
    }

    return location;
}

} // namespace zaidfx
