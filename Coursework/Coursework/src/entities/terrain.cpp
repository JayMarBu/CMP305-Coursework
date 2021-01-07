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
		gpfw::TerrainChunk CreateTerrainChunk(DeviceInfo d_info, TextureManager* texture_manager, const wchar_t* texture0 /*= L"Default"*/, const wchar_t* texture1,const wchar_t* texture2,const wchar_t* texture3, int resolution /*= 128*/, int sidelen /*= 10*/, XMFLOAT3 pos)
		{
			TerrainChunk t;
			t.mesh = new TerrainMesh(d_info.device, d_info.context, resolution, sidelen);
			t.texture_manager = texture_manager;
			t.texture0 = texture0;
			t.texture1 = texture1;
			t.texture2 = texture2;
			t.texture3 = texture3;
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
	




