// depth shader.cpp
#include "depth_shader.h"

DepthShader::DepthShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"res/shaders/depth_vs.cso", L"res/shaders/depth_ps.cso");
}

DepthShader::~DepthShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// release clip plane buffer
	if (clipPlaneBuffer)
	{
		clipPlaneBuffer->Release();
		clipPlaneBuffer = 0;
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

void DepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
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
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	clipPlaneBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	clipPlaneBufferDesc.ByteWidth = sizeof(XMFLOAT4);
	clipPlaneBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	clipPlaneBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	clipPlaneBufferDesc.MiscFlags = 0;
	clipPlaneBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&clipPlaneBufferDesc, NULL, &clipPlaneBuffer);
}

void DepthShader::setShaderParameters(gpfw::ShaderInfo s_info, XMFLOAT4 clip_plane)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(s_info.world);
	XMMATRIX tview = XMMatrixTranspose(s_info.view);
	XMMATRIX tproj = XMMatrixTranspose(s_info.projection);

	// Lock the constant buffer so it can be written to.
	s_info.d_info.context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	s_info.d_info.context->Unmap(matrixBuffer, 0);
	s_info.d_info.context->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// send clip plane buffer to shader
	XMFLOAT4* cliPlanePtr;
	s_info.d_info.context->Map(clipPlaneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cliPlanePtr = (XMFLOAT4*)mappedResource.pData;
	cliPlanePtr->x = clip_plane.x;
	cliPlanePtr->y = clip_plane.y;
	cliPlanePtr->z = clip_plane.z;
	cliPlanePtr->w = clip_plane.w;
	s_info.d_info.context->Unmap(clipPlaneBuffer, 0);
	s_info.d_info.context->VSSetConstantBuffers(1, 1, &clipPlaneBuffer);
}

void DepthShader::Render(gpfw::ShaderInfo s_info, gpfw::Entity e, XMFLOAT4 clip_plane)
{
	s_info.world = gpfw::entity::GetTransformMatrix(e);
	e.mesh->sendData(s_info.d_info.context);
	setShaderParameters(
		s_info,
		clip_plane
	);
	render(s_info.d_info.context, e.mesh->getIndexCount());
}

void DepthShader::Render(gpfw::ShaderInfo s_info, gpfw::TerrainChunk t, XMFLOAT4 clip_plane)
{
	s_info.world = gpfw::terrain::GetTransformMatrix(t);
	t.mesh->sendData(s_info.d_info.context);
	setShaderParameters(
		s_info,
		clip_plane
	);
	render(s_info.d_info.context, t.mesh->getIndexCount());
}
