#pragma once

#include "DXF.h"
#include "entities/entity.h"
#include "entities/terrain.h"
#include "shaders/shader_utils.h"

using namespace std;
using namespace DirectX;

class LightShader : public BaseShader
{
public:

private:

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 dir_spec_pow;
		XMFLOAT4 specular_colour;
	};

	struct CameraBufferType
	{
		XMFLOAT3 camera_position;
		float padding;
	};

public:

	LightShader(ID3D11Device* device, HWND hwnd);
	~LightShader();

	void setShaderParameters(gpfw::ShaderInfo s_info, ID3D11ShaderResourceView* texture, gpfw::LightingInfo, ID3D11ShaderResourceView* shadow_map, XMFLOAT4 clip_plane = XMFLOAT4(0,0,0,0));

	void Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo, gpfw::Entity e, ID3D11ShaderResourceView* shadow_map, XMFLOAT4 clip_plane = XMFLOAT4(0, 0, 0, 0));
	void Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo, gpfw::TerrainChunk t, ID3D11ShaderResourceView* shadow_map, XMFLOAT4 clip_plane = XMFLOAT4(0, 0, 0, 0));

private:

	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrix_buffer_;
	ID3D11SamplerState* sample_state_;
	ID3D11SamplerState* sample_state_shadow_;
	ID3D11Buffer* light_buffer_;
	ID3D11Buffer* camera_buffer_;
	ID3D11Buffer* clip_plane_buffer_;
};