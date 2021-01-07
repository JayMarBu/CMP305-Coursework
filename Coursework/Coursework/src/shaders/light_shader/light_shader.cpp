#include "light_shader.h"

LightShader::LightShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"res/shaders/light_vs.cso", L"res/shaders/light_ps.cso");
}

LightShader::~LightShader()
{
	// Release the sampler state.
	if (sample_state_)
	{
		sample_state_->Release();
		sample_state_ = 0;
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

	if (clip_plane_buffer_)
	{
		clip_plane_buffer_->Release();
		clip_plane_buffer_ = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void LightShader::setShaderParameters(gpfw::ShaderInfo s_info, ID3D11ShaderResourceView* texture0, ID3D11ShaderResourceView* texture1, ID3D11ShaderResourceView* texture2,ID3D11ShaderResourceView* texture3, gpfw::LightingInfo l_info, ID3D11ShaderResourceView* shadow_map, XMFLOAT4 clip_plane)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	LightBufferType* lightPtr;

	XMMATRIX t_world, t_view, t_proj;

	// Transpose the matrices to prepare them for the shader.
	t_world = XMMatrixTranspose(s_info.world);
	t_view = XMMatrixTranspose(s_info.view);
	t_proj = XMMatrixTranspose(s_info.projection);

	result = s_info.d_info.context->Map(matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = t_world;// worldMatrix;
	dataPtr->view = t_view;
	dataPtr->projection = t_proj;
	
	s_info.d_info.context->Unmap(matrix_buffer_, 0);
	s_info.d_info.context->VSSetConstantBuffers(0, 1, &matrix_buffer_);

	//Additional
	// Send light data to pixel shader
	s_info.d_info.context->Map(light_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->ambient = l_info.ambient;
	lightPtr->diffuse = l_info.diffuse;
	lightPtr->direction= gpfw::light::GetDirection(l_info);
	lightPtr->water_level = clip_plane.w;
	s_info.d_info.context->Unmap(light_buffer_, 0);
	s_info.d_info.context->PSSetConstantBuffers(0, 1, &light_buffer_);

	// send camera buffer to shader
	CameraBufferType* cameraPtr;
	s_info.d_info.context->Map(camera_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->camera_position = s_info.cam_pos;
	cameraPtr->padding = 0.0f;
	s_info.d_info.context->Unmap(camera_buffer_, 0);
	s_info.d_info.context->VSSetConstantBuffers(1, 1, &camera_buffer_);

	// send clip plane buffer to shader
	XMFLOAT4* cliPlanePtr;
	s_info.d_info.context->Map(clip_plane_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cliPlanePtr = (XMFLOAT4*)mappedResource.pData;
	cliPlanePtr->x = clip_plane.x;
	cliPlanePtr->y = clip_plane.y;
	cliPlanePtr->z = clip_plane.z;
	cliPlanePtr->w = clip_plane.w;
	s_info.d_info.context->Unmap(clip_plane_buffer_, 0);
	s_info.d_info.context->VSSetConstantBuffers(2, 1, &clip_plane_buffer_);

	// Set shader texture resource in the pixel shader.
	s_info.d_info.context->PSSetShaderResources(0, 1, &texture0);
	s_info.d_info.context->PSSetShaderResources(1, 1, &texture1);
	s_info.d_info.context->PSSetShaderResources(2, 1, &texture2);
	s_info.d_info.context->PSSetShaderResources(3, 1, &texture3);
	s_info.d_info.context->PSSetSamplers(0, 1, &sample_state_);

	// Set shader texture resource in the pixel shader.
	s_info.d_info.context->PSSetShaderResources(4, 1, &shadow_map);
	s_info.d_info.context->PSSetSamplers(1, 1, &sample_state_shadow_);


}

void LightShader::Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo l_info, gpfw::Entity e, ID3D11ShaderResourceView* shadow_map, XMFLOAT4 clip_plane)
{
	s_info.world = gpfw::entity::GetTransformMatrix(e);
	e.mesh->sendData(s_info.d_info.context);
	setShaderParameters(
		s_info,
		gpfw::entity::GetTexture(e),
		gpfw::entity::GetTexture(e),
		gpfw::entity::GetTexture(e),
		gpfw::entity::GetTexture(e),
		l_info,
		shadow_map,
		clip_plane
	);
	render(s_info.d_info.context, e.mesh->getIndexCount());
}

void LightShader::Render(gpfw::ShaderInfo s_info, gpfw::LightingInfo l_info, gpfw::TerrainChunk t, ID3D11ShaderResourceView* shadow_map, XMFLOAT4 clip_plane)
{
	s_info.world = gpfw::terrain::GetTransformMatrix(t);
	t.mesh->sendData(s_info.d_info.context);
	setShaderParameters(
		s_info,
		gpfw::terrain::GetTexture0(t),
		gpfw::terrain::GetTexture1(t),
		gpfw::terrain::GetTexture2(t),
		gpfw::terrain::GetTexture3(t),
		l_info,
		shadow_map,
		clip_plane
	);
	render(s_info.d_info.context, t.mesh->getIndexCount());
}

void LightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC clipPlaneBufferDesc;

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

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sample_state_);

	// Sampler for shadow map sampling.
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	renderer->CreateSamplerState(&samplerDesc, &sample_state_shadow_);

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &light_buffer_);

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &camera_buffer_);

	clipPlaneBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	clipPlaneBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	clipPlaneBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	clipPlaneBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	clipPlaneBufferDesc.MiscFlags = 0;
	clipPlaneBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&clipPlaneBufferDesc, NULL, &clip_plane_buffer_);
}
