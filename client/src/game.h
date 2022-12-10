#pragma once

#include "input.h"

#include <glm/vec2.hpp>

#include <vector>

namespace game
{
    enum class GameState {
        None,
        Join,
        Leave,
        Game,
        Pause,
        Menu,
        Exit,
        Play
    };

    GameState update(GameState gameState, const input::InputData& inputData, GameState uiOutputState, GameState netOutputState);
}