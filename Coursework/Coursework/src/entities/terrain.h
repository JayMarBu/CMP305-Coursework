#pragma once
#include "DXF.h"
//#include "entities/entity.h"
#include "shaders/compute_shaders/generate_terrain_cshader.h"
#include "shaders/shader_utils.h"
#include "improved_framework_classes/TerrainMesh.h"
#include "noise/perlin_noise.h"

#define TERRAIN_SIZE 256

namespace gpfw
{
	struct HeightParameters
	{
		float height = 10.0f;
	};

	// TERRAIN CHUNK STRUCT .........................................................................................................................
	struct TerrainChunk
	{
		// world coordinate translation coordinates
		XMFLOAT3 position;

		// mesh pointer
		TerrainMesh* mesh;

		// height & material textures
		const wchar_t* heightmap = L"Default";
		const wchar_t* texture = L"Default";

		// pointer to application class' texture manager
		// for ease of access
		TextureManager* texture_manager;

		HeightParameters h_params;
	};

	// COMPUTE SHADER DIMENSIONS PACKAGE ............................................................................................................
	// exists to shrink function parameter list length
	struct CSDimentions
	{
		int x;
		int y;
		int z;

		CSDimentions(int ix, int iy, int iz)
			: x(ix),y(iy),z(iz){}

		CSDimentions()
			: x(1), y(1), z(1){}

	};

	// TERRAIN CHUNK RELATED METHODS ................................................................................................................
	namespace terrain
	{
		// TERRAIN CREATION AND DESTRUCTION METHODS ...............................................
		TerrainChunk CreateTerrainChunk(DeviceInfo d_info, TextureManager* texture_manager, const wchar_t* texture = L"Default", int resolution = 128, int sidelen = 10, XMFLOAT3 pos = XMFLOAT3(0,0,0));

		void ReleaseTerrainChunk(TerrainChunk& t);

		// HEIGHT GENERATION RELATED METHODS ......................................................
		int getSideLength();
		void Generate(TerrainChunk& t, DeviceInfo s_info, GenerateTerrainCShader* cs,ID3D11ShaderResourceView* height_map, CSDimentions dim = CSDimentions(16,16,1));

		// NON-TRIVIAL GETTER METHODS .............................................................
		inline XMMATRIX GetTransformMatrix(TerrainChunk& t) { return XMMatrixTranslation(t.position.x, t.position.y, t.position.z); }
		inline ID3D11ShaderResourceView* GetTexture(TerrainChunk& t) { return t.texture_manager->getTexture(t.texture); }

		// NOISE FUNCTIONS ........................................................................
	}
}