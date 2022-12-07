//#include <Windows.h>
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
#include "shader.h"
#include "net.h"
#include "render.h"

#include <ft2build.h>
#include FT_FREETYPE_H

float prevWidth;
float prevHeight;
State gameState = State::MENU;

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) return;
    prevWidth = width;
    prevHeight = height;
    unsigned int ubo = *(unsigned int*)glfwGetWindowUserPointer(window);

    setProjectionMatrix(ubo, width, height);

    glViewport(0, 0, width, height);
}

glm::vec2 lerp(glm::vec2 a, glm::vec2 b, float t) {
    return a + (t * (b - a));
}

void drawLoop(GLFWwindow* window) {
    glm::ivec2 goals = glm::ivec2(0, 0);

    double cursorX;
    double cursorY;

    bool prevEsc = false;

    net::Socket sock;
    if (!net::socketCreate(&sock)) {
        throw std::runtime_error("Socket creation failed");
    }
    
    net::IPEndpoint serverEndpoint = net::ipEndpointCreate("127.0.0.1", 9999);
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

    glfwSetFramebufferSizeCallback(window, windowResizeCallback);
    glfwSwapInterval(1);

    render::RenderData renderData = render::init(window);

    drawLoop(window);

    return 0;
}