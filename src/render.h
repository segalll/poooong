#pragma once

#include "game.h"
#include "shader.h"
#include "ui.h"

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include <string>
#include <unordered_map>

namespace render
{
    using ShaderData = std::unordered_map<
        std::string, // shader name
        std::pair<
            GLuint, // program
            std::unordered_map<
                std::string, // uniform name
                GLuint // uniform location
            >
        >
    >;

    struct FontCharacter {
        GLuint textureID; // ID handle of the glyph texture
        glm::ivec2 size; // size of glyph
        glm::ivec2 bearing; // offset from baseline to left/top of glyph
        unsigned int advance; // offset to advance to next glyph
    };

    using FontData = std::unordered_map<
        std::string, // font name
        std::unordered_map<
            char,
            FontCharacter
        >
    >;

    struct RenderData {
        GLuint vao, vbo;
        GLuint textVao, textVbo;
        GLuint ubo;
        ShaderData shaderData;
        FontData fontData;
    };

    RenderData init(GLFWwindow* window);
    void renderUi(const RenderData& renderData, const ui::UiData& uiData, game::State gameState);
    void renderGame(const RenderData& renderData, const game::GameData& gameData);
}