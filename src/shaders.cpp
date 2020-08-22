#include "shaders.h"

namespace shader
{
    source parse(const std::string& filepath)
    {
        std::ifstream stream(filepath);

        enum class ShaderType {
            NONE = -1, VERTEX = 0, FRAGMENT = 1
        };

        std::string line;
        std::stringstream ss[2];
        ShaderType type = ShaderType::NONE;
        while (getline(stream, line))
        {
            if (line.find("#shader") != std::string::npos)
            {
                if (line.find("vertex") != std::string::npos)
                    type = ShaderType::VERTEX;
                else if (line.find("fragment") != std::string::npos)
                    type = ShaderType::FRAGMENT;
            }
            else
            {
                ss[(int)type] << line << '\n';
            }
        }
        return { ss[0].str(), ss[1].str() };
    }

    unsigned int compile(unsigned int type, const std::string& source)
    {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE)
        {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = new char[length];
            glGetShaderInfoLog(id, length, &length, message);
            printf("Failed to compile %s shader!\n%s\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment", message);
            glDeleteShader(id);
            delete[] message;
            return 0;
        }

        return id;
    }

    unsigned int create(const source& shader)
    {
        unsigned int program = glCreateProgram();
        unsigned int vs = compile(GL_VERTEX_SHADER, shader.VertexSource);
        unsigned int fs = compile(GL_FRAGMENT_SHADER, shader.FragmentSource);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }
}