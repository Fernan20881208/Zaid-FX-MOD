#include "ShaderProgram.hpp"

#include <Geode/Geode.hpp>

#include <fstream>
#include <sstream>

using namespace geode::prelude;

namespace zaidfx {

ShaderProgram::~ShaderProgram() {
    reset();
}

std::string ShaderProgram::readTextFile(std::filesystem::path const& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        log::error("Unable to open shader file: {}", path.string());
        return {};
    }

    std::ostringstream stream;
    stream << input.rdbuf();
    return stream.str();
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
        log::error("Vertex shader compilation failed: {}", program->vertexShaderLog());
        log::error("Fragment shader compilation failed: {}", program->fragmentShaderLog());
        program->release();
        return false;
    }

    program->addAttribute(kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
    program->addAttribute(kCCAttributeNameColor, cocos2d::kCCVertexAttrib_Color);
    program->addAttribute(kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);

    if (!program->link()) {
        log::error("Shader link failed: {}", program->programLog());
        program->release();
        return false;
    }

    program->updateUniforms();
    m_program = program;
    m_uniformLocations.clear();
    return true;
}

void ShaderProgram::use() const {
    if (!m_program) {
        return;
    }

    m_program->use();
    m_program->setUniformsForBuiltins();
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

bool ShaderProgram::isLoaded() const {
    return m_program != nullptr;
}

int ShaderProgram::uniformLocation(char const* uniformName) const {
    auto const found = m_uniformLocations.find(uniformName);
    if (found != m_uniformLocations.end()) {
        return found->second;
    }

    auto const location = m_program->getUniformLocationForName(uniformName);
    m_uniformLocations.emplace(uniformName, location);
    return location;
}

} // namespace zaidfx
