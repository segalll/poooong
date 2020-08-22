#pragma once

#include <string>
#include <glm/vec2.hpp>

enum class State {
    JOIN,
    LEAVE,
    GAME,
    PAUSE,
    MENU,
    EXIT,
    PLAY
};

struct Input {
    bool w = false;
    bool a = false;
    bool s = false;
    bool d = false;
    bool click = false;
    bool sprint;
};

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

struct Button {
    glm::vec2 pos;
    glm::vec2 size;
    State onClick;
    std::string text;
    float textScale = 1.0f;

    Button(glm::vec2 bPos, glm::vec2 bSize, State bOnClick, std::string bText)
    {
        pos = bPos;
        size = bSize;
        onClick = bOnClick;
        text = bText;
    }
};

struct Slider {
    glm::vec2 pos;
    glm::vec2 size;
    float value;
    bool moving = false;

    Slider(glm::vec2 sPos, glm::vec2 sSize, float sValue)
    {
        pos = sPos;
        size = sSize;
        value = sValue;
    }
};

enum class ClientMessage : unsigned char
{
    Join, // tell server we're new here
    Leave, // tell server we're leaving
    Input // tell server our user input
};

enum class ServerMessage : unsigned char
{
    JoinResult, // tell client they're accepted/rejected
    State, // tell client game state
    GoalScored // tell client goal was scored
};

struct PlayerState
{
    glm::vec2 pos;
    glm::vec2 paddlePos;
    float stamina;
};

struct Snapshot
{
    glm::vec2 playerPositions[2];
    glm::vec2 ballPos;
    float time;

    bool operator <(float const i) const
    {
        return time < i;
    }
};