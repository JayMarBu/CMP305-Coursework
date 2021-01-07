#include "noise_texture.h"
#include "perlin_noise.h"
#include <random>

NoiseTexture::NoiseTexture(ID3D11Device* device, const unsigned int w, const unsigned int h)
	: texture(nullptr), texture_srv(nullptr)
{
	textureWidth = w;
	textureHeight = h;

	D3D11_TEXTURE2D_DESC tex_desc;
	HRESULT result;
	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;

	ZeroMemory(&tex_desc, sizeof(tex_desc));

	// Setup the texture description.
	tex_desc.Width = textureWidth;
	tex_desc.Height = textureHeight;
	tex_desc.MipLevels = 0;
	tex_desc.ArraySize = 1;
	//tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tex_desc.SampleDesc.Count = 1;
	//tex_desc.Usage = D3D11_USAGE_DYNAMIC;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.MiscFlags = 0;
	// Create the render target texture.
	result = device->CreateTexture2D(&tex_desc, NULL, &texture);

	// Setup the description of the shader resource view.
	srv_desc.Format = tex_desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;
	// Create the shader resource view.
	result = device->CreateShaderResourceView(texture, &srv_desc, &texture_srv);
}

void NoiseTexture::setColour(ID3D11DeviceContext* context, float r, float g, float b, float a)
{
	XMFLOAT4* Data = new XMFLOAT4[textureWidth * textureHeight]();

	for (int i = 0; i < textureWidth; ++i)
	{
		for (int j = 0; j < textureHeight; ++j)
		{
			Data[i + j * textureWidth] = XMFLOAT4(r, g, b, a);
		}
	}

	UINT const DataSize = sizeof(XMFLOAT4);
	UINT const RowPitch = DataSize * textureWidth;
	UINT const DepthPitch = DataSize * textureWidth * textureHeight;

	D3D11_BOX Box;
	Box.left = 0;
	Box.right = textureWidth;
	Box.top = 0;
	Box.bottom = textureHeight;
	Box.front = 0;
	Box.back = 1;

	context->UpdateSubresource(texture, 0, &Box, Data, RowPitch, DepthPitch);
	delete[] Data;
}

void NoiseTexture::setNoise(ID3D11DeviceContext* context, NoiseParameters np)
{
	// create data buffer
	XMFLOAT4* Data = new XMFLOAT4[textureWidth * textureHeight]();

	float max_noise_height = -1000;
	float min_noise_height = 1000;

	// set each pixel to the appropriate colour using Perlin noise
	for (int i = 0; i < textureWidth; ++i)
	{
		for (int j = 0; j < textureHeight; ++j)
		{
			float amplitude = 1;
			float freaquency = 1;

			float noise_height = 0;

			for (int k = 0; k < np.octaves; k++)
			{
				float sample_x = ((float)i / ((float)textureWidth ))/ np.scale * freaquency + np.octave_offsets[k].x;
				float sample_y = ((float)j / ((float)textureHeight))/ np.scale * freaquency + np.octave_offsets[k].y;

				float perlin_value = PerlinNoise::noise(sample_x + np.offset[0], sample_y + np.offset[1])*2-1;

				noise_height += perlin_value * amplitude;
				amplitude *= np.persistance;
				freaquency *= np.lacunarity;
			}

			max_noise_height = (max_noise_height < noise_height) ? noise_height : max_noise_height;
			min_noise_height = (min_noise_height > noise_height) ? noise_height : min_noise_height;

			Data[i + j * textureWidth] = XMFLOAT4(noise_height, noise_height, noise_height, 1);
		}
	}

	for (int i = 0; i < textureWidth; ++i)
	{
		for (int j = 0; j < textureHeight; ++j)
		{
			float noise_height = normaliseHeight(Data[i + j * textureWidth].x, min_noise_height, max_noise_height);

			Data[i + j * textureWidth] = XMFLOAT4(noise_height, noise_height, noise_height, 1);
		}
	}

	// update the texture with new colours
	UINT const DataSize = sizeof(XMFLOAT4);
	UINT const RowPitch = DataSize * textureWidth;
	UINT const DepthPitch = DataSize * textureWidth * textureHeight;

	D3D11_BOX Box;
	Box.left = 0;
	Box.right = textureWidth;
	Box.top = 0;
	Box.bottom = textureHeight;
	Box.front = 0;
	Box.back = 1;

	context->UpdateSubresource(texture, 0, &Box, Data, RowPitch, DepthPitch);

	delete[] Data;
	Data = nullptr;

}

float NoiseTexture::normaliseHeight(float data, float data_min, float data_max)
{
	return (data - data_min) / (data_max - data_min);
}

