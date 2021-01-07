#pragma once

#include "BaseShader.h"
#include "shaders/shader_utils.h"
#include "noise/noise_texture.h"

using namespace std;
using namespace DirectX;

class NoiseShader : public BaseShader
{
public:
	struct ParametersInputType
	{
		XMFLOAT2 octave_offsets_0;
		XMFLOAT2 octave_offsets_1;

		XMFLOAT2 octave_offsets_2;
		XMFLOAT2 octave_offsets_3;

		XMFLOAT2 octave_offsets_4;
		XMFLOAT2 octave_offsets_5;

		XMFLOAT2 octave_offsets_6;
		XMFLOAT2 octave_offsets_7;

		XMFLOAT2 octave_offsets_8;
		XMFLOAT2 octave_offsets_9;
		XMFLOAT2 offset;
		float scale;
		int octaves;

		float persistance;
		float lacunarity;
		XMFLOAT2 falloff;
	};

	NoiseShader(ID3D11Device* device, HWND hwnd);
	~NoiseShader();

	inline ID3D11ShaderResourceView* getPermSRV() { return perm_srv_; }

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, NoiseParameters n_params);

	void Render(gpfw::ShaderInfo s_info, OrthoMesh* o_mesh, NoiseParameters n_params);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* noiseBuffer;

	ID3D11Texture1D* perm_texture_;
	ID3D11ShaderResourceView* perm_srv_;
	ID3D11SamplerState* perm_sampler_;
};

