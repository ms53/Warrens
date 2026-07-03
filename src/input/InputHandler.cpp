#include "InputHandler.h"
#include "rendering/Camera.h"

void InputHandler::processEvent(const sf::Event& event)
{
	if (auto* mouseButton = event.getIf<sf::Event::MouseButtonPressed>())
	{
		if (mouseButton->button == sf::Mouse::Button::Right)
		{
			m_rightMouseDown = true;
			m_rightMousePressedEvent = true;
		}
		if (mouseButton->button == sf::Mouse::Button::Left)
		{
			m_leftMouseDown = true;
			m_leftMousePressedEvent = true;
		}
		if (mouseButton->button == sf::Mouse::Button::Middle)
		{
			m_middleMouseDown = true;
			m_middleMousePressedEvent = true;
		}
	}
	if (auto* released = event.getIf<sf::Event::MouseButtonReleased>())
	{
		if (released->button == sf::Mouse::Button::Right)
		{
			m_rightMouseDown = false;
			m_rightMousePressedEvent = false;
		}
		if (released->button == sf::Mouse::Button::Left)
		{
			m_leftMouseDown = false;
			m_leftMousePressedEvent = false;
		}
		if (released->button == sf::Mouse::Button::Middle)
		{
			m_middleMouseDown = false;
			m_middleMousePressedEvent = false;
		}
	}
	if (auto* move = event.getIf<sf::Event::MouseMoved>())
	{
		sf::Vector2i pos = move->position;

		m_mouseDelta = pos - m_lastMousePosition;
		m_lastMousePosition = pos;
	}
	if (auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>())
	{
		m_scrollDelta += wheel->delta;
	}
	if (auto* joystick = event.getIf<sf::Event::JoystickMoved>())
	{
		if (joystick->axis == sf::Joystick::Axis::X)
		{
			m_leftStick.x = joystick->position;
		}
		if (joystick->axis == sf::Joystick::Axis::Y)
		{
			m_leftStick.y = joystick->position;
		}

		if (joystick->axis == sf::Joystick::Axis::U)
		{
			m_rightStick.x = joystick->position;
		}
		if (joystick->axis == sf::Joystick::Axis::R)
		{
			m_rightStick.y = joystick->position;
		}

	}
}

void InputHandler::update()
{
	if (!sf::Joystick::isConnected(0))
		return;
	m_leftStick.x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X);
	m_leftStick.y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y);
	m_rightStick.x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::U);
	m_rightStick.y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::V);

	m_leftTrigger = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Z);
	m_rightTrigger = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::R);

	for (unsigned int i = 0; i < sf::Joystick::ButtonCount; i++)
	{
		m_prevButtons[i] = m_currButtons[i];
		m_currButtons[i] = sf::Joystick::isButtonPressed(0, i);
	}

	m_dpad.x = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX);
	m_dpad.y = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY);
}

float InputHandler::getScrollDelta()
{
	return m_scrollDelta;
}

sf::Vector2i InputHandler::getMouseDelta()
{
	return m_mouseDelta;
}

sf::Vector2f InputHandler::getMousePosition()
{
	return static_cast<sf::Vector2f>(m_lastMousePosition);
}

bool InputHandler::isButtonDown(unsigned int button) const 
{
	return m_currButtons[button];
}

bool InputHandler::isButtonPressed(unsigned int button) const
{
	return m_currButtons[button] && !m_prevButtons[button];
}

bool InputHandler::isButtonReleased(unsigned int button) const
{
	return !m_currButtons[button] && m_prevButtons[button];
}

void InputHandler::endFrame()
{
	m_mouseDelta = sf::Vector2i(0, 0);
	m_scrollDelta = 0.f;
	m_leftMousePressedEvent = false;
	m_leftMouseReleasedEvent = false;
	m_rightMousePressedEvent = false;
	m_rightMouseReleasedEvent = false;
	m_middleMousePressedEvent = false;
	m_middleMouseReleasedEvent = false;
}