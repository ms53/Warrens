#include "Perlin.h"
#include <random>

namespace
{
    double fade(double t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    double lerp(double a, double b, double t)
    {
        return a + t * (b - a);
    }

    double grad(int hash, double x, double y)
    {
        switch (hash & 7)
        {
        case 0: return  x + y;
        case 1: return -x + y;
        case 2: return  x - y;
        case 3: return -x - y;
        case 4: return  x;
        case 5: return -x;
        case 6: return  y;
        default:return -y;
        }
    }
}

Perlin::Perlin(unsigned int seed)
{
    permutation.resize(256);
    std::iota(permutation.begin(), permutation.end(), 0);

    std::mt19937 rng(seed);
    std::shuffle(permutation.begin(), permutation.end(), rng);

    permutation.insert(
        permutation.end(),
        permutation.begin(),
        permutation.end());
}

double Perlin::noise(double x, double y) const
{
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    double xf = x - std::floor(x);
    double yf = y - std::floor(y);

    double u = fade(xf);
    double v = fade(yf);

    int aa = permutation[permutation[X] + Y];
    int ab = permutation[permutation[X] + Y + 1];
    int ba = permutation[permutation[X + 1] + Y];
    int bb = permutation[permutation[X + 1] + Y + 1];

    double x1 = lerp(
        grad(aa, xf, yf),
        grad(ba, xf - 1.f, yf),
        u);

    double x2 = lerp(
        grad(ab, xf, yf - 1.f),
        grad(bb, xf - 1.f, yf - 1.f),
        u);

    double result = lerp(x1, x2, v);

    return (result + 1.f) * 0.5f;
}

double Perlin::octaveNoise(double x,
    double y,
    int octaves,
    double persistence) const
{
    double total = 0.f;
    double frequency = 1.f;
    double amplitude = 1.f;
    double maxAmplitude = 0.f;

    for (int i = 0; i < octaves; i++)
    {
        total += noise(x * frequency,
            y * frequency) * amplitude;

        maxAmplitude += amplitude;

        amplitude *= persistence;
        frequency *= 2.f;
    }

    return total / maxAmplitude;
}