#pragma once

#include <GL/glew.h>

#include <string_view>
#include <unordered_map>

namespace shader
{
    using ShaderSources = std::unordered_map<GLenum, std::string>;

    ShaderSources parse(const std::string& filepath);
    GLuint compile(unsigned int type, const std::string_view& source);
    GLuint create(const ShaderSources& shader);
}