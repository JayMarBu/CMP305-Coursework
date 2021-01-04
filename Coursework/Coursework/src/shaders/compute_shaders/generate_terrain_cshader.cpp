#include "generate_terrain_cshader.h"
// CONSTRUCTOR & DECONSTRUCTOR ..................................................................................................................
GenerateTerrainCShader::GenerateTerrainCShader(ID3D11Device* device, HWND hwnd, const unsigned int count) 
	: BaseShader(device, hwnd), v_count_(count)
{
	initShader(L"res/shaders/generate_terrain_cs.cso", NULL);
}

GenerateTerrainCShader::~GenerateTerrainCShader()
{
	if (!vertex_out_buffer_)
	{
		delete vertex_out_buffer_;
		vertex_out_buffer_ = nullptr;
	}

	if (!terrain_parameters_buffer_)
	{
		delete terrain_parameters_buffer_;
		terrain_parameters_buffer_ = nullptr;
	}

	if (!vertex_out_uav_)
	{
		delete vertex_out_uav_;
		vertex_out_uav_ = nullptr;
	}

	if (!heightmap_sampler_)
	{
		delete heightmap_sampler_;
		heightmap_sampler_ = nullptr;
	}
}

// INITIALISE SHADER METHOD .........................................................................................................................
void GenerateTerrainCShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	// load shader from file
	loadComputeShader(cfile);

	// create output buffer
	gpfw::CreateStructuredBuffer(renderer, sizeof(VertexType), v_count_, nullptr, &vertex_out_buffer_, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);
	gpfw::CreateBufferUAV(renderer, vertex_out_buffer_, &vertex_out_uav_);

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &heightmap_sampler_);

	// terrain parameters constant buffer description
	D3D11_BUFFER_DESC terrainParametersDesc;
	terrainParametersDesc.Usage = D3D11_USAGE_DYNAMIC;
	terrainParametersDesc.ByteWidth = sizeof(TerrainParametersType);
	terrainParametersDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	terrainParametersDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	terrainParametersDesc.MiscFlags = 0;
	terrainParametersDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&terrainParametersDesc, NULL, &terrain_parameters_buffer_);
}

// SHADER DISPATCH METHODS ..........................................................................................................................
void GenerateTerrainCShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* in_data, ID3D11ShaderResourceView* height_map, TerrainParametersType min_max)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	TerrainParametersType* TerrainParamPtr;

	// send constant buffer
	dc->Map(terrain_parameters_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	TerrainParamPtr = (TerrainParametersType*)mappedResource.pData;
	TerrainParamPtr->data_min = min_max.data_min;
	TerrainParamPtr->data_max = min_max.data_max;
	dc->Unmap(terrain_parameters_buffer_, 0);
	dc->CSSetConstantBuffers(0, 1, &terrain_parameters_buffer_);

	// set shader resources
	dc->CSSetShaderResources(0, 1, &height_map);
	dc->CSSetShaderResources(1, 1, &in_data);
	dc->CSSetUnorderedAccessViews(0, 1, &vertex_out_uav_, 0);
}

void GenerateTerrainCShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	dc->CSSetShader(nullptr, nullptr, 0);
}

void GenerateTerrainCShader::RunComputeShader(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* in_data, ID3D11ShaderResourceView* height_map, XMFLOAT2 data_min, XMFLOAT2 data_max, int x, int y, int z)
{
	// set the shader parameters
	TerrainParametersType temp;
	temp.data_max = data_max;
	temp.data_min = data_min;
	setShaderParameters(dc, in_data, height_map, temp);

	// dispatch the shader
	dc->CSSetShader(computeShader, NULL, 0);
	dc->Dispatch(x, y, z);

	// unbind shader
	unbind(dc);
}
