#include "shadow_shader.h"

ShadowShader::ShadowShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"res/shaders/shadow_vs.cso", L"res/shaders/shadow_ps.cso");
}

ShadowShader::~ShadowShader()
{
	// Release the sampler state.
	if (sample_state_shadow_)
	{
		sample_state_shadow_->Release();
		sample_state_shadow_ = 0;
	}

	// Release the matrix constant buffer.
	if (matrix_buffer_)
	{
		matrix_buffer_->Release();
		matrix_buffer_ = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (light_buffer_)
	{
		light_buffer_->Release();
		light_buffer_ = 0;
	}

	if (camera_buffer_)
	{
		camera_buffer_->Release();
		camera_buffer_ = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void ShadowShader::setShaderParameters(gpfw::ShaderInfo s_info, gpfw::LightingInfo l_info, gpfw::CascadeInfo c_info)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;
	ShadowBufferType* shadowPtr;

	XMMATRIX t_world, t_view, t_proj;
	XMMATRIX t_l_view[CASCADE_COUNT], t_l_proj[CASCADE_COUNT];

	// Transpose the matrices to prepare them for the shader.
	t_world = XMMatrixTranspose(s_info.world);
	t_view = XMMatrixTranspose(s_info.view);
	t_proj = XMMatrixTranspose(s_info.projection);

	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		t_l_view[i] = XMMatrixTranspose(c_info.view_matrices[i]);
		t_l_proj[i] = XMMatrixTranspose(c_info.proj_matrices[i]);
	}

	result = s_info.d_info.context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = t_world;
	dataPtr->view = t_view;
	dataPtr->projection = t_proj;

	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		dataPtr->lightView[i] = t_l_view[i];
		dataPtr->lightProjection[i] = t_l_proj[i];
	}
	
	s_info.d_info.context->Unmap(matrix_buffer_, 0);
	s_info.d_info.context->VSSetConstantBuffers(0, 1, &matrix_buffer_);

	// Send light data to pixel shader
	s_info.d_info.context->Map(light_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = l_info.ambient;
	s_info.d_info.context->Unmap(light_buffer_, 0);
	s_info.d_info.context->PSSetConstantBuffers(0, 1, &light_buffer_);

	// send shadow buffer to the shader
	s_info.d_info.context->Map(shadow_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	shadowPtr = (ShadowBufferType*)mappedResource.pData;
	shadowPtr->cascade_depths = XMFLOAT4(c_info.cascade_depths[0], c_info.cascade_depths[1], c_info.cascade_depths[2], 0);
	shadowPtr->far_cp = SCREEN_DEPTH;
	shadowPtr->near_cp = SCREEN_NEAR;
	shadowPtr->show_cascades = c_info.show_cascades;
	shadowPtr->padding = 0.f;
	shadowPtr->shadow_map_size = XMFLOAT4(c_info.depth_map_size[0], c_info.depth_map_size[1], c_info.depth_map_size[2], 0);
	s_info.d_info.context->Unmap(shadow_buffer_, 0);
	s_info.d_info.context->PSSetConstantBuffers(1, 1, &shadow_buffer_);

	// send camera buffer to shader
	CameraBufferType* cameraPtr;
	s_info.d_info.context->Map(camera_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->camera_position = s_info.cam_pos;
	cameraPtr->padding = 0.0f;
	s_info.d_info.context->Unmap(camera_buffer_, 0);
	s_info.d_info.context->VSSetConstantBuffers(1, 1, &camera_buffer_);

	ID3D11ShaderResourceView* depth_maps[CASCADE_COUNT];

	depth_maps[0] = c_info.cascade_maps[0]->getDepthMapSRV();
	depth_maps[1] = c_info.cascade_maps[1]->getDepthMapSRV();
	depth_maps[2] = c_info.cascade_maps[2]->getDepthMapSRV();

	// Set shader texture resource in the pixel shader.
	s_info.d_info.context->PSSetShaderResources(0, CASCADE_COUNT, depth_maps);
	s_info.d_info.context->PSSetSamplers(0, 1, &sample_state_shadow_);


}

void ShadowShader::Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo l_info, gpfw::Entity e, gpfw::CascadeInfo c_info)
{
	s_info.world = gpfw::entity::GetTransformMatrix(e);
	e.mesh->sendData(s_info.d_info.context);
	setShaderParameters(
		s_info,
		l_info,
		c_info
	);
	render(s_info.d_info.context, e.mesh->getIndexCount());
}

void ShadowShader::Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo l_info, gpfw::TerrainChunk t, gpfw::CascadeInfo c_info)
{
	s_info.world = gpfw::terrain::GetTransformMatrix(t);
	t.mesh->sendData(s_info.d_info.context);
	setShaderParameters(
		s_info,
		l_info,
		c_info
	);
	render(s_info.d_info.context, t.mesh->getIndexCount());
}

void ShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_SAMPLER_DESC shadowSamplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC shadowBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrix_buffer_);

	// Sampler for shadow map sampling.
	shadowSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&shadowSamplerDesc, &sample_state_shadow_);

	// set up buffers
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &light_buffer_);

	shadowBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	shadowBufferDesc.ByteWidth = sizeof(ShadowBufferType);
	shadowBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	shadowBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	shadowBufferDesc.MiscFlags = 0;
	shadowBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&shadowBufferDesc, NULL, &shadow_buffer_);

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &camera_buffer_);
}
