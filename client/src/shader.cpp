#include "shader.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace shader
{
    ShaderSources parse(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file: " + filepath);
        }

        std::string line;
        std::stringstream ss[2];
        ShaderSources sources;

        while (std::getline(file, line)) {
            if (line.find("#shader") != std::string::npos) {
                if (line.find("vertex") != std::string::npos) {
                    sources[GL_VERTEX_SHADER] = ss[0].str();
                    ss[0].str("");
                } else if (line.find("fragment") != std::string::npos) {
                    sources[GL_FRAGMENT_SHADER] = ss[1].str();
                    ss[1].str("");
                }
            } else {
                ss[0] << line << '\n';
                ss[1] << line << '\n';
            }
        }

        sources[GL_VERTEX_SHADER] = ss[0].str();
        sources[GL_FRAGMENT_SHADER] = ss[1].str();

        return sources;
    }

    GLuint compile(GLenum type, const std::string_view& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.data();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint length;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            char* message = new char[length];
            glGetShaderInfoLog(shader, length, &length, message);
            throw std::runtime_error("Failed to compile shader!\n" + std::string(message));
        }

        return shader;
    }

    GLuint create(const ShaderSources& sources) {
        GLuint program = glCreateProgram();

        std::vector<GLuint> shaders;
        for (const auto& [type, source] : sources) {
            shaders.push_back(compile(type, source));
        }

        for (GLuint shader : shaders) {
            glAttachShader(program, shader);
        }

        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            char* message = new char[length];
            glGetProgramInfoLog(program, length, &length, message);
            throw std::runtime_error("Failed to link shader program!\n" + std::string(message));
        }

        for (GLuint shader : shaders) {
            glDeleteShader(shader);
        }

        return program;
    }
}