#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include "MapSize.h"
#include "WorldType.h"
#include "Tile.h"
#include "Perlin.h"
class Map
{
private:
	std::vector<Tile> tiles;
	int width = 0;
	int height = 0;
	float mapWorldWidth = 0.f;
	float mapWorldHeight = 0.f;
	void computeWorldSize()
	{
		mapWorldWidth = width * TileMetrics::Width;
		mapWorldHeight = height * TileMetrics::Height * 0.75f;
	}
	float wrapOffsetX = 0.f;
	MapSize mapSize;

	struct ContinentSeed { sf::Vector2f pos; float radius; };
	float macroArchipelago(const sf::Vector2f& p, Perlin& n) const;
	float macroContinents(const sf::Vector2f& p, Perlin& n) const;
	float macroPangaea(const sf::Vector2f& p, Perlin& n) const;
	float macroFractal(const sf::Vector2f& p, Perlin& n) const;
	float macroInlandSea(const sf::Vector2f& p, Perlin& n) const;

	float continentalFractal(const sf::Vector2f& p, Perlin& noise) const;
	float archipelagoFractal(const sf::Vector2f& p,Perlin& noise) const;
	float fractalFractal(const sf::Vector2f& p, Perlin& noise)const;
	//float inlandSeaFractal(const sf::Vector2f& p, Perlin& noise) const;
	sf::Vector2f warpPosition(const sf::Vector2f& pos, Perlin& noise) const;
	bool isLand(const Tile& tile);

public:
	Map(MapSize size, WorldType type);
	void draw(sf::RenderWindow& window);
	void generateTerrain(WorldType type);
	std::vector<std::vector<Tile*>> findLandmasses();

	int getWidth() const { return width; }
	int getHeight() const { return height; }
	float getMapWorldWidth() const { return mapWorldWidth; }
	float getMapWorldHeight() const { return mapWorldHeight; }
	MapSize getMapSize() const { return mapSize; }
	Tile* getTile(int x, int y);
	Tile* getTileAtPosition(const sf::Vector2f& pos);
	void getNeighbors(int x, int y, std::vector<Tile*>& out);

	struct CubeCoord {
		int x, y, z;
	};

	struct CubeCoordF {
		float x, y, z;
	};
	
	CubeCoord offsetToCube(int x, int y) const;
	std::pair<int, int> cubeToOffset(const CubeCoord& c) const;
	CubeCoordF worldToCube(const sf::Vector2f& pos) const;
	CubeCoord cubeRound(const CubeCoordF& cube) const;

	void updateWrapping(float cameraX);
	void shiftLeft();
	void shiftRight();
};