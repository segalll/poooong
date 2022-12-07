#include "net.h"

namespace net
{
    NetData init() {
        return {
            .socketData = socket::init()
        };
    }

    constexpr glm::vec2 lerp(glm::vec2 a, glm::vec2 b, float t) {
        return a + (t * (b - a));
    }

    game::GameData receive(NetData& netData) {
        game::GameData gameData;

        while (true) {
            std::string packet = socket::receive(netData.socketData);
            if (packet.size() == 0) break;
            switch ((ServerMessage)packet[0]) {
                case ServerMessage::JoinResult: {
                    if (packet[1]) {
                        memcpy(&netData.serverData.slot, &packet[2], sizeof(netData.serverData.slot));
                    } else {
                        printf("server doesn't like us\n");
                    }
                    break;
                }
                case ServerMessage::State: {
                    netData.serverData.playerCount = 0;

                    int bytesRead = 1;

                    memcpy(&gameData.ballData.pos, &packet[bytesRead], sizeof(gameData.ballData.pos));
                    bytesRead += sizeof(gameData.ballData.pos);

                    memcpy(&netData.serverData.time, &packet[bytesRead], sizeof(netData.serverData.time));
                    bytesRead += sizeof(netData.serverData.time);

                    while (bytesRead < packet.size()) {
                        if (gameData.playerData.size() <= netData.serverData.playerCount) {
                            gameData.playerData.push_back({});
                        }

                        unsigned char id;
                        memcpy(&id, &packet[bytesRead], sizeof(id));
                        bytesRead += sizeof(id);

                        memcpy(&gameData.playerData[netData.serverData.playerCount].pos, &packet[bytesRead], sizeof(gameData.playerData[netData.serverData.playerCount].pos));
                        bytesRead += sizeof(gameData.playerData[netData.serverData.playerCount].pos);

                        memcpy(&gameData.playerData[netData.serverData.playerCount].paddlePos, &packet[bytesRead], sizeof(gameData.playerData[netData.serverData.playerCount].paddlePos));
                        bytesRead += sizeof(gameData.playerData[netData.serverData.playerCount].paddlePos);

                        memcpy(&gameData.playerData[netData.serverData.playerCount].stamina, &packet[bytesRead], sizeof(gameData.playerData[netData.serverData.playerCount].stamina));
                        bytesRead += sizeof(gameData.playerData[netData.serverData.playerCount].stamina);

                        ++netData.serverData.playerCount;
                    }

                    auto it = std::lower_bound(netData.snapshots.begin(), netData.snapshots.end(), netData.serverData.time - 0.05f);
                    if (it != netData.snapshots.end() && it - netData.snapshots.begin() > 0) {
                        netData.snapshots.erase(netData.snapshots.begin(), it - 1);
                    }
                    netData.snapshots.emplace_back(gameData.playerData, gameData.ballData.pos, netData.serverData.time);
                    break;
                }
                case ServerMessage::GoalScored: {
                    char team;
                    memcpy(&team, &packet[1], sizeof(team));
                    gameData.goals[team]++;
                    break;
                }
            }
        }

        // game data may or may not be empty and cause a runtime error
        // i have not though it through very much yet
        if (netData.snapshots.size() < 1) {
            gameData.playerData[0].pos = glm::vec2(-0.3f, 0.0f);
            gameData.playerData[1].pos = glm::vec2(0.3f, 0.0f);
            gameData.ballData.pos = glm::vec2(0.0f, 0.0f);
        } else if (netData.snapshots.size() == 1) {
            for (int i = 0; i < netData.snapshots[0].playerPositions.size(); ++i) {
                gameData.playerData[i].pos = netData.snapshots[0].playerPositions[i];
            }
            gameData.ballData.pos = netData.snapshots[0].ballPos;
        } else {
            float denominator = netData.snapshots[1].time - netData.snapshots[0].time;
            float numerator = (netData.serverData.time - 0.05f) - netData.snapshots[0].time;
            for (int i = 0; i < netData.snapshots[0].playerPositions.size() && i < netData.snapshots[1].playerPositions.size(); ++i) {
                gameData.playerData[i].pos = lerp(netData.snapshots[0].playerPositions[i], netData.snapshots[1].playerPositions[i], numerator / denominator);
            }
            gameData.ballData.pos = lerp(netData.snapshots[0].ballPos, netData.snapshots[1].ballPos, numerator / denominator);
        }

        return gameData;
    }

    void send(const NetData& netData, const input::InputData& inputData) {
        if (netData.serverData.slot != 0xFF) {
            char buffer[57];
            buffer[0] = (char)ClientMessage::Input;
            int bytesWritten = 1;

            memcpy(&buffer[bytesWritten], &netData.serverData.slot, sizeof(netData.serverData.slot));
            bytesWritten += sizeof(netData.serverData.slot);

            unsigned char encodedInput = (unsigned char)inputData.w.down | ((unsigned char)inputData.s.down << 1) | ((unsigned char)inputData.a.down << 2) |
                ((unsigned char)inputData.d.down << 3) | ((unsigned char)inputData.cursorDown << 4) | ((unsigned char)inputData.shift.down << 5);
            buffer[bytesWritten] = encodedInput;
            ++bytesWritten;

            memcpy(&buffer[bytesWritten], &inputData.cursorPos.x, sizeof(inputData.cursorPos.x));
            bytesWritten += sizeof(inputData.cursorPos.x);

            memcpy(&buffer[bytesWritten], &inputData.cursorPos.y, sizeof(inputData.cursorPos.y));
            bytesWritten += sizeof(inputData.cursorPos.y);

            socket::send(netData.socketData, buffer);
        }
    }
}