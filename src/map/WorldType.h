enum class WorldType {
	Archipelago,
	Continents,
	Terra,
	Pangaea,
	InlandSea,
	Fractal
};

struct WorldGenProfile {
	float continentScale;
	float detailScale;
	float warpStrength;
	float landBias;
	int continentSeedCount;
};

constexpr WorldGenProfile WORLD_PROFILES[] = {
    // Archipelago
    {
        0.0015f,  // continentScale
        0.010f,   // detailScale
        0.60f,    // warpStrength
        0.45f,    // landBias
        10        // continentSeedCount
    },

    // Continents
    {
        0.0008f,
        0.005f,
        0.25f,
        0.55f,
        4
    },

    // Terra
    {
        0.0007f,
        0.006f,
        0.30f,
        0.58f,
        6
    },

    // Pangaea
    {
        0.0004f,
        0.003f,
        0.10f,
        0.62f,
        1
    },

    // InlandSea
    {
        0.0008f,
        0.005f,
        0.25f,
        0.55f,
        4
    },

    // Fractal
    {
        0.0020f,
        0.020f,
        0.70f,
        0.50f,
        0
    }
};
inline const WorldGenProfile& getWorldProfile(WorldType type)
{
    return WORLD_PROFILES[static_cast<int>(type)];
}