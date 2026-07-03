#pragma once
#include <SFML/Window.hpp>

class InputHandler
{
public:
	 void processEvent(const sf::Event& event);
	 void update();
	 void endFrame();

	bool WPressed() { return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up); }
	bool APressed() { return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left); }
	bool SPressed() { return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down); }
	bool DPressed() { return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right); }
	bool LeftIsDown() const { return m_leftMouseDown; }
	bool LeftPressed() const { return m_leftMousePressedEvent; }
	bool LeftReleased() const { return m_leftMouseReleasedEvent; }
	bool RightIsDown() const { return m_rightMouseDown; }
	bool RightPressed() const { return m_rightMousePressedEvent; }
	bool RightReleased() const { return m_rightMouseReleasedEvent; }
	bool MiddleIsDown() const { return m_middleMouseDown; }
	bool MiddlePressed() const { return m_middleMousePressedEvent; }
	bool MiddleReleased() const { return m_middleMouseReleasedEvent; }

	 sf::Vector2i getMouseDelta();
	 sf::Vector2f getMousePosition();
	 sf::Vector2f getLeftStick() { return m_leftStick; }
	 sf::Vector2f getRightStick() { return m_rightStick; }
	 float getScrollDelta();
	 bool isButtonDown(unsigned int button) const;
	 bool isButtonPressed(unsigned int button) const;
	 bool isButtonReleased(unsigned int button) const;

	 float getLeftTrigger() const { return (m_leftTrigger + 100.f) / 200.f; }
	 float getRightTrigger() const { return (m_rightTrigger + 100.f) / 200.f; }

	 bool dpadLeft() const { return m_dpad.x < -50.f; }
	 bool dpadRight() const { return m_dpad.x > 50.f; }
	 bool dpadUp() const { return m_dpad.y > 50.f; }
	 bool dpadDown() const { return m_dpad.y < -50.f; }

private:
	 sf::Vector2i m_lastMousePosition{ 0,0 };
	 sf::Vector2i m_mouseDelta{ 0,0 };

	 bool m_leftMouseDown = false;
	 bool m_leftMousePressedEvent = false;
	 bool m_leftMouseReleasedEvent = false;

	 bool m_rightMouseDown = false;
	 bool m_rightMousePressedEvent = false;
	 bool m_rightMouseReleasedEvent = false;

	 bool m_middleMouseDown = false;
	 bool m_middleMousePressedEvent = false;
	 bool m_middleMouseReleasedEvent = false;

	 sf::Vector2f m_leftStick{ 0.f, 0.f };
	 sf::Vector2f m_rightStick{ 0.f, 0.f };
	 sf::Vector2f m_dpad{ 0.f, 0.f };
	 float m_leftTrigger = 0.f;
	 float m_rightTrigger = 0.f;
	 std::array<bool, sf::Joystick::ButtonCount> m_prevButtons{};
	 std::array<bool, sf::Joystick::ButtonCount> m_currButtons{};

	  float m_scrollDelta = 0.f;

};