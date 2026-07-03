#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include "rendering/Camera.h"
#include "rendering/CameraController.h"
#include "input/InputHandler.h"
#include "map/Tile.h"
#include "map/Map.h"
#include "map/MapSize.h"

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update();
    void render();

private:
    sf::RenderWindow window;
    sf::Clock clock;

    Map map;
    Camera camera;
    CameraController cameraController;
    InputHandler inputHandler;

	
    

    std::vector<Tile> tiles;

    // --- joystick debug state ---
    float lastRightStickX = 0.f;
    float lastRightStickY = 0.f;
};