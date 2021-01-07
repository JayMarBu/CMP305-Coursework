#pragma once
#include "DXF.h"

struct NoiseParameters
{
	float debug_colours[3]		= { 0,0,0 };
	float offset[2]				= { 0,0 };
	float scale					= 1.0f;
	int octaves					= 1;
	XMFLOAT2 octave_offsets[10] = { XMFLOAT2(0,0) };
	float persistance			= 0.5f;
	float lacunarity			= 2.0f;
	int seed					= 1800892;
	float falloff[2]			= { 1,1 };
};

class NoiseTexture
{
public:
	NoiseTexture(ID3D11Device* device, const unsigned int w, const unsigned int h);

	inline ID3D11ShaderResourceView* getShaderResourceView() { return texture_srv; }

	inline unsigned int getTextureHeight() { return textureHeight; }
	inline unsigned int getTextureWidth() { return textureWidth; }

	void setColour(ID3D11DeviceContext* context, float r, float g, float b, float a);
	void setNoise(ID3D11DeviceContext* context, NoiseParameters noise_params);

private:

	float normaliseHeight(float data, float data_min, float data_max);

	unsigned int textureWidth, textureHeight;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* texture_srv;
};