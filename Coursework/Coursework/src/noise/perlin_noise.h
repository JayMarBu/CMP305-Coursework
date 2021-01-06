#pragma once

// Improved Perlin Noise Singleton class
class PerlinNoise
{
public:
	PerlinNoise() {}
	PerlinNoise(PerlinNoise&) = delete;
	~PerlinNoise();

	static void init();
	static PerlinNoise& get();

	static float noise(float x, float y);

private:
	static float fade(float t);
	static float grad(int hash, float x, float y);
	static float lerp(float t, float a, float b);

	static int* p;
	static int permutation[];
};