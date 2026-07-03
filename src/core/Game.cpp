#include "Game.h"
#include <iostream>

Game::Game()
    : window(sf::VideoMode::getDesktopMode(), "Warrens", sf::Style::Close),
    map(MapSize::STANDARD),
    camera(window, map, 0.f, 0.f)
{
    
}

void Game::run()
{
    while (window.isOpen())
    {
        processEvents();
        update();
        render();
    }
}

void Game::processEvents()
{
    while (auto event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            window.close();

        inputHandler.processEvent(*event);
    }
}

void Game::render()
{
    window.clear(sf::Color::Black);
    window.setView(camera.getView());

	map.draw(window);

    window.display();
}

void Game::update()
{
    float deltaTime = clock.restart().asSeconds();

    inputHandler.update();
    if (inputHandler.RightPressed())
    {
        sf::Vector2f worldPos =
            window.mapPixelToCoords(sf::Mouse::getPosition(window), camera.getView());

        if (Tile* tile = map.getTileAtPosition(worldPos))
        {
            tile->setColor(sf::Color::Yellow);
            int x = tile->getCoords().x;
            int y = tile->getCoords().y;
            std::vector<Tile*> neighbors;
            map.getNeighbors(x, y, neighbors);
            for (Tile* neighbor : neighbors)
            {
                neighbor->setColor(sf::Color::Cyan);
            }
        }
    }
    cameraController.update(camera, inputHandler, window, deltaTime);
    camera.update(deltaTime, window, sf::Mouse::getPosition(window));
    map.updateWrapping(camera.getView().getCenter().x);
    inputHandler.endFrame();
}