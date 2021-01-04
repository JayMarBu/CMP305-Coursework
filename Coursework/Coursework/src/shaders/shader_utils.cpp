#include "shader_utils.h"
#include "shaders/texture_shader/texture_shader.h"
#include <cmath>
#include <ctgmath>

namespace gpfw
{
	// LIGHT NAMESPACE .................................................................................................................................
	namespace light
	{
		void SetLight(LightingInfo& l, Light* light)
		{
			light->setDirection(1.0f, 0.0f, 0.0f);
			light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);

			l.light = light;

			l.diffuse = light->getDiffuseColour();
			l.dir_spec_pow = XMFLOAT4(
				light->getDirection().x,
				light->getDirection().y,
				light->getDirection().z,
				light->getSpecularPower()
			);
			l.spec_colour = light->getSpecularColour();
		}

		void ReleaseLight(LightingInfo& l)
		{
			if (l.light != nullptr)
			{
				delete l.light;
				l.light = nullptr;
			}
		}

		void SetDiffuseColour(LightingInfo& l, XMFLOAT4 in)
		{
			l.diffuse = in;
			l.light->setDiffuseColour(in.x, in.y, in.z, in.w);
		}

		void SetDiffuseColour(LightingInfo& l, float r, float g, float b, float a)
		{
			SetDiffuseColour(l, XMFLOAT4(r, g, b, a));
		}

		void SetDirection(LightingInfo& l, XMFLOAT3 in)
		{
			// normalize vector
			XMVECTOR in_vec = XMLoadFloat3(&in);
			XMVector3Normalize(in_vec);
			XMStoreFloat3(&in, in_vec);

			// set direction vector
			l.dir_spec_pow.x = in.x;
			l.dir_spec_pow.y = in.y;
			l.dir_spec_pow.z = in.z;
			l.light->setDirection(in.x, in.y, in.z);
		}

		void SetDirection(LightingInfo& l, float x, float y, float z)
		{
			SetDirection(l, XMFLOAT3(x, y, z));
		}

		XMFLOAT3 GetDirection(LightingInfo& l)
		{
			return XMFLOAT3(l.dir_spec_pow.x, l.dir_spec_pow.y, l.dir_spec_pow.z);
		}

		void SetSpecularPower(LightingInfo& l, float p)
		{
			l.dir_spec_pow.w = p;
			l.light->setSpecularPower(p);
		}

		void SetSpecularColour(LightingInfo& l, XMFLOAT4 in)
		{
			l.spec_colour = in;
			l.light->setSpecularColour(in.x, in.y, in.z, in.w);

		}

		void SetSpecularColour(LightingInfo& l, float r, float g, float b, float a)
		{
			SetSpecularColour(l, XMFLOAT4(r, g, b, a));
		}

		void SetAmbientColour(LightingInfo& l, float r, float g, float b, float a)
		{
			l.ambient = XMFLOAT4(r, g, b, a);
		}

	}

	// CASCADE MAP NAMESPACE ........................................................................................................................
	namespace cascade
	{
		// method to find the centre and bounding radius of a given cascade frustrum
		// returns { centree_point.x, centre_point.y, centre_point.z, bounding_radius }
		XMFLOAT4 CalcFrustrumCentre(XMMATRIX view_matrix, XMMATRIX proj_matrix, float height)
		{
			XMFLOAT4 output = XMFLOAT4(0, 0, 0, 0);
			XMVECTOR temp_vec;

			// map out the frustrum of each cascade as a unit cube
			XMFLOAT3 frustrum_corners[8] =
			{
				XMFLOAT3(-1.0f,1.0f,0.0f),
				XMFLOAT3(1.0f,1.0f,0.0f),
				XMFLOAT3(1.0f,-1.0f,0.0f),
				XMFLOAT3(-1.0f,-1.0f,0.0f),
				XMFLOAT3(-1.0f,1.0f,1.0f),
				XMFLOAT3(1.0f,1.0f,1.0f),
				XMFLOAT3(1.0f,-1.0f,1.0f),
				XMFLOAT3(-1.0f,-1.0f,1.0f)//*/
			};

			// get the inverse view matrix of the camera along with the 
			// projection matrix of each cascade region
			XMMATRIX cam_proj_matrix = view_matrix;
			cam_proj_matrix = XMMatrixMultiply(cam_proj_matrix, proj_matrix);
			XMMATRIX inv_view_proj;
			inv_view_proj = XMMatrixInverse(nullptr, cam_proj_matrix);

			// apply the cascade view matrix to the unit cube to map the frustrum
			// in world space
			for (int i = 0; i < 8; ++i)
			{
				temp_vec = XMLoadFloat3(&frustrum_corners[i]);
				temp_vec = XMVector3TransformCoord(temp_vec, inv_view_proj);
				XMStoreFloat3(&frustrum_corners[i], temp_vec);
			}

			// calculate the centre point of each frustrum
			for (int i = 0; i < 8; ++i)
			{
				output.x = output.x + frustrum_corners[i].x;
				output.y = output.y + frustrum_corners[i].y;
				output.z = output.z + frustrum_corners[i].z;
			}

			output.x *= 1.0f / 8.0f;
			output.y *= 1.0f / 8.0f;
			output.z *= 1.0f / 8.0f;

			// calculate radius
			temp_vec;
			XMFLOAT3 temp_float3 = XMFLOAT3(
				output.x - frustrum_corners[4].x,
				output.y - frustrum_corners[4].y,
				output.z - frustrum_corners[4].z
			);
			temp_vec = XMLoadFloat3(&temp_float3);
			temp_vec = DirectX::XMVector3Length(temp_vec);
			DirectX::XMStoreFloat3(&temp_float3, temp_vec);
			output.w = temp_float3.x;

			return output;
		}

		void InitCascadeMap(
			CascadeInfo& c,
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			const unsigned int screen_width,
			const unsigned int screen_height,
			const unsigned int map_size,
			const unsigned int scene_width,
			const unsigned int scene_height
		)
		{
			c.scene_height = scene_height;
			c.scene_width = scene_width;
			const unsigned int padding = 5;

			for (int i = 0; i < CASCADE_COUNT; i++)
			{
				c.depth_map_size[i] = map_size;

				c.cascade_maps[i] = new ShadowMap(device, c.depth_map_size[i], c.depth_map_size[i]);
				c.ortho_mesh[i] = new OrthoMesh(
					device, context,
					scene_width, scene_height,
					screen_width / 2 - (scene_width / 2) - padding,
					screen_height / 2 - (scene_height / 2) - (padding * (i + 1)) - (scene_height * i)
				);
				c.s_map_pos[i] = XMFLOAT3(0, 0, 0);
			}
		}

		void ReleaseCascadeMaps(CascadeInfo c)
		{
			for (int i = 0; i < CASCADE_COUNT; i++)
			{
				if (c.cascade_maps[i] != nullptr)
				{
					delete c.cascade_maps[i];
					c.cascade_maps[i] = nullptr;
				}

				if (c.ortho_mesh[i] != nullptr)
				{
					delete c.ortho_mesh[i];
					c.ortho_mesh[i] = nullptr;
				}
			}
		}

		void RenderOrthoMeshes(CascadeInfo& c, TextureShader* shader, ShaderInfo s_info, XMMATRIX o_view_matrix, XMMATRIX o_proj_matrix)
		{
			for (int i = 0; i < CASCADE_COUNT; i++)
			{
				c.ortho_mesh[i]->sendData(s_info.d_info.context);
				shader->setShaderParameters(s_info.d_info.context, s_info.world, o_view_matrix, o_proj_matrix, c.cascade_maps[i]->getDepthMapSRV());
				shader->render(s_info.d_info.context, c.ortho_mesh[i]->getIndexCount());
			}
		}

	}

	// COMPUTE SHADER RELETAED METHODS ..............................................................................................................
	void CreateStructuredBuffer(ID3D11Device* d, unsigned int element_size, unsigned int count, void* initData, ID3D11Buffer** buffer, int cpu_flag)
	{
		// create buffer description
		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = cpu_flag;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = element_size * count;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = element_size;

		// fill with initial data
		if (initData)
		{
			D3D11_SUBRESOURCE_DATA init_data;
			init_data.pSysMem = initData;
			HRESULT result = d->CreateBuffer(&desc, &init_data, buffer);
			return;
		}

		// create the buffer
		HRESULT result = d->CreateBuffer(&desc, nullptr, buffer);
	}

	void CreateBufferSRV(ID3D11Device* d, ID3D11Buffer* buffer, ID3D11ShaderResourceView** out_SRV)
	{
		// get the buffer sescription
		D3D11_BUFFER_DESC desc_buffer;
		buffer->GetDesc(&desc_buffer);

		// create SRV description
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.BufferEx.FirstElement = 0;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.BufferEx.NumElements = desc_buffer.ByteWidth / desc_buffer.StructureByteStride;

		// create SRV
		HRESULT result = d->CreateShaderResourceView(buffer, &desc, out_SRV);
	}

	void CreateBufferUAV(ID3D11Device* d, ID3D11Buffer* buffer, ID3D11UnorderedAccessView** out_UAV)
	{
		// get buffer description
		D3D11_BUFFER_DESC desc_buffer;
		buffer->GetDesc(&desc_buffer);

		// create UAV description
		D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Format = DXGI_FORMAT_UNKNOWN;      
		desc.Buffer.NumElements = desc_buffer.ByteWidth / desc_buffer.StructureByteStride;

		// create UAV
		d->CreateUnorderedAccessView(buffer, &desc, out_UAV);
	}

	ID3D11Buffer* CreateAndCopyDebugBuffer(ID3D11Device* d, ID3D11DeviceContext* dc, ID3D11Buffer* in_buffer)
	{
		ID3D11Buffer* debug_bufffer = nullptr;

		D3D11_BUFFER_DESC desc = {};
		in_buffer->GetDesc(&desc);
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.MiscFlags = 0;
		if (SUCCEEDED(d->CreateBuffer(&desc, nullptr, &debug_bufffer)))
		{
			dc->CopyResource(debug_bufffer, in_buffer);
		}

		return debug_bufffer;
	}
}