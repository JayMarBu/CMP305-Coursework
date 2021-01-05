#include "noise_texture.h"
#include "perlin_noise.h"

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

void NoiseTexture::setNoise(ID3D11DeviceContext* context, float n_scale, XMFLOAT2 offset)
{
	XMFLOAT4* Data = new XMFLOAT4[textureWidth * textureHeight]();

	PerlinNoise noise_function;

	for (int i = 0; i < textureWidth; ++i)
	{
		for (int j = 0; j < textureHeight; ++j)
		{
			float in_x = ((float)i / ((float)textureWidth))/n_scale;
			float in_y = ((float)j / ((float)textureHeight))/n_scale;

			float perlin_value = noise_function.noise(in_x+offset.x, in_y+offset.y);

			Data[i + j * textureWidth] = XMFLOAT4(perlin_value, perlin_value, perlin_value, 1);
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
	Data = nullptr;

}

