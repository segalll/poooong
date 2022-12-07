#include "game.h"

namespace game
{
    void update(GameData& gameData, const GameData& netGameData, const input::InputData& inputData, State uiOutputState) {
        gameData.playerData = netGameData.playerData;
        gameData.ballData = netGameData.ballData;
        gameData.goals = netGameData.goals;

        if (inputData.escape.wasTapped()) {
            if (gameData.state == State::Play) {
                gameData.state = State::Pause;
            } else if (gameData.state == State::Pause) {
                gameData.state = State::Play;
            }
        }

        if (uiOutputState != State::None) {
            gameData.state = uiOutputState;
        }
    }
}