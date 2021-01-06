#include "noise_shader.h"

NoiseShader::NoiseShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"res/shaders/noise_vs.cso", L"res/shaders/noise_ps.cso");
}

NoiseShader::~NoiseShader()
{

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the noise constant buffer.
	if (noiseBuffer)
	{
		noiseBuffer->Release();
		noiseBuffer = 0;
	}

	if (perm_texture_)
	{
		perm_texture_->Release();
		perm_texture_ = 0;
	}

	if (perm_srv_)
	{
		perm_srv_->Release();
		perm_srv_ = 0;
	}

	if (perm_sampler_)
	{
		perm_sampler_->Release();
		perm_sampler_ = 0;
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

void NoiseShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, NoiseParameters n_params)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(world);
	tview = XMMatrixTranspose(view);
	tproj = XMMatrixTranspose(projection);

	// Send matrix data
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// send water ps buffer to shader
	ParametersInputType* noisePtr;
	deviceContext->Map(noiseBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	noisePtr = (ParametersInputType*)mappedResource.pData;
	noisePtr->octave_offsets_0 = n_params.octave_offsets[0];
	noisePtr->octave_offsets_1 = n_params.octave_offsets[1];
	noisePtr->octave_offsets_2 = n_params.octave_offsets[2];
	noisePtr->octave_offsets_3 = n_params.octave_offsets[3];
	noisePtr->octave_offsets_4 = n_params.octave_offsets[4];
	noisePtr->octave_offsets_5 = n_params.octave_offsets[5];
	noisePtr->octave_offsets_6 = n_params.octave_offsets[6];
	noisePtr->octave_offsets_7 = n_params.octave_offsets[7];
	noisePtr->octave_offsets_8 = n_params.octave_offsets[8];
	noisePtr->octave_offsets_9 = n_params.octave_offsets[9];

	noisePtr->offset = XMFLOAT2(n_params.offset[0], n_params.offset[1]);
	noisePtr->scale = n_params.scale;
	noisePtr->octaves = n_params.octaves;
	noisePtr->persistance = n_params.persistance;
	noisePtr->lacunarity = n_params.lacunarity;
	noisePtr->padding = XMFLOAT2(0,0);
	deviceContext->Unmap(noiseBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 2, &noiseBuffer);

	deviceContext->PSSetShaderResources(0, 1, &perm_srv_);

	deviceContext->PSSetSamplers(0, 1, &perm_sampler_);
}

void NoiseShader::Render(gpfw::ShaderInfo s_info, OrthoMesh* o_mesh, NoiseParameters n_params)
{
	o_mesh->sendData(s_info.d_info.context);
	setShaderParameters(s_info.d_info.context, s_info.world, s_info.view, s_info.projection, n_params);
	render(s_info.d_info.context, o_mesh->getIndexCount());
}

void NoiseShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	// buffer descriptions
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC noiseBufferDesc;

	// texture & sampler descriptions
	D3D11_TEXTURE1D_DESC perm_tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC perm_srv_desc;
	D3D11_SAMPLER_DESC perm_sampler_desc;

	D3D11_TEXTURE1D_DESC grad_tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC grad_srv_desc;
	D3D11_SAMPLER_DESC grad_sampler_desc;

	HRESULT result;

	// Load (+ compile) shader files
	loadVertexShader(vs);
	loadPixelShader(ps);

	// CONSTANT BUFFER INITIALISATION ...........................................................................................

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Setup the description of the parameters constant buffer
	noiseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	noiseBufferDesc.ByteWidth = sizeof(ParametersInputType);
	noiseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	noiseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	noiseBufferDesc.MiscFlags = 0;
	noiseBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&noiseBufferDesc, NULL, &noiseBuffer);

	// PERMUTATION TEXTURE INITIALISATION .......................................................................................

	ZeroMemory(&perm_tex_desc, sizeof(perm_tex_desc));

	float p[256] = { 151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	for (int i = 0; i < 256; i++)
	{
		p[i] = p[i] / 255;
	}

	D3D11_SUBRESOURCE_DATA perm_init_data;
	perm_init_data.pSysMem = p;
	perm_init_data.SysMemPitch = 0;

	// Setup the texture description.
	perm_tex_desc.Width = 256;
	perm_tex_desc.MipLevels = 1;
	perm_tex_desc.ArraySize = 1;
	perm_tex_desc.Format = DXGI_FORMAT_R32_FLOAT;
	perm_tex_desc.Usage = D3D11_USAGE_DEFAULT;
	//perm_tex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perm_tex_desc.CPUAccessFlags = 0;
	perm_tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	perm_tex_desc.MiscFlags = 0;
	// Create the render target texture.
	result = renderer->CreateTexture1D(&perm_tex_desc, &perm_init_data, &perm_texture_);

	// Setup the description of the shader resource view.
	perm_srv_desc.Format = perm_tex_desc.Format;
	perm_srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	perm_srv_desc.Texture2D.MostDetailedMip = 0;
	perm_srv_desc.Texture2D.MipLevels = 1;
	// Create the shader resource view.
	result = renderer->CreateShaderResourceView(perm_texture_, &perm_srv_desc, &perm_srv_);

	// Create a texture sampler state description.
	perm_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	perm_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	perm_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	perm_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	perm_sampler_desc.MipLODBias = 0.0f;
	perm_sampler_desc.MaxAnisotropy = 1;
	perm_sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	perm_sampler_desc.MinLOD = 0;
	perm_sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&perm_sampler_desc, &perm_sampler_);
}

