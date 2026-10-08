#include <SFML/Graphics.hpp>
#include "Map.h"
#include <iostream>
#include "Perlin.h"
#include <random>
#include <queue>
Map::Map(MapSize size, WorldType type)
{
	mapSize = size;
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
	generateTerrain(type);
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
	float v = n.octaveNoise(p.x * 0.0008f, p.y * 0.0008f, 4, 0.5f);

	// soften into large landmasses
	v = 0.5f + std::tanh((v - 0.5f) * 3.0f) * 0.5f;
	return v;
}

float Map::macroInlandSea(const sf::Vector2f& p, Perlin& noise) const
{
	const sf::Vector2f center(
		mapWorldWidth * 0.5f,
		mapWorldHeight * 0.5f
	);

	// Size of the inland sea.
	// Adjust these independently to make it wider/narrower.
	const float radiusX = mapWorldWidth * 0.4f;
	const float radiusY = mapWorldHeight * 0.4f;

	float dx = (p.x - center.x) / radiusX;
	float dy = (p.y - center.y) / radiusY;

	// 1 at centre, 0 at the nominal coastline.
	float distance = std::sqrt(dx * dx + dy * dy);
	float basin = distance - 1.0f;

	// Low-frequency noise makes the coastline irregular.
	float coastNoise =
		noise.octaveNoise(
			p.x * 0.0012f,
			p.y * 0.0012f,
			3,
			0.5f
		);

	basin += (coastNoise - 0.5f) * 0.30f;

	// Convert to a land/water value.
	//
	// > 0 = land
	// < 0 = water
	return basin;
}


float Map::continentalFractal(const sf::Vector2f& p, Perlin& noise) const
{

	float continentScale = 3.0f / mapWorldWidth; // NOTE!!! NUMERATOR MAY NEED TO BE ADJUSTED (IE. continentScale < 0.001f)
	float large =
		noise.octaveNoise(
			p.x * continentScale,
			p.y * continentScale,
			5,
			0.55f
		);


	float medium =
		noise.octaveNoise(
			p.x * 0.00065f,
			p.y * 0.00065f,
			4,
			0.5f
		);


	float detail =
		noise.octaveNoise(
			p.x * 0.0035f,
			p.y * 0.0035f,
			3,
			0.5f
		);


	// Large shapes dominate
	float result =
		large * 0.65f +
		medium * 0.25f +
		detail * 0.10f;


	return result;
}

float Map::archipelagoFractal(const sf::Vector2f& p, Perlin& noise) const
{
	float archipelagoScale = 0.001f / mapWorldWidth; // NOTE!!! NUMERATOR MAY NEED TO BE ADJUSTED (IE. archipelagoScale < 0.001f)

	float large =
		noise.octaveNoise(
			p.x * archipelagoScale,
			p.y * archipelagoScale,
			4,
			0.55f
		);

	float medium =
		noise.octaveNoise(
			p.x * 0.00075f,
			p.y * 0.00075f,
			3,
			0.5f
		);
	float detail =
		noise.octaveNoise(
			p.x * 0.0035f,
			p.y * 0.0035f,
			2,
			0.5f
		);
	float result =
		large * 0.65f +
		medium * 0.25f +
		detail * 0.10f;


	return result;
}
float Map::fractalFractal(const sf::Vector2f& p, Perlin& noise) const {
	float fractalScale = 0.1f / mapWorldWidth; // NOTE!!! NUMERATOR MAY NEED TO BE ADJUSTED (IE. archipelagoScale < 0.001f)

	float large =
		noise.octaveNoise(
			p.x * fractalScale,
			p.y * fractalScale,
			5,
			0.55f
		);


	float medium =
		noise.octaveNoise(
			p.x * 0.00065f,
			p.y * 0.00065f,
			4,
			0.5f
		);


	float detail =
		noise.octaveNoise(
			p.x * 0.0035f,
			p.y * 0.0035f,
			3,
			0.5f
		);


	// Large shapes dominate
	float result =
		large * 0.55f +
		medium * 0.25f +
		detail * 0.20f;


	return result;
}


sf::Vector2f Map::warpPosition(
	const sf::Vector2f& p,
	Perlin& noise
) const
{
	float wx =
		(noise.octaveNoise(
			p.x * 0.0008f,
			p.y * 0.0008f,
			3,
			0.5f
		) - 0.5f) * 250.f;


	float wy =
		(noise.octaveNoise(
			(p.x + 5000.f) * 0.0008f,
			(p.y + 5000.f) * 0.0008f,
			3,
			0.5f
		) - 0.5f) * 250.f;


	return {
		p.x + wx,
		p.y + wy
	};
}

void Map::generateTerrain(WorldType type)
{
	


	const WorldGenProfile& profile = getWorldProfile(type);
	

	
	// =====================================================
	// CONTINENT GENERATION
	// =====================================================
	bool condition = true;
	bool foundValidWorld = false;
	int attempts = 0;
	while (true) {
		Perlin noise(time(nullptr));
		std::vector<float> heights;
		heights.reserve(tiles.size());
		float maxH = 0.f;
		float minH = 1.f;
		float DPMean = 0.f, SHMean = 0.f, FLMean = 0.f, HLMean = 0.f, MTMean = 0.f;
		float DPCount = 0, SHCount = 0, FLCount = 0, HLCount = 0, MTCount = 0;
		float DPMinCount = 0, SHMinCount = 0, FLMinCount = 0, HLMinCount = 0, MTMinCount = 0;
		float DPMaxCount = 0, SHMaxCount = 0, FLMaxCount = 0, HLMaxCount = 0, MTMaxCount = 0;
		if (type == WorldType::Continents || type == WorldType::Pangaea || type == WorldType::Terra)
		{
			float seaLevel = 0.4f;

			for (Tile& tile : tiles)
			{
				sf::Vector2f pos =
					tile.getHex().getPosition();


				// distort coordinates
				sf::Vector2f warped =
					warpPosition(pos, noise);


				// ===================================
				// Continental scale
				// ===================================

				float continent =
					continentalFractal(
						warped,
						noise
					);


				// make continents larger
				continent =
					std::pow(
						continent,
						1.2f //Originally 1.35f
					);


				// ===================================
				// latitude effect
				// ===================================

				float latitude =
					std::abs(
						pos.y / mapWorldHeight - 0.5f
					);


				continent -= latitude * 0.12f;







				// ===================================
				// land / ocean
				// ===================================

				if (continent > seaLevel)
				{

					float elevationLarge =
						noise.octaveNoise(
							warped.x * 0.00075f,
							warped.y * 0.00075f,
							3,
							0.55f
						);

					float elevationMedium =
						noise.octaveNoise(
							warped.x * 0.00015f,
							warped.y * 0.00015f,
							3,
							0.5f
						);

					float elevationDetail =
						noise.octaveNoise(
							warped.x * 0.0030f,
							warped.y * 0.0030f,
							1,
							0.5f
						);

					float elevation =
						elevationLarge * 0.70f +
						elevationMedium * 0.25f +
						elevationDetail * 0.05f;


					// make mountains rarer
					elevation =
						std::pow(
							elevation,
							1.45f
						);


					// stretch range
					elevation *= 1.45f;


					elevation =
						std::clamp(
							elevation,
							0.f,
							1.f
						);

					float mountainField =
						noise.octaveNoise(
							warped.x * 0.0007f,
							warped.y * 0.0007f,
							3,
							0.5f
						);

					elevation += mountainField * 0.1f;

					tile.setElevation(elevation);

					heights.push_back(elevation);
				}
				else
				{
					// Ocean:
					float depth = (seaLevel - continent) / seaLevel;

					// Map ocean to 0.0 - 0.4
					float oceanElevation = depth * 0.4f;

					tile.setElevation(-oceanElevation);
				}
			}
		}

		else if (type == WorldType::Archipelago) {
			float seaLevel = 0.4f;
			for (Tile& tile : tiles)
			{
				sf::Vector2f pos =
					tile.getHex().getPosition();


				// distort coordinates
				sf::Vector2f warped =
					warpPosition(pos, noise);


				// ===================================
				// Continental scale
				// ===================================

				float continent =
					archipelagoFractal(
						warped,
						noise
					);


				// make continents larger
				continent =
					std::pow(
						continent,
						1.3f //Originally 1.35f
					);


				// ===================================
				// latitude effect
				// ===================================

				float latitude =
					std::abs(
						pos.y / mapWorldHeight - 0.5f
					);


				continent -= latitude * 0.12f;

				// ===================================
				// land / ocean
				// ===================================

				if (continent > seaLevel)
				{

					float elevationLarge =
						noise.octaveNoise(
							warped.x * 0.00075f,
							warped.y * 0.00075f,
							3,
							0.55f
						);

					float elevationMedium =
						noise.octaveNoise(
							warped.x * 0.00015f,
							warped.y * 0.00015f,
							3,
							0.5f
						);

					float elevationDetail =
						noise.octaveNoise(
							warped.x * 0.0030f,
							warped.y * 0.0030f,
							1,
							0.5f
						);

					float elevation =
						elevationLarge * 0.70f +
						elevationMedium * 0.25f +
						elevationDetail * 0.05f;


					// make mountains rarer
					elevation =
						std::pow(
							elevation,
							1.7f
						);


					// stretch range
					elevation *= 1.45f;


					elevation =
						std::clamp(
							elevation,
							0.f,
							1.f
						);

					float mountainField =
						noise.octaveNoise(
							warped.x * 0.0007f,
							warped.y * 0.0007f,
							3,
							0.5f
						);

					elevation += mountainField * 0.1f;

					tile.setElevation(elevation);

					heights.push_back(elevation);
				}
				else
				{
					// Ocean:
					float depth = (seaLevel - continent) / seaLevel;

					// Map ocean to 0.0 - 0.4
					float oceanElevation = depth * 0.4f;

					tile.setElevation(-oceanElevation);
				}
			}
		}

		else if (type == WorldType::Fractal) {
			float seaLevel = 0.4f;

			for (Tile& tile : tiles)
			{
				sf::Vector2f pos =
					tile.getHex().getPosition();


				// distort coordinates
				sf::Vector2f warped =
					warpPosition(pos, noise);


				// ===================================
				// Continental scale
				// ===================================

				float continent =
					fractalFractal(
						warped,
						noise
					);


				// make continents larger
				continent =
					std::pow(
						continent,
						1.2f //Originally 1.35f
					);


				// ===================================
				// latitude effect
				// ===================================

				float latitude =
					std::abs(
						pos.y / mapWorldHeight - 0.5f
					);


				continent -= latitude * 0.12f;







				// ===================================
				// land / ocean
				// ===================================

				if (continent > seaLevel)
				{

					float elevationLarge =
						noise.octaveNoise(
							warped.x * 0.00075f,
							warped.y * 0.00075f,
							3,
							0.55f
						);

					float elevationMedium =
						noise.octaveNoise(
							warped.x * 0.00015f,
							warped.y * 0.00015f,
							3,
							0.5f
						);

					float elevationDetail =
						noise.octaveNoise(
							warped.x * 0.0030f,
							warped.y * 0.0030f,
							1,
							0.5f
						);

					float elevation =
						elevationLarge * 0.70f +
						elevationMedium * 0.25f +
						elevationDetail * 0.05f;


					// make mountains rarer
					elevation =
						std::pow(
							elevation,
							1.45f
						);


					// stretch range
					elevation *= 1.45f;


					elevation =
						std::clamp(
							elevation,
							0.f,
							1.f
						);

					float mountainField =
						noise.octaveNoise(
							warped.x * 0.0007f,
							warped.y * 0.0007f,
							3,
							0.5f
						);

					elevation += mountainField * 0.1f;

					tile.setElevation(elevation);

					heights.push_back(elevation);
				}
				else
				{
					// Ocean:
					float depth = (seaLevel - continent) / seaLevel;

					// Map ocean to 0.0 - 0.4
					float oceanElevation = depth * 0.4f;

					tile.setElevation(-oceanElevation);
				}
			}
		}

		else if (type == WorldType::InlandSea) {
			for (Tile& tile : tiles)
			{
				sf::Vector2f pos = tile.getHex().getPosition();

				sf::Vector2f warped = warpPosition(pos, noise);

				float basin = macroInlandSea(warped, noise);

				if (basin < 0.0f)
				{
					float depth = std::clamp(-basin, 0.0f, 1.0f);
					float oceanElevation = depth * 0.4f;
					tile.setElevation(-oceanElevation);
				}
				else {
					float elevationLarge =
						noise.octaveNoise(
							warped.x * 0.00075f,
							warped.y * 0.00075f,
							3,
							0.55f
						);
					float elevationMedium =
						noise.octaveNoise(
							warped.x * 0.00015f,
							warped.y * 0.00015f,
							3,
							0.5f
						);
					float elevationDetail =
						noise.octaveNoise(
							warped.x * 0.0030f,
							warped.y * 0.0030f,
							1,
							0.5f
						);
					float elevation =
						elevationLarge * 0.70f +
						elevationMedium * 0.25f +
						elevationDetail * 0.05f;
					elevation = std::pow(elevation, 1.45f);
					elevation *= 1.45f;
					elevation = std::clamp(elevation, 0.f, 1.f);
					float mountainField =
						noise.octaveNoise(
							warped.x * 0.0007f,
							warped.y * 0.0007f,
							3,
							0.5f
						);
					elevation += mountainField * 0.1f;
					tile.setElevation(elevation);
				}
				heights.push_back(tile.getElevation());
			}
		}
		// =====================================================
		// NORMAL PERLIN WORLDS
		// =====================================================

		else
		{
			auto macro = [&](const sf::Vector2f& p) -> float
				{
					switch (type)
					{
					case WorldType::Archipelago:
						//return macroArchipelago(p, noise);

					case WorldType::Continents:
						//return macroContinents(p, noise);

					case WorldType::Pangaea:
						//return macroPangaea(p, noise);

					case WorldType::Fractal:
						//return macroFractal(p, noise);

					case WorldType::Terra:
						//return macroContinents(p, noise);

					case WorldType::InlandSea:
						return 1.0f - macroContinents(p, noise);
					}

					return 0.5f;
				};


			for (Tile& tile : tiles)
			{
				sf::Vector2f pos =
					tile.getHex().getPosition();


				float h =
					macro(pos);


				float warp =
					noise.octaveNoise(
						pos.x * 0.0005f,
						pos.y * 0.0005f,
						3,
						0.5f
					);


				float wx =
					(warp - 0.5f) *
					profile.warpStrength *
					200.f;

				float wy =
					(warp - 0.5f) *
					profile.warpStrength *
					200.f;


				sf::Vector2f warped =
				{
					pos.x + wx,
					pos.y + wy
				};


				float detail =
					noise.octaveNoise(
						warped.x * profile.detailScale,
						warped.y * profile.detailScale,
						5,
						0.5f
					);


				h +=
					(detail - 0.5f) *
					0.25f;


				h =
					std::clamp(
						h,
						0.f,
						1.f
					);


				tile.setElevation(h);

				heights.push_back(h);
			}
		}



		// =====================================================
		// CLASSIFICATION
		// =====================================================



		for (Tile& tile : tiles)
		{
			float h =
				tile.getElevation();
			if (h > maxH) maxH = h;
			if (h < minH) minH = h;
			Elevation e;


			if (h <= -0.0375f)
			{
				e = Elevation::Deep_Sea;
				DPCount++;
				DPMean += h;
				if (h > DPMaxCount) DPMaxCount = h;
				if (h < DPMinCount) DPMinCount = h;
			}
			else if (h < 0.0f)
			{
				e = Elevation::Shallow_Sea;
				SHCount++;
				SHMean += h;
				if (h > SHMaxCount) SHMaxCount = h;
				if (h < SHMinCount) SHMinCount = h;
			}
			else if (h < 0.65f)
			{
				e = Elevation::Flat;
				FLCount++;
				FLMean += h;
				if (h > FLMaxCount) FLMaxCount = h;
				if (h < FLMinCount) FLMinCount = h;
			}
			else if (h < 0.75f)
			{
				e = Elevation::Hill;
				HLCount++;
				HLMean += h;
				if (h > HLMaxCount) HLMaxCount = h;
				if (h < HLMinCount) HLMinCount = h;
			}
			else
			{
				e = Elevation::Mountain;
				MTCount++;
				MTMean += h;
				if (h > MTMaxCount) MTMaxCount = h;
				if (h < MTMinCount) MTMinCount = h;
			}


			tile.setElevationType(e);


			switch (e)
			{
			case Elevation::Shallow_Sea:
				tile.setColor(sf::Color(60, 80, 220));
				break;
			case Elevation::Deep_Sea:
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
		std::vector<std::vector<Tile*>> landmasses = findLandmasses();
		float maxLandmassSize = 0;
		std::vector<Tile*> maxLandmass;
		for (std::vector<Tile*> landmass : landmasses)
		{
			if (landmass.size() > maxLandmassSize) {
				maxLandmassSize = landmass.size();
				maxLandmass = landmass;
			}
		}
		float coastSize = 0;
		for (Tile* tile : maxLandmass) {
			std::vector<Tile*> neighbors;
			getNeighbors(tile->getX(), tile->getY(), neighbors);
			for (Tile* neighbor : neighbors) {
				if (neighbor->getElevationType() == Elevation::Deep_Sea || neighbor->getElevationType() == Elevation::Shallow_Sea) {
					coastSize++;
					break;
				}
			}
		}
		float landPercentage = ((FLCount + HLCount + MTCount) / (DPCount + SHCount + FLCount + HLCount + MTCount));
		float biggestLandmassPercentage = (maxLandmassSize / (FLCount + HLCount + MTCount));
		float biggestLandmassCoastPercentage = coastSize / maxLandmassSize;
		switch (type) {
		case WorldType::Continents:
		case WorldType::Terra: condition = landPercentage >= 0.35f && landPercentage <= 0.5f && biggestLandmassPercentage <= 0.6f;
			break;
		case WorldType::Archipelago: condition = biggestLandmassPercentage <= 0.25f;
			break;
		case WorldType::Pangaea: condition = biggestLandmassPercentage >= 0.9f && landPercentage >= 0.35f;
			break;
		case WorldType::Fractal: condition = landPercentage >= 0.35f && landPercentage <= 0.5f;
		}
		if (FLCount > 0 && condition ) {
			std::cout << "Min Height: " << minH << ", Max Height: " << maxH;
			std::cout << "\nDeep Sea Count: " << DPCount << ", Mean: " << (DPCount > 0 ? DPMean / DPCount : 0.f) << " Min: " << DPMinCount << " Max: " << DPMaxCount;
			std::cout << "\nShallow Sea Count: " << SHCount << ", Mean: " << (SHCount > 0 ? SHMean / SHCount : 0.f) << " Min: " << SHMinCount << " Max: " << SHMaxCount;
			std::cout << "\nFlat Count: " << FLCount << ", Mean: " << (FLCount > 0 ? FLMean / FLCount : 0.f) << " Min: " << FLMinCount << " Max: " << FLMaxCount;
			std::cout << "\nHill Count: " << HLCount << ", Mean: " << (HLCount > 0 ? HLMean / HLCount : 0.f) << " Min: " << HLMinCount << " Max: " << HLMaxCount;
			std::cout << "\nMountain Count: " << MTCount << ", Mean: " << (MTCount > 0 ? MTMean / MTCount : 0.f) << " Min: " << MTMinCount << " Max: " << MTMaxCount;
			std::cout << "\nLandmass Count: " << landmasses.size();
			std::cout << "\nLand as percentage of whole world: " << landPercentage * 100 << "%";
			std::cout << "\nBiggest landmass as percentage of all land: " << (maxLandmassSize / (FLCount + HLCount + MTCount)) * 100 << "%";
			std::cout << "\nCoast percentage of biggest landmass: " << biggestLandmassCoastPercentage * 100 << "%";
			std::cout << "\nAttempts: " << attempts;
			
			foundValidWorld = true;
			break;
		}
		attempts++;
	}
	if (!foundValidWorld) throw std::runtime_error("Failed to generate valid world terrain");

}
std::vector<std::vector<Tile*>> Map::findLandmasses() {
	std::vector<std::vector<Tile*>> landmasses;

	std::vector<bool> visited(tiles.size(), false);

	std::vector<Tile*> neighbors;
	neighbors.reserve(6);

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int index = y * width + x;

			if (visited[index])
				continue;
			Tile& start = tiles[index];

			if (!isLand(start))
			{
				visited[index] = true;
				continue;
			}

			std::vector<Tile*> landmass;
			std::queue<Tile*> queue;

			visited[index] = true;
			queue.push(&start);

			while (!queue.empty())
			{
				Tile* current = queue.front();
				queue.pop();

				landmass.push_back(current);
				getNeighbors(current->getX(), current->getY(), neighbors);

				for (Tile* neighbor : neighbors)
				{
					if (neighbor == nullptr)
						continue;
					int neighborIndex = neighbor->getY() * width + neighbor->getX();

					if (visited[neighborIndex])
						continue;
					if (!isLand(*neighbor))
						continue;
					visited[neighborIndex] = true;
					queue.push(neighbor);
				}
			}
			landmasses.push_back(std::move(landmass));
		}
	}

	return landmasses;
}

bool Map::isLand(const Tile& tile)
{
	Elevation e = tile.getElevationType();

	return e == Elevation::Flat ||
		e == Elevation::Hill ||
		e == Elevation::Mountain;
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