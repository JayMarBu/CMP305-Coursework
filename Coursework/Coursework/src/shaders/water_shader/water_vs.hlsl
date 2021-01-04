// texture vertex shader
// Basic shader for rendering textured geometry

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer WaterBuffer : register(b1)
{
	float tiling;
	float3 eye_pos;
	float3 light_pos;
	float padding;
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
	float4 clipspace_coords : TEXCOORD1;
	float3 cam_to_pos_vector : TEXCOORD2;
	float3 pos_to_light_vector : TEXCOORD3;
};

OutputType main(InputType input)
{
	OutputType output;

	float4 world_pos = mul(input.position, worldMatrix);
	
	output.position = world_pos;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	
	output.clipspace_coords = output.position;
	
	output.tex.x = input.tex.x*tiling;
	output.tex.y = input.tex.y*tiling;
	
	output.cam_to_pos_vector = eye_pos - world_pos.xyz;
	
	output.pos_to_light_vector = world_pos.xyz - light_pos;
	
	return output;
}