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

    NetData init();
    game::GameData receive(NetData& netData);
    void send(const NetData& netData, const input::InputData& inputData);
}