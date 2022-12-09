#include "input.h"
#include "game.h"
#include "net.h"
#include "render.h"
#include "ui.h"

#include <stdexcept>

int main() {
    if (!glfwInit()) {
        throw std::runtime_error("GLFW failed to initialize");
    }
        
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Poooong", monitor, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    render::RenderData renderData = render::init(mode->width, mode->height);

    glfwSetWindowUserPointer(window, &renderData);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        if (width == 0 || height == 0) return;
        render::RenderData* renderData = (render::RenderData*)glfwGetWindowUserPointer(window);
        render::resize(*renderData, width, height);
    });

    game::GameState gameState;
    input::InputData inputData;
    net::NetData netData = net::init();
    ui::UiData uiData = ui::init();

    double currentTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double newTime = glfwGetTime();
        double dt = newTime - currentTime;
        currentTime = newTime;

        inputData = input::processInput(window, inputData);
        game::GameState uiOutputState = ui::handle(uiData, inputData, dt, gameState);
        net::GameData gameData = net::receive(netData);
        net::send(netData, inputData);
        render::render(renderData, gameData, uiData);
        game::update(gameState, inputData, uiOutputState);

        if (gameState == game::GameState::Exit) {
            glfwSetWindowShouldClose(window, 1);
        } else if (gameState == game::GameState::Join) {
            net::join(netData);
        } else if (gameState == game::GameState::Leave) {
            net::leave(netData);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    net::leave(netData);

    glfwTerminate();

    return 0;
}