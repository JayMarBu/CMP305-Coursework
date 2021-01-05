#pragma once
#include "DXF.h"

class NoiseTexture
{
public:
	NoiseTexture(ID3D11Device* device, const unsigned int w, const unsigned int h);

	inline ID3D11ShaderResourceView* getShaderResourceView() { return texture_srv; }

	inline unsigned int getTextureHeight() { return textureHeight; }
	inline unsigned int getTextureWidth() { return textureWidth; }

	void setColour(ID3D11DeviceContext* context, float r, float g, float b, float a);
	void setNoise(ID3D11DeviceContext* context, float n_scale, XMFLOAT2 offset);

private:
	unsigned int textureWidth, textureHeight;
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* texture_srv;
};