#include <SFML/Graphics.hpp>
#include "Tile.h"
#include <cmath>
const float TILE_SIZE = TileMetrics::Size;
static const float HEX_WIDTH = std::sqrt(3.f) * TILE_SIZE;
static const float HEX_HEIGHT = 2.f * TILE_SIZE;
Tile::Tile(int x, int y)
	: x(x), y(y)
{
	hex.setPointCount(6);
	hex.setPoint(0, sf::Vector2f(0.f, -TILE_SIZE));
	hex.setPoint(1, sf::Vector2f(HEX_WIDTH/2.f, -TILE_SIZE/2.f));
	hex.setPoint(2, sf::Vector2f(HEX_WIDTH/2.f, TILE_SIZE/2.f));
	hex.setPoint(3, sf::Vector2f(0.f, TILE_SIZE));
	hex.setPoint(4, sf::Vector2f(-HEX_WIDTH/2.f, TILE_SIZE/2.f));
	hex.setPoint(5, sf::Vector2f(-HEX_WIDTH/2.f, -TILE_SIZE/2.f));

	float worldX = x * HEX_WIDTH + (y%2) * (HEX_WIDTH/2.f);
	float worldY = y * (HEX_HEIGHT * 0.75f);

	hex.setPosition(sf::Vector2f(worldX, worldY));

	hex.setFillColor(sf::Color::Transparent);
	hex.setOutlineThickness(1.f);
	hex.setOutlineColor(sf::Color::White);

}

void Tile::draw(sf::RenderWindow& window)
{
	window.draw(hex);
}
 sf::ConvexShape& Tile::getHex()
{
	return hex;
}

 float Tile::getTileSize()
 {
	 return TILE_SIZE;
 }

 int Tile::getX() const
 {
	 return x;
 }

 int Tile::getY() const
 {
	 return y;
 }

 Elevation Tile::classifyElevation(float h) {
	 if (h < 0.20f)
		 return Elevation::Deep_Sea;
	 if (h < 0.40f)
		 return Elevation::Shallow_Sea;

	 if (h < 0.65f)
		 return Elevation::Flat;

	 if (h < 0.80f)
		 return Elevation::Hill;

	 return Elevation::Mountain;
 }