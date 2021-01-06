#pragma once

#include "BaseShader.h"
#include "shaders/shader_utils.h"

using namespace std;
using namespace DirectX;

class NoiseShader : public BaseShader
{
public:
	struct ParametersInputType
	{
		XMFLOAT2 offset;
		float scale;
		float padding;
	};

	NoiseShader(ID3D11Device* device, HWND hwnd);
	~NoiseShader();

	inline ID3D11ShaderResourceView* getPermSRV() { return perm_srv_; }
	inline ID3D11ShaderResourceView* getGradSRV() { return grad_srv_; }

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, float scale, XMFLOAT2 offset);

	void Render(gpfw::ShaderInfo s_info, OrthoMesh* o_mesh, float scale, XMFLOAT2 offset);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* noiseBuffer;

	ID3D11Texture1D* perm_texture_;
	ID3D11ShaderResourceView* perm_srv_;
	ID3D11SamplerState* perm_sampler_;

	ID3D11Texture1D* grad_texture_;
	ID3D11ShaderResourceView* grad_srv_;
	ID3D11SamplerState* grad_sampler_;
};

