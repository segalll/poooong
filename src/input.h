#pragma once

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace input
{
    struct KeyState {
        bool down;
        bool previousDown;

        bool wasTapped() const {
            return down && !previousDown;
        }

        void update(bool keyDown) {
            previousDown = down;
            down = keyDown;
        }
    };

    struct InputData {
        glm::vec2 cursorPos;
        bool cursorDown;
        KeyState w, a, s, d;
        KeyState shift;
        KeyState escape;
    };

    InputData processInput(GLFWwindow* window, const InputData& inputData);
}