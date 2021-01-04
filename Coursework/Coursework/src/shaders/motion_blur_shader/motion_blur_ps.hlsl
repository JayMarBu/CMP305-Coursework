// Texture and sampler registers
Texture2D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer ScreenBufferSize : register(b0)
{
	float2 blur_direction;
	float2 screen_size;
	int distance;
	float3 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
	
	 // Initialize the colour to black.
	float4 colour = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float2 texelSize = 1.0f / screen_size;
    // Add the horizontal pixels to the colour by the specific weight of each.
	for (int i = 0; i < distance; i++)
	{
		colour += texture0.Sample(Sampler0, input.tex + ((blur_direction * texelSize) * i));
	}

	colour /= distance;
    // Set the alpha channel to one.
	colour.a = 1.0f;

	return colour;
}