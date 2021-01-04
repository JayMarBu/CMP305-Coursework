#pragma once
#include "DXF.h"
#include "entities/entity.h"
#include "shaders/shader_utils.h"
#include "entities/terrain.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{

public:

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();

	void setShaderParameters(gpfw::ShaderInfo s_info, XMFLOAT4 clip_plane = XMFLOAT4(0, 0, 0, 0));

	void Render(gpfw::ShaderInfo s_info, gpfw::Entity e, XMFLOAT4 clip_plane = XMFLOAT4(0,0,0,0));
	void Render(gpfw::ShaderInfo s_info, gpfw::TerrainChunk t, XMFLOAT4 clip_plane = XMFLOAT4(0, 0, 0, 0));

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* clipPlaneBuffer;
};