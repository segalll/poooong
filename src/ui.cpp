#include "ui.h"

#include <math.h>

namespace ui
{
    void createButton(UiData& uiData, const glm::vec2& position, const glm::vec2& size, const std::string& text, game::State onClick, game::State gameState) {

        uiData[gameState].push_back(Button{position, size, text, onClick}); // could change to emplace_back, but I don't trust it with std::variant yet
    }

    void createSlider(UiData& uiData, const glm::vec2& position, const glm::vec2& size, float value, game::State gameState) {
        uiData[gameState].push_back(Slider{position, size, value});
    }
    
    UiData init() {
        UiData uiData;

        createButton(uiData, glm::vec2(0.0f, 0.0f), glm::vec2(0.5f, 0.15f), "Play", game::State::Join, game::State::Menu);
        createButton(uiData, glm::vec2(0.0f, -0.4f), glm::vec2(0.5f, 0.15f), "Exit", game::State::Exit, game::State::Menu);
        createButton(uiData, glm::vec2(0.0f, 0.0f), glm::vec2(0.5f, 0.15f), "Resume", game::State::Game, game::State::Pause);
        createButton(uiData, glm::vec2(0.0f, -0.4f), glm::vec2(0.5f, 0.15f), "Menu", game::State::Leave, game::State::Pause);
        createSlider(uiData, glm::vec2(0.5f, 0.5f), glm::vec2(0.4f, 0.025f), 0.5f, game::State::Menu);

        return uiData;
    }

    constexpr bool pointAndRectCollide(const glm::vec2& point, const glm::vec2& rectPos, const glm::vec2& rectDim) {
        return point.x > rectPos.x - (rectDim.x / 2) &&
               point.x < rectPos.x + (rectDim.x / 2) &&
               point.y > rectPos.y - (rectDim.y / 2) &&
               point.y < rectPos.y + (rectDim.y / 2);
    }

    std::pair<Button, game::State> handleButton(const Button& button, const input::InputData& inputData, double dt) {
        Button newButton = button;
        game::State outputState = game::State::None;
        if (pointAndRectCollide(inputData.cursorPos, button.pos, button.size)) {
            newButton.textScale = fmin(button.textScale + (dt * 2), 1.2f);
            if (inputData.cursorDown) outputState = button.onClick;
        } else {
            newButton.textScale = fmax(button.textScale - (dt * 2), 1.0f);
        }

        return std::pair(newButton, outputState);
    }

    Slider handleSlider(const Slider& slider, const input::InputData& inputData) {
        Slider newSlider = slider;

        if (!inputData.cursorDown) {
            newSlider.moving = false;
        }

        if (slider.moving) {
            float translatedCursor = fmax(fmin(inputData.cursorPos.x, slider.pos.x + slider.size.x - slider.size.y), slider.pos.x - slider.size.x + slider.size.y);
            newSlider.value = roundf((translatedCursor - (slider.pos.x - slider.size.x + slider.size.y)) / ((slider.size.x * 2) - (slider.size.y * 2)) * 500) / 500;
        } else {
            if (inputData.cursorDown && inputData.cursorPos.y < slider.pos.y + slider.size.y &&
                inputData.cursorPos.y > slider.pos.y - slider.size.y && inputData.cursorPos.x < slider.pos.x + slider.size.x &&
                inputData.cursorPos.x > slider.pos.x - slider.size.x) {
                newSlider.moving = true;
            }
        }

        return newSlider;
    }

    // could eventually use templates if adding more UI element types
    // also an alternative solution to copy the UI elements would be nice
    void handle(UiData& uiData, const input::InputData& inputData, float dt, game::State gameState) {
        game::State outputState = game::State::None;
        for (auto& element : uiData.at(gameState)) {
            if (std::holds_alternative<Button>(element)) {
                game::State result;
                std::tie(element, result) = handleButton(std::get<Button>(element), inputData, dt);
                if (result != game::State::None) {
                    outputState = result;
                }
            } else if (std::holds_alternative<Slider>(element)) {
                element = handleSlider(std::get<Slider>(element), inputData);
            }
        }
    }
}