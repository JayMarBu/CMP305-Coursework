#pragma once
#include "DXF.h"

// the max number of lights to be added to the scene
#define MAX_LIGHTS 8

#define CASCADE_COUNT 3

class TextureShader;

// using graphics programming framework namespace
namespace gpfw
{
	// D3D11 EDVICE INFORMATION STRUCT ..............................................................................................................
	struct DeviceInfo
	{
		ID3D11Device* device;
		ID3D11DeviceContext* context;
	};

	// COMMON SHADER PARAMETERS STRUCT ..............................................................................................................
	struct ShaderInfo
	{
		DeviceInfo d_info;
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMFLOAT3 cam_pos;
	};

	// LIGHTING INFORMATION STRUCT .....................................................................................................................
	struct LightingInfo
	{
		// pointers to light object
		Light* light = nullptr;

		// data to be sent to shader
		XMFLOAT4 diffuse;

		XMFLOAT4 dir_spec_pow;

		XMFLOAT4 spec_colour;

		XMFLOAT4 ambient;
	};

	// LIGHTING RELATED MEMBERS .....................................................................................................................
	namespace light
	{
		extern void SetLight(LightingInfo& l, Light* light);

		extern void ReleaseLight(LightingInfo& l);

		extern void SetDiffuseColour(LightingInfo& l, XMFLOAT4);
		extern void SetDiffuseColour(LightingInfo& l, float r, float g, float b, float a);

		extern void SetDirection(LightingInfo& l, XMFLOAT3);
		extern void SetDirection(LightingInfo& l, float x, float y, float z);

		extern XMFLOAT3 GetDirection(LightingInfo& l);

		extern void SetSpecularPower(LightingInfo& l, float p);

		extern void SetSpecularColour(LightingInfo& l, XMFLOAT4);
		extern void SetSpecularColour(LightingInfo& l, float r, float g, float b, float a);

		extern void SetAmbientColour(LightingInfo& l, float r, float g, float b, float a);
	}

	// CASCADE SHADOW MAP DATA ......................................................................................................................
	struct CascadeInfo
	{
		// array of shadow map objects to store depth textures
		ShadowMap* cascade_maps[CASCADE_COUNT];

		// the size in pixels of each depth texture
		unsigned int depth_map_size[CASCADE_COUNT];

		// array of ortho meshes to display debug information
		OrthoMesh* ortho_mesh[CASCADE_COUNT];

		// array of positions at the centre of each cascade
		XMFLOAT3 s_map_pos[CASCADE_COUNT];

		// array containing the dimensions of each cascades
		float cascade_depths[CASCADE_COUNT];

		// copy of the view frustrum's near and 
		// far plane for depth calculations
		float near_clipping_plane;
		float far_clipping_plane;

		// view matrix for each cascade region.
		// view is from the light offset to be centred
		// on the cascade region
		XMMATRIX view_matrices[CASCADE_COUNT];

		// projection frustrum slice of each cascade region
		XMMATRIX proj_matrices[CASCADE_COUNT];

		// debug render flag
		bool show_cascades = false;
		
		// orthographic size of light
		// projection matrices
		unsigned int scene_width;
		unsigned int scene_height;
	};

	// CASCADE MAP RELATED METHODS ..................................................................................................................
	namespace cascade
	{
		extern void InitCascadeMap(
			CascadeInfo& c,
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			const unsigned int screen_width,
			const unsigned int screen_height,
			const unsigned int first_map_size = 2048,
			const unsigned int scene_width = 200,
			const unsigned int scene_height = 200
		);

		extern void ReleaseCascadeMaps(CascadeInfo);

		extern void RenderOrthoMeshes(CascadeInfo& c, TextureShader* shader, ShaderInfo s_info, XMMATRIX o_view_matrix, XMMATRIX o_proj_matrix);

		extern XMFLOAT4 CalcFrustrumCentre(XMMATRIX view_matrix, XMMATRIX proj_matrix, float height);
	}

	// COMPUTE SHADER RELETED FUNCTIONS .............................................................................................................
	void CreateStructuredBuffer(ID3D11Device* d, unsigned int element_size, unsigned int count, void* initData, ID3D11Buffer** buffer, int cpu_flag = D3D11_CPU_ACCESS_WRITE);
	void CreateBufferSRV(ID3D11Device* d, ID3D11Buffer* buffer, ID3D11ShaderResourceView** out_SRV);
	void CreateBufferUAV(ID3D11Device* d, ID3D11Buffer* buffer, ID3D11UnorderedAccessView** out_UAV);

	ID3D11Buffer* CreateAndCopyDebugBuffer(ID3D11Device* d, ID3D11DeviceContext* dc, ID3D11Buffer* in_buffer);

	template <typename T>
	void GetDataFromBuffer(ID3D11Device* d, ID3D11DeviceContext* dc, ID3D11Buffer* in_buffer, T** data)
	{
		ID3D11Buffer* debug_buffer_ = gpfw::CreateAndCopyDebugBuffer(d, dc, in_buffer);
		D3D11_MAPPED_SUBRESOURCE MappedResource;

		dc->Map(debug_buffer_, 0, D3D11_MAP_READ, 0, &MappedResource);

		*data = (T*)MappedResource.pData;

		dc->Unmap(debug_buffer_, 0);

		debug_buffer_->Release();
	}

	// UTILITY FUNCTIONS ............................................................................................................................
	template <typename T> int sign(T val) { return (T(0) < val) - (val < T(0)); }

}

