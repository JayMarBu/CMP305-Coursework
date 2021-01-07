#pragma once
#include "BaseMesh.h"
#include "shaders/compute_shaders/generate_terrain_cshader.h"

namespace gpfw
{
	struct HeightParameters;
}

// improved plane mesh class with the added ability to generate from a height map
class TerrainMesh : public BaseMesh
{
public:
	TerrainMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int resolution = 128, int side_length = 10);
	~TerrainMesh();

	void ApplyHeightMap(ID3D11Device* device, ID3D11DeviceContext* context, GenerateTerrainCShader* cs, ID3D11ShaderResourceView* height_map, gpfw::HeightParameters h_params, int x, int y, int z);
	void CreateVertexBuffer(ID3D11Device* device, void* data);
	void CreateIndexBuffer(ID3D11Device* device, void* data);

private:
	void initBuffers(ID3D11Device* device);

	int resolution;
	int side_length;
};