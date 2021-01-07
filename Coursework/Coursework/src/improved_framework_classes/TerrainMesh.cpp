#include "TerrainMesh.h"
#include "entities/terrain.h"
#include "shaders/shader_utils.h"
#include <iostream>

TerrainMesh::TerrainMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int lresolution, int lside_length) : BaseMesh()
{
	resolution = lresolution;
	side_length = lside_length;
	initBuffers(device);
}

TerrainMesh::~TerrainMesh()
{
	BaseMesh::~BaseMesh();
}

void TerrainMesh::ApplyHeightMap(ID3D11Device* device, ID3D11DeviceContext* context, GenerateTerrainCShader* cs, ID3D11ShaderResourceView* height_map, gpfw::HeightParameters h_params, int x, int y, int z)
{
	// create temporary shader resource views
	ID3D11Buffer* vertex_srv_buffer_;
	ID3D11ShaderResourceView* vertex_srv_;

	// create temporary buffer to store data
	VertexType* verts = nullptr;

	// get current vertex buffer
	gpfw::GetDataFromBuffer<VertexType>(device, context, vertexBuffer, &verts);

	// create a new structured buffer containing the vertex array
	gpfw::CreateStructuredBuffer(device, sizeof(VertexType), vertexCount, verts, &vertex_srv_buffer_, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ);

	// create a shader resource view for the vertex buffer
	gpfw::CreateBufferSRV(device, vertex_srv_buffer_, &vertex_srv_);

	// set minimum and maximum span of plane mesh
	XMFLOAT2 min_values;
	XMFLOAT2 max_values;

	min_values.x = -side_length / 2;
	min_values.y = -side_length / 2;
	max_values.x = side_length / 2;
	max_values.y = side_length / 2;

	// dispatch the compute shader
	cs->RunComputeShader(context, vertex_srv_, height_map, min_values, max_values, h_params, x, y, z);

	// empty temporary buffer
	verts = nullptr;

	// receive newly updated vertex buffer
	gpfw::GetDataFromBuffer<VertexType>(device, context, cs->getVertexOutBuffer(), &verts);

	// map new vertices to current buffer
	D3D11_MAPPED_SUBRESOURCE resource;
	VertexType* new_verts;
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	new_verts = (VertexType*)resource.pData;
	memcpy(new_verts, verts, 65536*sizeof(VertexType));
	context->Unmap(vertexBuffer, 0);

	// release uneeded resources
	vertex_srv_buffer_->Release();
	vertex_srv_->Release();
}

void TerrainMesh::CreateVertexBuffer(ID3D11Device* device, void* data)
{
	VertexType* vertices = nullptr;
	vertices = (VertexType*)data;

	D3D11_BUFFER_DESC desc;
	D3D11_SUBRESOURCE_DATA sr_data;

	// Set up the description of the static vertex buffer.
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(VertexType) * ((resolution + 1) * (resolution + 1));
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// Give the sub-resource structure a pointer to the vertex data.
	sr_data.pSysMem = vertices;
	sr_data.SysMemPitch = 0;
	sr_data.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&desc, &sr_data, &vertexBuffer);
}

void TerrainMesh::CreateIndexBuffer(ID3D11Device* device, void* data)
{
	D3D11_BUFFER_DESC desc;
	D3D11_SUBRESOURCE_DATA sr_data;

	// Set up the description of the static index buffer.
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(unsigned long) * indexCount;
	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	// Give the sub-resource structure a pointer to the index data.
	sr_data.pSysMem = data;
	sr_data.SysMemPitch = 0;
	sr_data.SysMemSlicePitch = 0;

	if (indexBuffer != nullptr)
	{
		indexBuffer->Release();
		indexBuffer = 0;
	}

	HRESULT result = device->CreateBuffer(&desc, &sr_data, &indexBuffer);
}

void TerrainMesh::initBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	int index, i, j;
	float positionX, positionZ, u, v, increment;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, vertexBufferUAVDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Calculate the number of vertices in the terrain mesh.
	vertexCount = (resolution+1) * (resolution+1);

	//indexCount = vertexCount;
	indexCount = resolution * resolution * 6;
	vertices = new VertexType[vertexCount];
	indices = new unsigned long[indexCount];
	
	// UV coords.
	u = 0;
	v = 0;
	increment = side_length/(float)resolution;
	int v_resolution = resolution + 1;

	// create the vertex array
	for (int i = 0; i < v_resolution; i++)
	{
		for (int j = 0; j < v_resolution; j++)
		{
			// build vertex
			positionX = ((float)j * increment) -((float)side_length/2);
			positionZ = ((float)i * increment) -((float)side_length/2);

			vertices[(v_resolution * i) + j].position = XMFLOAT3(positionX, 0.0f, positionZ);
			vertices[(v_resolution * i) + j].texture = XMFLOAT2((float)i/((float)resolution/64.0f), (float)j/((float)resolution/64.0f));
			vertices[(v_resolution * i) + j].normal = XMFLOAT3(0.0, 1.0, 0.0);
		}
	}

	index = 0;

	// create the index array
	int balancer = resolution % 2;
	for (int i = 0; i < resolution; i++)
	{
		for (int j = 0; j < resolution; j++)
		{
			int k = ((float)(resolution * i) + j)+(resolution%2);

			if (balancer == 0)
				if (i % 2 == 1)
					k += 1;

			int quad_index = ((v_resolution * i) + j);

			if (k%2 == 0)
			{
				indices[index] = quad_index + v_resolution;
				indices[index + 1] = quad_index;
				indices[index + 2] = quad_index + 1;

				indices[index + 3] = quad_index + v_resolution;
				indices[index + 4] = quad_index + 1;
				indices[index + 5] = quad_index + v_resolution + 1;
			}
			else // k%2 == 1
			{
				indices[index] = quad_index + v_resolution;
				indices[index + 1] = quad_index;
				indices[index + 2] = quad_index + v_resolution+1;

				indices[index + 3] = quad_index + v_resolution+1;
				indices[index + 4] = quad_index;
				indices[index + 5] = quad_index + 1;
			}

			index += 6;
		}
	}

	CreateVertexBuffer(device, vertices);

	CreateIndexBuffer(device, indices);
	

	// Release the arrays now that the buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

