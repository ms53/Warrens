#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
enum class BiomeType {
	Null,
	Grassland,
	Steppe,
	Desert,
	Jungle,
	Tundra,
	Snow,
	Coast,
	Sea,
	Ocean,
	Ice
};

enum class FeatureType {
	None,
	Forest,
	Rainforest,
	Marsh,
	Floodplain
};

enum class Elevation {
	Mountain,
	Hill,
	Flat,
	Shallow_Sea,
	Deep_Sea
};

enum class Humidity {
	Arid,
	Temperate,
	Humid
};

enum class Temperature {
	Cold,
	Temperate,
	Hot
};

struct River {
	bool northEast;
	bool east;
	bool southEast;
	bool southWest;
	bool west;
	bool northWest;
};

class Tile
{
private:
	int x, y;
	sf::ConvexShape hex;

	float elevation = 0.f;
	float moisture = 0.f;
	float temperature = 0.f;

	Elevation elevationType;
	BiomeType biome = BiomeType::Null;

public:
	Tile(int x, int y);
	void draw(sf::RenderWindow& window);
	//int* getCoords();
	int getX() const;
	int getY() const;
	sf::ConvexShape& getHex();
	static float getTileSize();
	void setColor(const sf::Color& color) {
		if (this == nullptr) {
			std::cout << "Tile is null!\n";
			return;
		}
		hex.setFillColor(color);
	}
	void setElevation(float e) { elevation = e; }
	void setElevationType(Elevation e) { elevationType = e; }
	float getElevation() const { return elevation; }
	Elevation getElevationType() const { return elevationType; }
	Elevation classifyElevation(float h);
	void setMoisture(float m) { moisture = m; }
	float getMoisture() const { return moisture; }
	void setTemperature(float t) { temperature = t; }
	float getTemperature() const { return temperature; }

	void setBiome(BiomeType b) { biome = b; }
	BiomeType getBiome() const { return biome; }
	sf::Vector2i getCoords() const {
		return { x, y };
	}
};
struct TileMetrics {
	static constexpr float Size = 100.f;
	static constexpr float Width = 1.73205080757f * Size; //std::sqrt(3.f) * Size
	static constexpr float Height = 2.f * Size;
};



