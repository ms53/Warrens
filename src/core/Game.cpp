#include "Game.h"
#include <iostream>

Game::Game()
    : window(sf::VideoMode::getDesktopMode(), "Warrens", sf::Style::Close),
    map(MapSize::MASSIVE),
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
    cameraController.update(camera, inputHandler, window, deltaTime);
    camera.update(deltaTime, window, sf::Mouse::getPosition(window));
    map.updateWrapping(camera.getView().getCenter().x);
    inputHandler.endFrame();
}