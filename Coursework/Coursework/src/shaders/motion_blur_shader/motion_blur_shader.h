#pragma once

#include "BaseShader.h"
#include "shaders/shader_utils.h"

using namespace std;
using namespace DirectX;

class MotionBlurShader : public BaseShader
{
public:
	struct ScreenBufferType
	{
		XMFLOAT2 blur_direction;
		XMFLOAT2 screen_size;
		int blur_distance;
		XMFLOAT3 padding;
	};

	MotionBlurShader(ID3D11Device* device, HWND hwnd);
	~MotionBlurShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, gpfw::ShaderInfo s_info, ID3D11ShaderResourceView* texture, XMFLOAT2 blur_direction, XMFLOAT2 screen_size, int blur_distance);

	void Render(gpfw::ShaderInfo s_info, ID3D11ShaderResourceView* texture, OrthoMesh* o_mesh, XMFLOAT2 blur_direction, XMFLOAT2 screen_size, int blur_distance);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* screenBuffer;
	ID3D11SamplerState* sampleState;
};
