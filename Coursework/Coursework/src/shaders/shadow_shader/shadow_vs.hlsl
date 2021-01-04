// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

static const int cascade_number = 3;

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	
	matrix lightViewMatrix[cascade_number];
	matrix lightProjectionMatrix[cascade_number];
};

cbuffer CameraBuffer : register(b1)
{
	float3 camerPosition;
	float padding;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewVector : TEXCOORD1;
	float4 lightViewPos[cascade_number] : TEXCOORD2;
};

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3) worldMatrix);
	output.normal = normalize(output.normal);
	
	float4 worldPosition = mul(input.position, worldMatrix);
	
	output.viewVector = camerPosition.xyz - worldPosition.xyz;
	output.viewVector = normalize(output.viewVector);
	
	// Calculate the position of the vertice as viewed by the light source.
	for (int i = 0; i < cascade_number; i++)
	{
		output.lightViewPos[i] = mul(input.position, worldMatrix);
		output.lightViewPos[i] = mul(output.lightViewPos[i], lightViewMatrix[i]);
		output.lightViewPos[i] = mul(output.lightViewPos[i], lightProjectionMatrix[i]);
	}
	
	

	return output;
}