#include "game.h"

namespace game
{
    GameState update(GameState gameState, const input::InputData& inputData, GameState uiOutputState) {
        if (inputData.escape.wasTapped()) {
            if (gameState == GameState::Play) {
                return GameState::Pause;
            } else if (gameState == GameState::Pause) {
                return GameState::Play;
            }
        }

        if (gameState == GameState::Join) {
            return GameState::Play;
        } else if (gameState == GameState::Leave) {
            return GameState::Menu;
        }

        if (uiOutputState != GameState::None) {
            return uiOutputState;
        }

        return gameState;
    }
}