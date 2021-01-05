#pragma once

class PerlinNoise
{
public:
	PerlinNoise();
	~PerlinNoise();

	static float noise(float x, float y);

	//static int* p() { return get().p; }

private:
	static float fade(float t);
	static float grad(int hash, float x, float y);
	static float lerp(float t, float a, float b);

	static int* p;
	static int permutation[];
};