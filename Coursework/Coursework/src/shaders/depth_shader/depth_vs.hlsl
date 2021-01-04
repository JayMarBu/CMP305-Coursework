cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer ClipPlane : register(b1)
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
	float4 depthPosition : TEXCOORD0;
	float clip : SV_ClipDistance0;
};

OutputType main(InputType input)
{
	OutputType output;

    // Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
	output.depthPosition = output.position;
	
	output.clip = dot(mul(input.position, worldMatrix), clip_plane);
	
	return output;
}