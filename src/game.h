#pragma once

#include "input.h"

#include <glm/vec2.hpp>

#include <vector>

namespace game
{
    enum class State {
        None,
        Join,
        Leave,
        Game,
        Pause,
        Menu,
        Exit,
        Play
    };

    struct PlayerData {
        glm::vec2 pos;
        glm::vec2 paddlePos;
        float stamina;
    };

    struct BallData {
        glm::vec2 pos;
    };

    struct GameData {
        State state;
        std::vector<PlayerData> playerData;
        BallData ballData;
        glm::ivec2 goals;
    };

    void update(GameData& gameData, const GameData& netGameData, const input::InputData& inputData, State uiOutputState);
}