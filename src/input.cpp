#include "input.h"

namespace input
{
    InputData processInput(GLFWwindow* window, const InputData& inputData) {
        InputData newInputData = inputData;

        double cursorX, cursorY;
        glfwGetCursorPos(window, &cursorX, &cursorY);
        newInputData.cursorPos = glm::vec2(cursorX, cursorY);

        newInputData.cursorDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        newInputData.w.update(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
        newInputData.a.update(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
        newInputData.s.update(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
        newInputData.d.update(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
        newInputData.shift.update(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
        newInputData.escape.update(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);

        return newInputData;
    }
}