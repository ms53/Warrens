#include "rendering/Camera.h"
#include <iostream>
#include <cmath>

Camera::Camera(const sf::RenderWindow& window,
    const Map& map,
    float startX,
    float startY)
    : m_view(window.getDefaultView()), m_map(map)
{
    m_targetPosition = { startX, startY };
    m_view.setCenter(m_targetPosition);

    m_baseViewSize = window.getView().getSize();

    if (m_baseViewSize.x <= 0.f || m_baseViewSize.y <= 0.f)
    {
        m_baseViewSize = { 1920.f, 1080.f };
    }

    configureForMap(map);
}

void Camera::update(float deltaTime, const sf::RenderWindow& window, const sf::Vector2i& mousePixel)
{
    m_targetPosition += m_velocity * deltaTime;
    m_velocity *= std::exp(-m_damping * deltaTime);

    float oldZoom = m_zoom;
    float t = 1.f - std::exp(-14.f * deltaTime);
    m_zoom += (m_targetZoom - m_zoom) * t;
    if (!std::isfinite(m_zoom)) m_zoom = 1.f;
    if (!std::isfinite(m_targetZoom)) m_targetZoom = 1.f;
    m_zoom = std::clamp(m_zoom, m_minZoom, m_maxZoom);

    sf::View newView;
    newView.setSize(m_baseViewSize * m_zoom);
    clampPosition(newView);
    newView.setCenter(m_targetPosition);

    if (std::abs(m_zoom - oldZoom) > 0.0001f)
    {
		sf::Vector2f currentAnchorWorld = window.mapPixelToCoords(m_zoomAnchorPixel, newView);
        sf::Vector2f correction = m_zoomAnchorPoint - currentAnchorWorld;

        m_targetPosition += correction * 0.6f;
        newView.setCenter(m_targetPosition);
    }

    newView.setCenter(m_targetPosition);

    m_view = newView;
}

void Camera::move(float offsetX, float offsetY)
{
	m_velocity += sf::Vector2f(offsetX, offsetY);
}

void Camera::zoom(float amount) {
    m_targetZoom *= amount;
    m_targetZoom = std::clamp(m_targetZoom, m_minZoom, m_maxZoom);
}

void Camera::zoomAt(float zoomFactor,
    const sf::RenderWindow& window,
    const sf::Vector2i& mousePixel)
{
	m_zoomAnchorPixel = mousePixel;
    m_zoomAnchorPoint = window.mapPixelToCoords(mousePixel, m_view);

    m_targetZoom *= zoomFactor;
    m_targetZoom = std::clamp(m_targetZoom, m_minZoom, m_maxZoom);
}

void Camera::configureForMap(const Map& map)
{
    m_mapWidth = map.getMapWorldWidth();
    m_mapHeight = map.getMapWorldHeight();

    if (m_mapWidth <= 0.f || m_mapHeight <= 0.f)
    {
        std::cerr << "Invalid map size!\n";
        m_maxZoom = 1.f;
        return;
    }

    sf::Vector2f base = m_baseViewSize;

    float zoomX = m_mapWidth / base.x;
    float zoomY = m_mapHeight / base.y;

    m_maxZoom = std::max(zoomX, zoomY);

    // critical safety clamp
    if (!std::isfinite(m_maxZoom) || m_maxZoom <= 0.f)
        m_maxZoom = 1.f;
}

void Camera::clampPosition(const sf::View& view)
{
    sf::Vector2f viewSize = view.getSize();
    sf::Vector2f halfView = viewSize * 0.5f;

    float minX = halfView.x;
    float minY = halfView.y;

    float maxX = m_mapWidth - halfView.x;
    float maxY = m_mapHeight - halfView.y;

   //  X axis
    /*if (minX > maxX)
    {
        m_targetPosition.x = m_mapWidth * 0.5f;
    }
    else
    {
        m_targetPosition.x = std::clamp(m_targetPosition.x, minX, maxX);
    }*/

    // Y axis
    if (minY > maxY)
    {
        m_targetPosition.y = m_mapHeight * 0.5f;
    }
    else
    {
        m_targetPosition.y = std::clamp(m_targetPosition.y, minY, maxY);
    }
}