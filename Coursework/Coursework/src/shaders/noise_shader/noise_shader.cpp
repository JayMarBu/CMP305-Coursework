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

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void NoiseShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, float scale, XMFLOAT2 offset)
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
	noisePtr->offset = offset;
	noisePtr->scale = scale;
	noisePtr->padding = 0;
	deviceContext->Unmap(noiseBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &noiseBuffer);
}

void NoiseShader::Render(gpfw::ShaderInfo s_info, OrthoMesh* o_mesh, float scale, XMFLOAT2 offset)
{
	o_mesh->sendData(s_info.d_info.context);
	setShaderParameters(s_info.d_info.context, s_info.world, s_info.view, s_info.projection, scale, offset);
	render(s_info.d_info.context, o_mesh->getIndexCount());
}

void NoiseShader::initShader(const wchar_t* vs, const wchar_t* ps)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC noiseBufferDesc;

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

	noiseBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	noiseBufferDesc.ByteWidth = sizeof(ParametersInputType);
	noiseBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	noiseBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	noiseBufferDesc.MiscFlags = 0;
	noiseBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&noiseBufferDesc, NULL, &noiseBuffer);
}

