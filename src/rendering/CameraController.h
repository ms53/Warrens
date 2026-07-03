#pragma once
#include "Camera.h"
#include "input/InputHandler.h"
#include <SFML/Graphics.hpp>

class CameraController {
public:
    void update(Camera& camera,
        InputHandler& inputHandler,
        const sf::RenderWindow& window,
        float deltaTime
    );
};
