#include "water_shader.h"

WaterShader::WaterShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"res/shaders/water_vs.cso", L"res/shaders/water_ps.cso");
}

WaterShader::~WaterShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void WaterShader::setShaderParameters(gpfw::ShaderInfo s_info, gpfw::WaterInfo w_info, gpfw::LightingInfo l_info,
	ID3D11ShaderResourceView* reflection, ID3D11ShaderResourceView* refraction, ID3D11ShaderResourceView* dudv, ID3D11ShaderResourceView* normal_map)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	WaterShader::MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj, tdproj, tdview;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(s_info.world);
	tview = XMMatrixTranspose(s_info.view);
	tproj = XMMatrixTranspose(s_info.projection);

	// Send matrix data
	result = s_info.d_info.context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (WaterShader::MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	s_info.d_info.context->Unmap(matrixBuffer, 0);
	s_info.d_info.context->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// send water vs buffer to shader
	WaterBufferVSType* waterVSPtr;
	s_info.d_info.context->Map(waterVSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	waterVSPtr = (WaterBufferVSType*)mappedResource.pData;
	waterVSPtr->eye_pos = s_info.cam_pos;
	waterVSPtr->tiling = w_info.tiling;
	waterVSPtr->light_pos =
		XMFLOAT3(
		s_info.cam_pos.x - l_info.light->getPosition().x*10,
		s_info.cam_pos.y - l_info.light->getPosition().y * 10,
		s_info.cam_pos.z - l_info.light->getPosition().z * 10
	);
	waterVSPtr->padding = 0.0f;
	s_info.d_info.context->Unmap(waterVSBuffer, 0);
	s_info.d_info.context->VSSetConstantBuffers(1, 1, &waterVSBuffer);

	// send water ps buffer to shader
	WaterBufferPSType* waterPSPtr;
	s_info.d_info.context->Map(waterPSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	waterPSPtr = (WaterBufferPSType*)mappedResource.pData;
	waterPSPtr->wave_strength = w_info.distortion_strength;
	waterPSPtr->move_factor = w_info.move_factor;
	waterPSPtr->colour_intensity = w_info.colour_intensity;
	waterPSPtr->water_colour = w_info.water_colour;
	waterPSPtr->reflection_strength = w_info.reflection_strength;
	s_info.d_info.context->Unmap(waterPSBuffer, 0);
	s_info.d_info.context->PSSetConstantBuffers(0, 1, &waterPSBuffer);

	// send water ps buffer to shader
	SceneBufferType* scenePtr;
	s_info.d_info.context->Map(sceneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	scenePtr = (SceneBufferType*)mappedResource.pData;
	scenePtr->colour = l_info.spec_colour;
	scenePtr->specular_power = l_info.dir_spec_pow.w;
	scenePtr->padding = XMFLOAT3(0, 0, 0);
	s_info.d_info.context->Unmap(sceneBuffer, 0);
	s_info.d_info.context->PSSetConstantBuffers(1, 1, &sceneBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	s_info.d_info.context->PSSetShaderResources(0, 1, &reflection);
	s_info.d_info.context->PSSetShaderResources(1, 1, &refraction);
	s_info.d_info.context->PSSetShaderResources(2, 1, &dudv);
	s_info.d_info.context->PSSetShaderResources(3, 1, &normal_map);
	s_info.d_info.context->PSSetSamplers(0, 1, &sampleState);
}

void WaterShader::Render(gpfw::ShaderInfo s_info, gpfw::WaterInfo w_info, gpfw::LightingInfo l_info)
{
	s_info.world = gpfw::entity::GetTransformMatrix(w_info.water_plane);
	w_info.water_plane.mesh->sendData(s_info.d_info.context);
	setShaderParameters(
		s_info,
		w_info,
		l_info,
		w_info.reflection_texture->getShaderResourceView(),
		w_info.refraction_texture->getShaderResourceView(),
		w_info.water_plane.texture_manager->getTexture(w_info.DuDv_map),
		w_info.water_plane.texture_manager->getTexture(w_info.normal_map)
	);
	render(s_info.d_info.context, w_info.water_plane.mesh->getIndexCount());
}

void WaterShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC waterVSBufferDesc;
	D3D11_BUFFER_DESC waterPSBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

	// Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
	
	// vertex buffer water settings
	waterVSBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterVSBufferDesc.ByteWidth = sizeof(WaterBufferVSType);
	waterVSBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterVSBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterVSBufferDesc.MiscFlags = 0;
	waterVSBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&waterVSBufferDesc, NULL, &waterVSBuffer);
	
	// vertex buffer water settings
	waterPSBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	waterPSBufferDesc.ByteWidth = sizeof(WaterBufferPSType);
	waterPSBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	waterPSBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	waterPSBufferDesc.MiscFlags = 0;
	waterPSBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&waterPSBufferDesc, NULL, &waterPSBuffer);

	// vertex buffer water settings
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(SceneBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &sceneBuffer);

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

}
