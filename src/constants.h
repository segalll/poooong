#pragma once

#include <glm/vec2.hpp>

enum class ClientMessage : unsigned char {
    Join, // tell server we're new here
    Leave, // tell server we're leaving
    Input // tell server our user input
};

enum class ServerMessage : unsigned char {
    JoinResult, // tell client they're accepted/rejected
    State, // tell client game state
    GoalScored // tell client goal was scored
};

struct PlayerState {
    glm::vec2 pos;
    glm::vec2 paddlePos;
    float stamina;
};

struct Snapshot {
    glm::vec2 playerPositions[2];
    glm::vec2 ballPos;
    float time;

    bool operator<(float const i) const {
        return time < i;
    }
};