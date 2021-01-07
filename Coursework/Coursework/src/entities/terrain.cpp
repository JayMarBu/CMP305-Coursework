#include "entities/terrain.h"
#include <cassert>

namespace gpfw
{
	namespace terrain 
	{
		// get global size of terrain chunk
		int gpfw::terrain::getSideLength()
		{
			int n = TERRAIN_SIZE;

			return TERRAIN_SIZE - 1;
		}

		// create a new terrain chunk
		gpfw::TerrainChunk CreateTerrainChunk(DeviceInfo d_info, TextureManager* texture_manager, const wchar_t* texture /*= L"Default"*/, int resolution /*= 128*/, int sidelen /*= 10*/, XMFLOAT3 pos)
		{
			TerrainChunk t;
			t.mesh = new TerrainMesh(d_info.device, d_info.context, resolution, sidelen);
			t.texture_manager = texture_manager;
			t.texture = texture;
			t.position = pos;

			return t;
		}

		// release terrain chunk memory
		void ReleaseTerrainChunk(TerrainChunk& t)
		{
			if (t.mesh != nullptr)
			{
				delete t.mesh;
				t.mesh = nullptr;
			}

			t.texture_manager = nullptr;
		}

		// generate terrain mesh on compute shader
		void Generate(TerrainChunk& t, DeviceInfo d_info, GenerateTerrainCShader* cs, ID3D11ShaderResourceView* height_map, CSDimentions dim)
		{
			t.mesh->ApplyHeightMap(d_info.device, d_info.context, cs, height_map, t.h_params, dim.x, dim.y, dim.z);
		}

	}
}
	




