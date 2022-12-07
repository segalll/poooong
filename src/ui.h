#pragma once

#include "game.h"
#include "input.h"

#include <glm/vec2.hpp>

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ui
{
    struct Button {
        glm::vec2 pos;
        glm::vec2 size;
        std::string text;
        game::State onClick;
        float textScale = 1.0f;
    };

    struct Slider {
        glm::vec2 pos;
        glm::vec2 size;
        float value;
        bool moving = false;
    };

    using UiData = std::unordered_map<
        game::State,
        std::vector<
            std::variant<
                Button,
                Slider
            >
        >
    >; // maps game state to visible ui elements

    UiData init();
    game::State handle(const UiData& uiData, const input::InputData& inputData, float dt, game::State gameState);
}