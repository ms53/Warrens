#include "CameraController.h"
#include <iostream>
static float const SPEED = 5.f;
void CameraController::update(Camera& camera,
    InputHandler& inputHandler,
    const sf::RenderWindow& window,
    float deltaTime
) {
	float speed = camera.getSpeed();
	sf::View view = camera.getView();
	sf::Vector2f dir(0.f, 0.f);
	if (inputHandler.WPressed())
		dir.y -= SPEED;
	if (inputHandler.APressed())
		dir.x -= SPEED;
	if (inputHandler.SPressed())
		dir.y += SPEED;
	if (inputHandler.DPressed())
		dir.x += SPEED;

	camera.move(dir.x*speed, dir.y*speed);

	if (inputHandler.MiddleIsDown() || inputHandler.LeftIsDown()) {
		sf::Vector2i mouseDelta = inputHandler.getMouseDelta();
		camera.move(-mouseDelta.x * 10.f, -mouseDelta.y * 10.f);
	}
	sf::Vector2f look = inputHandler.getLeftStick() / 100.f;
	constexpr float joystickDeadzone = 0.2f;

	if (std::abs(look.x) < joystickDeadzone) look.x = 0.f;
	if (std::abs(look.y) < joystickDeadzone) look.y = 0.f;

	camera.move(look.x * 10 * speed, look.y * 10 * speed);

	float zoomInTrigger = inputHandler.getRightTrigger();
	float zoomOutTrigger = inputHandler.getLeftTrigger();
	float triggerZoomAmount = zoomInTrigger - zoomOutTrigger;

	if (triggerZoomAmount != 0.f)
	{
		camera.zoomAt(
			std::pow(0.9f, triggerZoomAmount),
			window,
			sf::Mouse::getPosition(window));
	}

	float scrollZoom = inputHandler.getScrollDelta();

	if (scrollZoom != 0.f)
	{
		camera.zoomAt(
			std::pow(0.9f, scrollZoom),
			window,
			sf::Mouse::getPosition(window));
	}

	


}
