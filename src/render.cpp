#include "render.h"

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace render
{
    std::tuple<GLuint, GLuint, GLuint, GLuint> initializeVertexBuffers() {
        GLuint vao, vbo, textVao, textVbo;

        float positions[12] = {
            1, -1, // bottom right
            1, 1, // top right
            -1, 1, // top left
            -1, 1, // top left
            -1, -1, // bottom left
            1, -1 // bottom right
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        glBindVertexArray(0);

        glGenVertexArrays(1, &textVao);
        glGenBuffers(1, &textVbo);
        glBindVertexArray(textVao);
        glBindBuffer(GL_ARRAY_BUFFER, textVbo);
        glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return std::tuple(vao, vbo, textVao, textVbo);
    }

    void setProjectionMatrix(GLuint ubo, int width, int height) {
        float ratio = (float)width / (float)height;
        glm::mat4 projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    GLuint initializeUniformBuffer(int windowWidth, int windowHeight) {
        GLuint ubo;
        glGenBuffers(1, &ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, sizeof(glm::mat4));

        setProjectionMatrix(ubo, windowWidth, windowHeight);

        return ubo;
    }

    // TODO: make this constexpr to remove the mutable reference without unnecessary copying
    void addShader(ShaderData& shaderData, const std::string& name, const std::string& path, const std::vector<std::string>& uniforms) {
        shader::ShaderSources source = shader::parse(path);
        GLuint program = shader::create(source);
        GLuint ubi = glGetUniformBlockIndex(program, "Matrices");
        glUniformBlockBinding(program, ubi, 0);

        shaderData[name] = std::pair(program, std::unordered_map<std::string, GLuint>{});

        for (const std::string& uniform : uniforms) {
            GLuint uniformLocation = glGetUniformLocation(program, uniform.c_str());
            shaderData[name].second[uniform] = uniformLocation;
        }
    }

    // TODO: abstract this to support alternative fonts
    std::unordered_map<char, FontCharacter> initializeCharacters(FT_Face face, int size, int windowHeight) {
        std::unordered_map<char, FontCharacter> map;

        FT_Set_Pixel_Sizes(face, 0, int((float)size * windowHeight / 1080.0f)); // scales font size based on resolution

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++) {
            
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                printf("Failed to load glyph\n");
                continue;
            }
            
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            FontCharacter fontCharacter = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            map[c] = fontCharacter;
        }

        return map;
    }

    RenderData init(int windowWidth, int windowHeight) {
        RenderData renderData;

        if (glewInit() != GLEW_OK) {
            throw std::runtime_error("GLEW failed to initialize");
        }

        std::tie(renderData.vao, renderData.vbo, renderData.textVao, renderData.textVbo) = initializeVertexBuffers();
        renderData.ubo = initializeUniformBuffer(windowWidth, windowHeight);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0, 0.0, 0.0, 1.0);

        addShader(renderData.shaderData, "player", "res/shaders/Player.shader", { "model", "size", "color", "stamina", "paddleVec", "paddleSize" });
        addShader(renderData.shaderData, "ball", "res/shaders/Ball.shader", { "model", "size" });
        addShader(renderData.shaderData, "text", "res/shaders/Text.shader", { "textColor", "projection" });
        addShader(renderData.shaderData, "slider", "res/shaders/Slider.shader", { "model", "size", "circlePos" });
        addShader(renderData.shaderData, "rectangle", "res/shaders/Rectangle.shader", { "model", "size", "color" });
        addShader(renderData.shaderData, "obround", "res/shaders/Obround.shader", { "model", "size", "color" });

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            throw std::runtime_error("Could not initialize FreeType library");
        }

        FT_Face face;
        if (FT_New_Face(ft, "res/fonts/Bitter-Regular.otf", 0, &face)) {
            throw std::runtime_error("Failed to load font");
        }

        // TODO: consider regenerating on window resize
        renderData.fontData["small"] = initializeCharacters(face, 24, windowHeight);
        renderData.fontData["large"] = initializeCharacters(face, 45, windowHeight);

        renderData.windowWidth = windowWidth;
        renderData.windowHeight = windowHeight;

        return renderData;
    }

    void resize(RenderData& renderData, int width, int height) {
        setProjectionMatrix(renderData.ubo, width, height);
        glViewport(0, 0, width, height);
        renderData.windowWidth = width;
        renderData.windowHeight = height;
    }

    constexpr glm::vec2 coordsToPixels(glm::vec2 in, glm::vec2 windowSize) {
        return glm::vec2((in.x * windowSize.y + windowSize.x) / 2, (in.y + 1) * windowSize.y / 2);
    }

    constexpr glm::vec2 pixelsToCoords(glm::vec2 in, glm::vec2 windowSize) {
        return glm::vec2((2 * in.x - windowSize.x) / windowSize.y, (-2 * in.y) / windowSize.y + 1);
    }

    // TODO: make RenderData pointer the glfwWindowUserPointer and save window width and height in the struct, modifying it on resize
    void renderText(const RenderData& renderData, const std::string& text, const std::string& font, const glm::vec2& position, const glm::vec3& color, float scale) {
        const auto& shaderData = renderData.shaderData.at("text");
        glUseProgram(shaderData.first);
        glUniform3f(shaderData.second.at("color"), color.r, color.g, color.b);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(renderData.textVao);

        const auto& fontData = renderData.fontData.at(font);
        float total_width = 0.0f;
        for (char c : text) {
            const FontCharacter& ch = fontData.at(c);
            total_width += (ch.advance >> 6) * scale;
        }
        const FontCharacter& end = fontData.at(text.back());
        total_width -= (int(end.advance >> 6) - (end.bearing.x + end.size.x)) * scale;
        float height = fontData.at('o').size.y;

        glm::vec2 pos = coordsToPixels(position, glm::vec2(renderData.windowWidth, renderData.windowHeight));

        for (char c : text) {
            const FontCharacter& ch = fontData.at(c);

            float xpos = pos.x + ch.bearing.x * scale;
            float ypos = pos.y - (ch.size.y - ch.bearing.y) * scale;

            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            xpos -= total_width / 2;
            ypos -= height / 2;

            float vertices[6][4] = {
                { xpos, ypos + h, 0.0f, 0.0f },
                { xpos, ypos, 0.0f, 1.0f },
                { xpos + w, ypos, 1.0f, 1.0f },
                { xpos, ypos + h, 0.0f, 0.0f },
                { xpos + w, ypos, 1.0f, 1.0f },
                { xpos + w, ypos + h, 1.0f, 0.0f }
            };
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, renderData.textVbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            pos.x += (ch.advance >> 6) * scale;
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    constexpr glm::mat4 vec2ToModelMatrix(glm::vec2 v) {
        return glm::translate(glm::mat4(1.0f), glm::vec3(v, 0.0f));
    }

    void renderUi(const RenderData& renderData, const ui::UiData& uiData, game::State gameState) {
        for (const auto& element : uiData.at(gameState)) {
            if (std::holds_alternative<ui::Button>(element)) {
                const auto& button = std::get<ui::Button>(element);
                renderText(renderData, button.text, "large", button.pos, glm::vec3(1.0f, 1.0f, 1.0f), button.textScale);
            } else if (std::holds_alternative<ui::Slider>(element)) {
                const auto& slider = std::get<ui::Slider>(element);

                const auto& shaderData = renderData.shaderData.at("slider");
                glUseProgram(shaderData.first);

                glUniformMatrix4fv(shaderData.second.at("model"), 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(slider.pos)));
                glUniform2fv(shaderData.second.at("size"), 1, glm::value_ptr(slider.size));
                glUniform1f(shaderData.second.at("circlePos"), slider.value);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

                std::string sliderStr = std::to_string(slider.value);
                sliderStr = sliderStr.substr(0, sliderStr.size() - 3);
                renderText(renderData, sliderStr, "small", glm::vec2(1.1f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
            }
        }
    }

    void renderGame(const RenderData& renderData, const game::GameData& gameData) {
        const auto& obroundShaderData = renderData.shaderData.at("obround");
        glUseProgram(obroundShaderData.first);

        glUniformMatrix4fv(obroundShaderData.second.at("model"), 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(glm::vec2(-1.4f, 0.0f))));
        glUniform2fv(obroundShaderData.second.at("size"), 1, glm::value_ptr(glm::vec2(0.02f, 0.2f)));
        glUniform3fv(obroundShaderData.second.at("color"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

        glUniformMatrix4fv(obroundShaderData.second.at("model"), 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(glm::vec2(1.4f, 0.0f))));
        glUniform2fv(obroundShaderData.second.at("size"), 1, glm::value_ptr(glm::vec2(0.02f, 0.2f)));
        glUniform3fv(obroundShaderData.second.at("color"), 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

        const auto& playerShaderData = renderData.shaderData.at("player");
        glUseProgram(playerShaderData.first);

        for (int i = 0; i < gameData.playerData.size(); ++i) {
            glUniform1f(playerShaderData.second.at("size"), 0.1f);
            glUniform3fv(playerShaderData.second.at("color"), 1, glm::value_ptr(glm::vec3(float(i % 2), 0.0f, fabs(float(i % 2) - 1.0f))));
            glUniform1f(playerShaderData.second.at("stamina"), (gameData.playerData[i].stamina - 0.5f) * 2 * 3.14159265f);
            glUniform2fv(playerShaderData.second.at("paddleVec"), 1, glm::value_ptr(gameData.playerData[i].paddlePos));
            glUniform1f(playerShaderData.second.at("paddleSize"), 0.05f);
            glUniformMatrix4fv(playerShaderData.second.at("model"), 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(gameData.playerData[i].pos)));
            glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
        }

        const auto& ballShaderData = renderData.shaderData.at("ball");
        glUseProgram(ballShaderData.first);

        glUniform1f(ballShaderData.second.at("size"), 0.02f);
        glUniformMatrix4fv(ballShaderData.second.at("model"), 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(gameData.ballData.pos)));
        glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

        renderText(renderData, std::to_string(gameData.goals[0]) + " - " + std::to_string(gameData.goals[1]), "large", glm::vec2(0.0f, 0.9f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f);
    }

    void render(const RenderData& renderData, const game::GameData& gameData, const ui::UiData& uiData) {
        glClear(GL_COLOR_BUFFER_BIT);
        renderGame(renderData, gameData);
        renderUi(renderData, uiData, gameData.state);
    }
}