#include <Windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <math.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

#include "constants.h"
#include "shaders.h"
#include "net.h"

#include <ft2build.h>
#include FT_FREETYPE_H

float prevWidth;
float prevHeight;
State gameState = State::MENU;
unsigned int vao, vbo, textVao, textVbo;
std::map<char, Character> largeCharacters;
std::map<char, Character> smallCharacters;

bool pointAndRectCollide(glm::vec2 point, glm::vec2 rectPos, glm::vec2 rectDim) {
    return point.x > rectPos.x - (rectDim.x / 2) && point.x < rectPos.x + (rectDim.x / 2) && point.y > rectPos.y - (rectDim.y / 2) && point.y < rectPos.y + (rectDim.y / 2);
}

Input processInput(GLFWwindow* window) {
    Input ret;
    ret.w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    ret.a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
    ret.s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    ret.d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
    ret.click = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    ret.sprint = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    return ret;
}

void initializeVertexBuffer() {
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
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void setProjectionMatrix(unsigned int ubo, int width, int height) {
    float ratio = (float)width / (float)height;
    glm::mat4 projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

unsigned int initializeUniformBuffer(GLFWwindow* window) {
    unsigned int ubo;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo, 0, sizeof(glm::mat4));

    int width;
    int height;

    glfwGetWindowSize(window, &width, &height);

    setProjectionMatrix(ubo, width, height);

    return ubo;
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) return;
    prevWidth = width;
    prevHeight = height;
    unsigned int ubo = *(unsigned int*)glfwGetWindowUserPointer(window);

    setProjectionMatrix(ubo, width, height);

    glViewport(0, 0, width, height);
}

void handleButton(Button& b, bool clicked, float cursorX, float cursorY, double delta) {
    if (pointAndRectCollide(glm::vec2(cursorX, cursorY), b.pos, b.size)) {
        b.textScale = fmin(b.textScale + (delta * 2), 1.2f);
        if (clicked) gameState = b.onClick;
    } else {
        b.textScale = fmax(b.textScale - (delta * 2), 1.0f);
    }
}

void handleSlider(Slider& s, float cursorX, float cursorY, bool cursorDown) {
    if (!cursorDown) {
        s.moving = false;
    }

    if (s.moving) {
        float translatedCursor = fmax(fmin(cursorX, s.pos.x + s.size.x - s.size.y), s.pos.x - s.size.x + s.size.y);
        s.value = roundf((translatedCursor - (s.pos.x - s.size.x + s.size.y)) / ((s.size.x * 2) - (s.size.y * 2)) * 500) / 500;
    } else {
        if (cursorDown && cursorY < s.pos.y + s.size.y && cursorY > s.pos.y - s.size.y && cursorX < s.pos.x + s.size.x && cursorX > s.pos.x - s.size.x) {
            s.moving = true;
        }
    }
}

std::map<char, Character> initializeCharacters(FT_Face face, int size) {
    std::map<char, Character> map;

    FT_Set_Pixel_Sizes(face, 0, size * prevHeight / 1080); // scales font size based on resolution

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            printf("Failed to load Glyph\n");
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
        
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        map.insert(std::pair<char, Character>(c, character));
    }

    return map;
}

glm::vec2 coordsToPixels(glm::vec2 in) {
    in.x += prevWidth / prevHeight;
    in.x *= prevHeight / 2;
    in.y += 1;
    in.y *= prevHeight / 2;
    return in;
}

glm::mat4 vec2ToModelMatrix(glm::vec2 v) {
    return glm::translate(glm::mat4(1.0f), glm::vec3(v, 0.0f));
}

void renderText(unsigned int shader, std::string text, glm::vec2 pos, float scale, glm::vec3 color, const std::map<char, Character>& characters) {
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glm::mat4 projection = glm::ortho(0.0f, prevWidth, 0.0f, prevHeight);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVao);

    std::string::const_iterator i;
    float total_width = 0.0f;
    for (i = text.begin(); i != text.end(); i++) {
        Character ch = characters.at(*i);
        total_width += (ch.Advance >> 6) * scale;
    }
    Character end = characters.at(text.back());
    total_width -= (int(end.Advance >> 6) - (end.Bearing.x + end.Size.x)) * scale;
    float height = characters.at('o').Size.y;

    pos = coordsToPixels(pos);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = characters.at(*c);

        float xpos = pos.x + ch.Bearing.x * scale;
        float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        xpos -= total_width / 2;
        ypos -= height / 2;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        pos.x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

glm::vec2 lerp(glm::vec2 a, glm::vec2 b, float t) {
    return a + (t * (b - a));
}

void drawLoop(GLFWwindow* window) {
    glm::ivec2 goals = glm::ivec2(0, 0);

    double cursorX;
    double cursorY;

    initializeVertexBuffer();

    unsigned int ubo = initializeUniformBuffer(window);

    glfwSetWindowUserPointer(window, &ubo);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    shader::source playerShaderSource = shader::parse("res/shaders/Player.shader");
    shader::source ballShaderSource = shader::parse("res/shaders/Ball.shader");
    shader::source textShaderSource = shader::parse("res/shaders/Text.shader");
    shader::source sliderShaderSource = shader::parse("res/shaders/Slider.shader");
    shader::source rectangleShaderSource = shader::parse("res/shaders/Rectangle.shader");
    shader::source obroundShaderSource = shader::parse("res/shaders/Obround.shader");

    unsigned int playerShaderProgram = shader::create(playerShaderSource);
    unsigned int ballShaderProgram = shader::create(ballShaderSource);
    unsigned int textShaderProgram = shader::create(textShaderSource);
    unsigned int sliderShaderProgram = shader::create(sliderShaderSource);
    unsigned int rectangleShaderProgram = shader::create(rectangleShaderSource);
    unsigned int obroundShaderProgram = shader::create(obroundShaderSource);

    unsigned int playerUBI = glGetUniformBlockIndex(playerShaderProgram, "Matrices");
    unsigned int ballUBI = glGetUniformBlockIndex(ballShaderProgram, "Matrices");
    unsigned int sliderUBI = glGetUniformBlockIndex(sliderShaderProgram, "Matrices");
    unsigned int rectangleUBI = glGetUniformBlockIndex(rectangleShaderProgram, "Matrices");
    unsigned int obroundUBI = glGetUniformBlockIndex(obroundShaderProgram, "Matrices");

    glUniformBlockBinding(playerShaderProgram, playerUBI, 0);
    glUniformBlockBinding(ballShaderProgram, ballUBI, 0);
    glUniformBlockBinding(sliderShaderProgram, sliderUBI, 0);
    glUniformBlockBinding(rectangleShaderProgram, rectangleUBI, 0);
    glUniformBlockBinding(obroundShaderProgram, obroundUBI, 0);

    unsigned int playerModel = glGetUniformLocation(playerShaderProgram, "model");
    unsigned int playerSize = glGetUniformLocation(playerShaderProgram, "size");
    unsigned int playerColor = glGetUniformLocation(playerShaderProgram, "color");
    unsigned int playerStamina = glGetUniformLocation(playerShaderProgram, "stamina");
    unsigned int paddleVec = glGetUniformLocation(playerShaderProgram, "paddleVec");
    unsigned int paddleSize = glGetUniformLocation(playerShaderProgram, "paddleSize");

    unsigned int ballModel = glGetUniformLocation(ballShaderProgram, "model");
    unsigned int ballSize = glGetUniformLocation(ballShaderProgram, "size");

    unsigned int textColor = glGetUniformLocation(textShaderProgram, "textColor");
    unsigned int textProjection = glGetUniformLocation(textShaderProgram, "projection");

    unsigned int sliderModel = glGetUniformLocation(sliderShaderProgram, "model");
    unsigned int sliderSize = glGetUniformLocation(sliderShaderProgram, "size");
    unsigned int sliderCirclePos = glGetUniformLocation(sliderShaderProgram, "circlePos");

    unsigned int rectangleModel = glGetUniformLocation(rectangleShaderProgram, "model");
    unsigned int rectangleSize = glGetUniformLocation(rectangleShaderProgram, "size");
    unsigned int rectangleColor = glGetUniformLocation(rectangleShaderProgram, "color");

    unsigned int obroundModel = glGetUniformLocation(obroundShaderProgram, "model");
    unsigned int obroundSize = glGetUniformLocation(obroundShaderProgram, "size");
    unsigned int obroundColor = glGetUniformLocation(obroundShaderProgram, "color");

    Input prevInput;
    bool prevEsc = false;

    Button playButton(
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.5f, 0.15f),
        State::JOIN,
        "Play"
    );

    Button exitButton(
        glm::vec2(0.0f, -0.4f),
        glm::vec2(0.5f, 0.15f),
        State::EXIT,
        "Exit"
    );

    Button resumeButton(
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.5f, 0.15f),
        State::GAME,
        "Resume"
    );

    Button menuButton(
        glm::vec2(0.0f, -0.4f),
        glm::vec2(0.5f, 0.15f),
        State::LEAVE,
        "Menu"
    );

    Slider testSlider(
        glm::vec2(0.5f, 0.5f),
        glm::vec2(0.4f, 0.025f),
        0.5f
    );

    if (!net::init()) {
        printf("net::init failed\n");
    }
    net::Socket sock;
    if (!net::socketCreate(&sock)) {
        printf("socketCreate failed\n");
    }
    
    net::IPEndpoint serverEndpoint = net::ipEndpointCreate(127, 0, 0, 1, 9999);
    unsigned char buffer[57];

    PlayerState players[2];
    float serverTime = 0.0f;
    unsigned int numPlayers = 0;
    unsigned short slot = 0xFFFF;
    
    std::vector<Snapshot> snapshots;

    glm::vec2 ballPos;
    
    double currentTime = glfwGetTime();
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        glfwGetCursorPos(window, &cursorX, &cursorY);

        cursorX /= prevHeight / 2;
        cursorX -= prevWidth / prevHeight;
        cursorY /= -prevHeight / 2;
        cursorY += 1;

        Input input = processInput(window);
        bool clicked = input.click && !prevInput.click;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            if (!prevEsc) {
                if (gameState == State::PLAY) {
                    gameState = State::PAUSE;
                } else if (gameState == State::PAUSE) {
                    gameState = State::PLAY;
                }
            }
            prevEsc = true;
        } else {
            prevEsc = false;
        }

        double newTime = glfwGetTime();
        double delta = newTime - currentTime;
        currentTime = newTime;

        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao);

        if (gameState == State::EXIT) {
            glfwSetWindowShouldClose(window, 1);
        } else if (gameState == State::MENU) {
            handleButton(playButton, clicked, cursorX, cursorY, delta);
            handleButton(exitButton, clicked, cursorX, cursorY, delta);
            
            handleSlider(testSlider, cursorX, cursorY, input.click);

            glUseProgram(sliderShaderProgram);

            glUniformMatrix4fv(sliderModel, 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(testSlider.pos)));
            glUniform2fv(sliderSize, 1, glm::value_ptr(testSlider.size));
            glUniform1f(sliderCirclePos, testSlider.value);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

            std::string sliderStr = std::to_string(testSlider.value);
            sliderStr = sliderStr.substr(0, sliderStr.size() - 3);

            renderText(textShaderProgram, playButton.text, playButton.pos, playButton.textScale, glm::vec3(1.0f, 1.0f, 1.0f), largeCharacters);
            renderText(textShaderProgram, exitButton.text, exitButton.pos, exitButton.textScale, glm::vec3(1.0f, 1.0f, 1.0f), largeCharacters);
            renderText(textShaderProgram, sliderStr, glm::vec2(1.1f, 0.5f), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), smallCharacters);
        } else if (gameState == State::JOIN) {
            buffer[0] = (char)ClientMessage::Join;

            if (!net::socketSend(&sock, buffer, 1, &serverEndpoint)) {
                printf("join message failed\n");
                gameState = State::MENU;
            } else {
                gameState = State::PLAY;
            }
        } else if (gameState == State::PLAY || gameState == State::PAUSE) {
            serverTime += delta;

            net::IPEndpoint from;
            unsigned int bytesReceived;
            while (net::socketReceive(&sock, buffer, 57, &from, &bytesReceived)) {
                switch ((ServerMessage)buffer[0]) {
                    case ServerMessage::JoinResult: {
                        if (buffer[1]) {
                            memcpy(&slot, &buffer[2], sizeof(slot));
                        } else {
                            printf("server doesn't like us\n");
                        }
                        break;
                    }
                    case ServerMessage::State: {
                        numPlayers = 0;
                        unsigned int bytesRead = 1;

                        memcpy(&ballPos, &buffer[bytesRead], sizeof(ballPos));
                        bytesRead += sizeof(ballPos);

                        memcpy(&serverTime, &buffer[bytesRead], sizeof(serverTime));
                        bytesRead += sizeof(serverTime);

                        while (bytesRead < bytesReceived) {
                            unsigned short id;
                            memcpy(&id, &buffer[bytesRead], sizeof(id));
                            bytesRead += sizeof(id);

                            memcpy(&players[numPlayers].pos, &buffer[bytesRead], sizeof(players[numPlayers].pos));
                            bytesRead += sizeof(players[numPlayers].pos);

                            memcpy(&players[numPlayers].paddlePos, &buffer[bytesRead], sizeof(players[numPlayers].paddlePos));
                            bytesRead += sizeof(players[numPlayers].paddlePos);

                            memcpy(&players[numPlayers].stamina, &buffer[bytesRead], sizeof(players[numPlayers].stamina));
                            bytesRead += sizeof(players[numPlayers].stamina);

                            numPlayers++;
                        }

                        auto it = std::lower_bound(snapshots.begin(), snapshots.end(), serverTime - 0.05f);
                        if (it != snapshots.end() && it - snapshots.begin() > 0) {
                            snapshots.erase(snapshots.begin(), it - 1);
                        }
                        snapshots.emplace_back(glm::vec2{ players[0].pos, players[1].pos }, ballPos, serverTime);
                        break;
                    }
                    case ServerMessage::GoalScored:
                    {
                        char team;
                        memcpy(&team, &buffer[1], sizeof(team));
                        goals[team]++;
                        break;
                    }
                }
            }

            if (slot != 0xFFFF) {
                buffer[0] = (unsigned char)ClientMessage::Input;
                int bytesWritten = 1;

                memcpy(&buffer[bytesWritten], &slot, sizeof(slot));
                bytesWritten += sizeof(slot);

                unsigned char cInput = (unsigned char)input.w | ((unsigned char)input.s << 1) | ((unsigned char)input.a << 2) | ((unsigned char)input.d << 3) | ((unsigned char)input.click << 4) | ((unsigned char)input.sprint << 5);
                buffer[bytesWritten] = cInput;
                bytesWritten++;

                memcpy(&buffer[bytesWritten], &cursorX, sizeof(cursorX));
                bytesWritten += sizeof(cursorX);

                memcpy(&buffer[bytesWritten], &cursorY, sizeof(cursorY));
                bytesWritten += sizeof(cursorY);

                if (!net::socketSend(&sock, buffer, bytesWritten, &serverEndpoint)) {
                    printf("socketSend failed\n");
                }
            }

            glm::vec2 playerRenders[2];
            glm::vec2 ballRender;
            if (snapshots.size() < 1)
            {
                playerRenders[0] = glm::vec2(-0.3f, 0.0f);
                playerRenders[1] = glm::vec2(0.3f, 0.0f);
                ballRender = glm::vec2(0.0f, 0.0f);
            }
            else if (snapshots.size() == 1)
            {
                playerRenders[0] = snapshots[0].playerPositions[0];
                playerRenders[1] = snapshots[0].playerPositions[1];
                ballRender = snapshots[0].ballPos;
            }
            else
            {
                float difference = snapshots[1].time - snapshots[0].time;
                float otherDifference = (serverTime - 0.05f) - snapshots[0].time;
                playerRenders[0] = lerp(snapshots[0].playerPositions[0], snapshots[1].playerPositions[0], otherDifference / difference);
                playerRenders[1] = lerp(snapshots[0].playerPositions[1], snapshots[1].playerPositions[1], otherDifference / difference);
                ballRender = lerp(snapshots[0].ballPos, snapshots[1].ballPos, otherDifference / difference);
            }

            glUseProgram(obroundShaderProgram);

            glUniformMatrix4fv(obroundModel, 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(glm::vec2(-1.4f, 0.0f))));
            glUniform2fv(obroundSize, 1, glm::value_ptr(glm::vec2(0.02f, 0.2f)));
            glUniform3fv(obroundColor, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 1.0f)));
            glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

            glUniformMatrix4fv(obroundModel, 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(glm::vec2(1.4f, 0.0f))));
            glUniform2fv(obroundSize, 1, glm::value_ptr(glm::vec2(0.02f, 0.2f)));
            glUniform3fv(obroundColor, 1, glm::value_ptr(glm::vec3(1.0f, 0.0f, 0.0f)));
            glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

            glUseProgram(playerShaderProgram);

            for (int i = 0; i < numPlayers; ++i) {
                glUniform1f(playerSize, 0.1f);
                glUniform3fv(playerColor, 1, glm::value_ptr(glm::vec3(float(i % 2), 0.0f, fabs(float(i % 2) - 1.0f))));
                glUniform1f(playerStamina, (players[i].stamina - 0.5f) * 2 * 3.14159265f);
                glUniform2fv(paddleVec, 1, glm::value_ptr(players[i].paddlePos));
                glUniform1f(paddleSize, 0.05f);
                glUniformMatrix4fv(playerModel, 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(playerRenders[i])));
                glDrawArrays(GL_TRIANGLE_FAN, 0, 6);
            }

            glUseProgram(ballShaderProgram);

            glUniform1f(ballSize, 0.02f);
            glUniformMatrix4fv(ballModel, 1, GL_FALSE, glm::value_ptr(vec2ToModelMatrix(ballRender)));
            glDrawArrays(GL_TRIANGLE_FAN, 0, 6);

            if (gameState == State::PAUSE) {
                handleButton(resumeButton, clicked, cursorX, cursorY, delta);
                handleButton(menuButton, clicked, cursorX, cursorY, delta);
                renderText(textShaderProgram, resumeButton.text, resumeButton.pos, resumeButton.textScale, glm::vec3(1.0f, 1.0f, 1.0f), largeCharacters);
                renderText(textShaderProgram, menuButton.text, menuButton.pos, menuButton.textScale, glm::vec3(1.0f, 1.0f, 1.0f), largeCharacters);
            }

            renderText(textShaderProgram, std::to_string(goals[0]) + " - " + std::to_string(goals[1]), glm::vec2(0.0f, 0.9f), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f), largeCharacters);
        } else if (gameState == State::LEAVE) {
            buffer[0] = (unsigned char)ClientMessage::Leave;
            int bytesWritten = 1;
            memcpy(&buffer[bytesWritten], &slot, sizeof(slot));
            net::socketSend(&sock, buffer, bytesWritten, &serverEndpoint);
            gameState = State::MENU;
        }

        prevInput = input;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    buffer[0] = (unsigned char)ClientMessage::Leave;
    int bytesWritten = 1;
    memcpy(&buffer[bytesWritten], &slot, sizeof(slot));
    net::socketSend(&sock, buffer, bytesWritten, &serverEndpoint);

    glDeleteProgram(playerShaderProgram);
    glDeleteProgram(ballShaderProgram);
    glDeleteProgram(textShaderProgram);
    glfwTerminate();
    return;
}

void setVSync(bool sync) {
    typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALPROC)(int);
    PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

    const char* extensions = (char*)glGetString(GL_EXTENSIONS);

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(sync);
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_SHOW);

    if (!glfwInit()) {
        printf("GFLW failed to initialize.");
        return -1;
    }
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    prevWidth = mode->width;
    prevHeight = mode->height;

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Poooong", monitor, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        printf("Glew failed to initialize.");
        glfwTerminate();
        return -1;
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        printf("Could not init FreeType Library\n");
        glfwTerminate();
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "res/fonts/Bitter-Regular.otf", 0, &face)) {
        printf("Failed to load font\n");
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, windowResizeCallback);
    glfwSwapInterval(1);

    largeCharacters = initializeCharacters(face, 45);
    smallCharacters = initializeCharacters(face, 24);

    drawLoop(window);

    return 0;
}