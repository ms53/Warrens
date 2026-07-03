#include <SFML/Graphics.hpp>
#include "Map.h"
#include <iostream>
#include "Perlin.h"
#include <random>
Map::Map(MapSize size)
{
	switch (size)
	{
	case MapSize::DUEL:     width = 44;  height = 26; break;
	case MapSize::TINY:     width = 60;  height = 38; break;
	case MapSize::SMALL:    width = 74;  height = 46; break;
	case MapSize::STANDARD: width = 84;  height = 54; break;
	case MapSize::LARGE:    width = 96;  height = 60; break;
	case MapSize::HUGE:     width = 106; height = 66; break;
	case MapSize::MASSIVE:	width = 152; height = 96; break;
	}

	computeWorldSize();

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			tiles.emplace_back(j, i);
		}
	}
	generateTerrain(WorldType::Continents);
}

void Map::draw(sf::RenderWindow& window)
{
	sf::View view = window.getView();

	sf::Vector2f center = view.getCenter();
	sf::Vector2f size = view.getSize();

	float left = center.x - size.x * 0.5f;
	float right = center.x + size.x * 0.5f;
	float top = center.y - size.y * 0.5f;
	float bottom = center.y + size.y * 0.5f;

	int minX = (int)(left / TileMetrics::Width) - 2;
	int maxX = (int)(right / TileMetrics::Width) + 2;

	int minY = (int)(top / (TileMetrics::Height * 0.75f)) - 2;
	int maxY = (int)(bottom / (TileMetrics::Height * 0.75f)) + 2;

	minX = std::max(0, minX);
	minY = std::max(0, minY);
	maxX = std::min(width - 1, maxX);
	maxY = std::min(height - 1, maxY);

	float camX = view.getCenter().x;
	float camY = view.getCenter().y;

	float viewWidth = view.getSize().x;
	float worldW = mapWorldWidth;

	auto drawMapAt = [&](float offsetX)
		{
			sf::RenderStates states;
			states.transform.translate({ offsetX, 0.f });

			float shiftedLeft = left - offsetX;
			float shiftedRight = right - offsetX;
			float shiftedTop = top;
			float shiftedBottom = bottom;

			int minX = (int)(shiftedLeft / TileMetrics::Width) - 2;
			int maxX = (int)(shiftedRight / TileMetrics::Width) + 2;

			int minY = (int)(shiftedTop / (TileMetrics::Height * 0.75f)) - 2;
			int maxY = (int)(shiftedBottom / (TileMetrics::Height * 0.75f)) + 2;

			minX = std::max(0, minX);
			minY = std::max(0, minY);
			maxX = std::min(width - 1, maxX);
			maxY = std::min(height - 1, maxY);

			for (int y = minY; y <= maxY; y++)
			{
				for (int x = minX; x <= maxX; x++)
				{
					Tile& tile = tiles[y * width + x];
					window.draw(tile.getHex(), states);
				}
			}
		};

	drawMapAt(wrapOffsetX);


	float leftEdge = camX - viewWidth * 0.5f;
	float rightEdge = camX + viewWidth * 0.5f;

	float mapLeft = wrapOffsetX;
	float mapRight = wrapOffsetX + worldW;

	if (leftEdge < mapLeft)
		drawMapAt(wrapOffsetX - worldW);

	if (rightEdge > mapRight)
		drawMapAt(wrapOffsetX + worldW);
}

float Map::macroArchipelago(const sf::Vector2f& p, Perlin& n) const
{
	float v = n.octaveNoise(p.x * 0.002f, p.y * 0.002f, 5, 0.5f);

	// crush land into rare islands
	v = std::pow(v, 2.5f);
	return v;
}

float Map::macroContinents(const sf::Vector2f& p, Perlin& n) const
{
	float v = n.octaveNoise(p.x * 0.0008f, p.y * 0.0008f, 4, 0.5f);

	// soften into large landmasses
	v = 0.5f + std::tanh((v - 0.5f) * 3.0f) * 0.5f;
	return v;
}

float Map::macroPangaea(const sf::Vector2f& p, Perlin& n) const
{
	sf::Vector2f center(mapWorldWidth * 0.5f, mapWorldHeight * 0.5f);

	float dx = (p.x - center.x);
	float dy = (p.y - center.y);

	float dist = std::sqrt(dx * dx + dy * dy);
	float radius = std::min(mapWorldWidth, mapWorldHeight) * 0.5f;

	float base = 1.0f - (dist / radius);

	float nval = n.octaveNoise(p.x * 0.001f, p.y * 0.001f, 3, 0.5f);

	return std::clamp(base + (nval - 0.5f) * 0.2f, 0.f, 1.f);
}

float Map::macroFractal(const sf::Vector2f& p, Perlin& n) const
{
	return n.octaveNoise(p.x * 0.003f, p.y * 0.003f, 6, 0.5f);
}

std::vector<Map::ContinentSeed> Map::generateContinentSeeds(int count, unsigned int seed)
{
	std::vector<ContinentSeed> seeds;
	std::mt19937 rng(seed);

	std::uniform_real_distribution<float> xDist(0.f, mapWorldWidth);
	std::uniform_real_distribution<float> yDist(mapWorldHeight * 0.1f, mapWorldHeight * 0.9f);
	std::uniform_real_distribution<float> rDist(mapWorldWidth * 0.10f, mapWorldWidth * 0.20f);

	seeds.reserve(count);
	for (int i = 0; i < count; i++)
	{
		seeds.push_back({ sf::Vector2f(xDist(rng), yDist(rng)), rDist(rng) });
	}

	return seeds;
}

float Map::continentMask(const sf::Vector2f& pos,
	const std::vector<ContinentSeed>& seeds,
	Perlin& noise) const
{
	float best = 0.f;

	for (const ContinentSeed& s : seeds)
	{
		float dx = pos.x - s.pos.x;
		float dy = pos.y - s.pos.y;

		// wrap X
		dx = std::abs(dx);
		dx = std::min(dx, mapWorldWidth - dx);

		float dist = std::sqrt(dx * dx + dy * dy);

		float angle = std::atan2(dy, dx);

		float warp = noise.octaveNoise(
			std::cos(angle) * 2.0f,
			std::sin(angle) * 2.0f,
			4, 0.5f);

		float localRadius = s.radius * (0.65f + warp * 0.7f);

		float falloff = 1.f - (dist / localRadius);
		falloff = std::clamp(falloff, 0.f, 1.f);

		// keep smoothstep (good choice)
		falloff = falloff * falloff * (3.f - 2.f * falloff);

		best = std::max(best, falloff);
	}

	return best;
}

void Map::generateTerrain(WorldType type)
{
	const WorldGenProfile& profile = getWorldProfile(type);
	Perlin noise(time(nullptr));

	std::vector<float> heights;
	heights.reserve(tiles.size());

	auto macro = [&](const sf::Vector2f& p) -> float
		{
			switch (type)
			{
			case WorldType::Archipelago: return macroArchipelago(p, noise);
			case WorldType::Continents:   return macroContinents(p, noise);
			case WorldType::Pangaea:      return macroPangaea(p, noise);
			case WorldType::Fractal:      return macroFractal(p, noise);
			case WorldType::Terra:        return macroContinents(p, noise); // hybrid
			case WorldType::InlandSea:    return 1.0f - macroContinents(p, noise);
			}
			return 0.5f;
		};

	// -------------------------
	// PASS 1: HEIGHT GENERATION
	// -------------------------
	for (Tile& tile : tiles)
	{
		sf::Vector2f pos = tile.getHex().getPosition();

		float h = macro(pos);

		// domain warp (optional but now controlled)
		float warp = noise.octaveNoise(pos.x * 0.0005f, pos.y * 0.0005f, 3, 0.5f);
		float wx = (warp - 0.5f) * profile.warpStrength * 200.f;
		float wy = (warp - 0.5f) * profile.warpStrength * 200.f;

		sf::Vector2f warped = { pos.x + wx, pos.y + wy };

		float detail = noise.octaveNoise(
			warped.x * profile.detailScale,
			warped.y * profile.detailScale,
			5, 0.5f
		);

		h += (detail - 0.5f) * 0.25f;

		h = std::clamp(h, 0.f, 1.f);

		tile.setElevation(h);
		heights.push_back(h);
	}

	// -------------------------
	// PASS 2: SEA LEVELS
	// -------------------------
	std::sort(heights.begin(), heights.end());

	auto percentile = [&](float p)
		{
			return heights[(size_t)(p * (heights.size() - 1))];
		};

	float seaLevel = percentile(profile.landBias);
	float hillLevel = percentile(0.90f);
	float mountainLevel = percentile(0.97f);

	// -------------------------
	// PASS 3: CLASSIFICATION
	// -------------------------
	for (Tile& tile : tiles)
	{
		float h = tile.getElevation();

		Elevation e;

		if (h < seaLevel) e = Elevation::Below_Sea_Level;
		else if (h < hillLevel) e = Elevation::Flat;
		else if (h < mountainLevel) e = Elevation::Hill;
		else e = Elevation::Mountain;

		tile.setElevationType(e);

		switch (e)
		{
		case Elevation::Below_Sea_Level:
			tile.setColor(sf::Color(20, 40, 180));
			break;
		case Elevation::Flat:
			tile.setColor(sf::Color(50, 200, 60));
			break;
		case Elevation::Hill:
			tile.setColor(sf::Color(110, 110, 110));
			break;
		case Elevation::Mountain:
			tile.setColor(sf::Color::White);
			break;
		}
	}

	pruneIsthmuses(seaLevel);
}
void Map::pruneIsthmuses(float seaLevel)
{
	std::vector<Tile*> neighbors;
	std::vector<int> toFlip;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Tile* t = getTile(x, y);
			if (t->getElevationType() == Elevation::Below_Sea_Level)
				continue;

			getNeighbors(x, y, neighbors);

			std::vector<Tile*> landNeighbors;
			for (Tile* n : neighbors)
			{
				if (n->getElevationType() != Elevation::Below_Sea_Level)
					landNeighbors.push_back(n);
			}

			if (landNeighbors.size() == 2)
			{
				std::vector<Tile*> n0Neighbors;
				sf::Vector2i c0 = landNeighbors[0]->getCoords();
				getNeighbors(c0.x, c0.y, n0Neighbors);

				bool adjacent = std::find(n0Neighbors.begin(), n0Neighbors.end(),
					landNeighbors[1]) != n0Neighbors.end();

				if (!adjacent)
					toFlip.push_back(y * width + x);
			}
		}
	}

	for (int idx : toFlip)
	{
		tiles[idx].setElevationType(Elevation::Below_Sea_Level);
		tiles[idx].setColor(sf::Color(20, 40, 180));
	}
}

void Map::updateWrapping(float cameraX)
{

	float worldW = mapWorldWidth;

	float delta = cameraX - wrapOffsetX;

	if (delta > worldW * 0.5f)
		wrapOffsetX += worldW;
	else if (delta < -worldW * 0.5f)
		wrapOffsetX -= worldW;
}

void Map::shiftRight()
{
	wrapOffsetX += mapWorldWidth;
}

void Map::shiftLeft()
{
	wrapOffsetX -= mapWorldWidth;
}

Tile* Map::getTile(int x, int y)
{
	if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;

	return &tiles[y * width + x];
}

Tile* Map::getTileAtPosition(const sf::Vector2f& pos)
{
	Map::CubeCoordF fractional = worldToCube(pos);

	Map::CubeCoord cube = cubeRound(fractional);

	auto [x, y] = cubeToOffset(cube);

	x = (x % width + width) % width;

	return getTile(x, y);
}

static const Map::CubeCoord cubeOffsets[6] = {
	{ 1, -1, 0}, { 1, 0, -1}, { 0, 1, -1},
	{-1, 1, 0}, {-1, 0, 1}, { 0, -1, 1}
};

void Map::getNeighbors(int x, int y, std::vector<Tile*>& out)
{
	out.clear();
	out.reserve(6);

	CubeCoord center = offsetToCube(x, y);

	for (int i = 0; i < 6; i++)
	{
		CubeCoord n{
			center.x + cubeOffsets[i].x,
			center.y + cubeOffsets[i].y,
			center.z + cubeOffsets[i].z
		};

		auto [nx, ny] = cubeToOffset(n);

		if (nx < 0) nx += width;
		if (nx >= width) nx -= width;
		if (Tile* tile = getTile(nx, ny))
		{
			out.push_back(tile);
		}
	}
}

Map::CubeCoord Map::offsetToCube(int x, int y) const
{
	int cubeX = x - (y - (y & 1)) / 2;
	int cubeZ = y;
	int cubeY = -cubeX - cubeZ;
	return { cubeX, cubeY, cubeZ };
}

std::pair<int, int> Map::cubeToOffset(const Map::CubeCoord& c) const
{
	int y = c.z;
	int x = c.x + (y - (y & 1)) / 2;
	return {x, y};
}

Map::CubeCoordF Map::worldToCube(const sf::Vector2f& pos) const
{
	constexpr float SQRT3 = 1.73205080757f;

	float q = (SQRT3 / 3.f * pos.x - 1.f / 3.f * pos.y)
		/ TileMetrics::Size;

	float r = (2.f / 3.f * pos.y)
		/ TileMetrics::Size;

	return {
		q,
		-q - r,
		r
	};
}

Map::CubeCoord Map::cubeRound(const CubeCoordF& cube) const
{
	int rx = std::round(cube.x);
	int ry = std::round(cube.y);
	int rz = std::round(cube.z);

	float dx = std::abs(rx - cube.x);
	float dy = std::abs(ry - cube.y);
	float dz = std::abs(rz - cube.z);

	if (dx > dy && dx > dz)
		rx = -ry - rz;
	else if (dy > dz)
		ry = -rx - rz;
	else
		rz = -rx - ry;

	return { rx, ry, rz };
}