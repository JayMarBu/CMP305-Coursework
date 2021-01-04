#pragma once

#include "DXF.h"
#include "shaders/shader_utils.h"

using namespace std;
using namespace DirectX;

class GenerateTerrainCShader : public BaseShader
{
public:

	// vertex data structure
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 tex_coords;
		XMFLOAT3 normals;
	};

	// data to be stored in structured buffer
	struct TerrainParametersType
	{
		XMFLOAT2 data_min;
		XMFLOAT2 data_max;
	};

	// CONSTRUCTOR & DECONSTRUCTOR ..................................................................................................................
	GenerateTerrainCShader(ID3D11Device* device, HWND hwnd, const unsigned int count);
	~GenerateTerrainCShader();

	// SHADER DISPATCH METHODS ......................................................................................................................
	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* in_data, ID3D11ShaderResourceView* height_map, TerrainParametersType min_max);

	void unbind(ID3D11DeviceContext* dc);

	void RunComputeShader(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* in_data, ID3D11ShaderResourceView* height_map, XMFLOAT2 data_min, XMFLOAT2 data_max, int x, int y, int z);

	// GETTER METHODS ...............................................................................................................................
	inline ID3D11Buffer* getVertexOutBuffer() { return vertex_out_buffer_; }
	inline ID3D11UnorderedAccessView* getVertexOutUAV() { return vertex_out_uav_; }
private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	ID3D11Buffer* vertex_out_buffer_;
	ID3D11Buffer* terrain_parameters_buffer_;
	ID3D11UnorderedAccessView* vertex_out_uav_;
	ID3D11SamplerState* heightmap_sampler_;

	const unsigned int v_count_;
};
