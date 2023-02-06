#pragma once

#include "game.h"
#include "input.h"
#include "socket.h"

#include <glm/vec2.hpp>

namespace net
{
    enum class ClientMessage : char {
        Join,
        Leave,
        Input
    };

    enum class ServerMessage : char {
        JoinResult,
        State,
        GoalScored
    };

    struct Snapshot {
        std::vector<glm::vec2> playerPositions;
        glm::vec2 ballPos;
        float time;

        Snapshot(std::vector<glm::vec2>&& pp, glm::vec2 bp, float t)
            : playerPositions(pp)
            , ballPos(bp)
            , time(t)
        {}

        bool operator<(float i) const {
            return time < i;
        }
    };

    struct ServerData {
        float time = 0.0f;
        unsigned int playerCount = 0;
        unsigned char slot = 0xFF;
    };

    struct NetData {
        socket::SocketData socketData;
        ServerData serverData;
        std::vector<Snapshot> snapshots;
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
        std::vector<PlayerData> playerData;
        BallData ballData;
        glm::ivec2 goals;
    };

    NetData init();
    GameData receive(NetData& netData, float dt);
    void send(NetData& netData, const input::InputData& inputData);
    game::GameState join(NetData& netData);
    void leave(NetData& netData);
}