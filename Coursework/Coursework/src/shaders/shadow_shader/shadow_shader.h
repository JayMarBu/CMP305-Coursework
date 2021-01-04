#pragma once

#include "DXF.h"
#include "entities/entity.h"
#include "shaders/shader_utils.h"
#include "entities/terrain.h"

using namespace std;
using namespace DirectX;

class ShadowShader : public BaseShader
{
public:

private:

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView[CASCADE_COUNT];
		XMMATRIX lightProjection[CASCADE_COUNT];
	};

	struct LightBufferType
	{
		XMFLOAT4 ambient;
	};

	struct ShadowBufferType
	{
		XMFLOAT4 cascade_depths;
		float near_cp;
		float far_cp;
		BOOL show_cascades;
		float padding;
		XMFLOAT4 shadow_map_size;
	};

	struct CameraBufferType
	{
		XMFLOAT3 camera_position;
		float padding;
	};

public:

	ShadowShader(ID3D11Device* device, HWND hwnd);
	~ShadowShader();

	void setShaderParameters(gpfw::ShaderInfo s_info, gpfw::LightingInfo, gpfw::CascadeInfo);

	void Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo, gpfw::Entity e, gpfw::CascadeInfo);
	void Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo, gpfw::TerrainChunk t, gpfw::CascadeInfo);

private:

	void initShader(const wchar_t* cs, const wchar_t* ps);

private:
	ID3D11Buffer* matrix_buffer_;
	ID3D11SamplerState* sample_state_shadow_;
	ID3D11Buffer* light_buffer_;
	ID3D11Buffer* shadow_buffer_;
	ID3D11Buffer* camera_buffer_;
};