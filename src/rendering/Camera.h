#pragma once

#include <SFML/Graphics.hpp>
#include "map/Map.h"

class Camera
{
public:
	explicit Camera(const sf::RenderWindow& window, const Map& map, float startX, float startY);

	void update(float deltaTime, const sf::RenderWindow& window, const sf::Vector2i& mousePixel);

	void move(float offsetX, float offsetY);

	void zoom(float amount);

	void zoomAt(float zoomFactor, const sf::RenderWindow& window, const sf::Vector2i& mousePixel);

	const sf::View& getView() const { return m_view; }

	const float getSpeed() const { return m_speed; }

	void setVelocity(sf::Vector2f v) { m_velocity = v; }

	const Map& getMap() const { return m_map; }

private:
	sf::View m_view;
	sf::Vector2f m_targetPosition;
	sf::Vector2f m_velocity{0.f, 0.f};
	float m_damping = 8.f;
	float m_speed = 50.0f;
	float m_mapWidth;
	float m_mapHeight;

	sf::Vector2f m_baseViewSize;
	float m_zoom = 1.f;
	float m_targetZoom = 1.f;

	float m_minZoom = 0.5f;
	float m_maxZoom = 2.f;

	bool m_zoomTargetChangedThisFrame = false;
	sf::Vector2f m_zoomAnchorPoint{ 0.f, 0.f };
	sf::Vector2i m_zoomMousePixel;
	sf::Vector2i m_zoomAnchorPixel;

	const Map& m_map;

	void configureForMap(const Map& map);

	void clampPosition(const sf::View& view);

};