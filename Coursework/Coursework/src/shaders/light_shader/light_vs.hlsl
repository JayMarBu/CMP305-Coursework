// Light vertex shader

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
	float3 camerPosition;
	float padding;
};

cbuffer ClipPlaneBuffer : register(b2)
{
	float4 clip_plane;
}

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
	float4 viewPos : TEXCOORD2;
	float clip : SV_ClipDistance0;
	float height : TEXCOORD3;
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
	
	output.viewPos = output.position;
	
	output.clip = dot(mul(input.position, worldMatrix), clip_plane);
	
	output.height = input.position.y;

	return output;
}