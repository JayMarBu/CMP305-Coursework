#pragma once

#include "BaseShader.h"
#include "shaders/shader_utils.h"
#include "entities/entity.h"

using namespace std;
using namespace DirectX;

class WaterShader : public BaseShader
{
public:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct WaterBufferVSType
	{
		float tiling;
		XMFLOAT3 eye_pos;
		XMFLOAT3 light_pos;
		float padding;
	};
	
	struct WaterBufferPSType
	{
		float wave_strength;
		XMFLOAT2 move_factor;
		float colour_intensity;
		float reflection_strength;
		XMFLOAT3 water_colour;
	};

	struct SceneBufferType
	{
		XMFLOAT4 colour;
		float specular_power;
		XMFLOAT3 padding;
	};

	WaterShader(ID3D11Device* device, HWND hwnd);
	~WaterShader();

	void setShaderParameters(
		gpfw::ShaderInfo s_info, gpfw::WaterInfo w_info, gpfw::LightingInfo l_info,
		ID3D11ShaderResourceView* reflection, ID3D11ShaderResourceView* refraction, ID3D11ShaderResourceView* dudv, ID3D11ShaderResourceView* normal_map
	);

	void Render(gpfw::ShaderInfo s_info, gpfw::WaterInfo w_info, gpfw::LightingInfo l_info);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* waterVSBuffer;
	ID3D11Buffer* waterPSBuffer;
	ID3D11Buffer* sceneBuffer;
	ID3D11SamplerState* sampleState;
};

#pragma once
