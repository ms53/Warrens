#pragma once
#include <iostream>
#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>

class Perlin {
public:
	explicit Perlin(unsigned int seed);

	double noise(double x, double y) const;

	double octaveNoise(double x, double y, int octaves, double persistence) const;
private:
	std::vector<int> permutation;
};
