Texture1D perm_texture : register(t0);

SamplerState perm_sampler : register(s0);

cbuffer NoiseParameters : register(b0)
{
	//16*10 bytes
	float2 octave_offsets_0;
	float2 octave_offsets_1;
	
	float2 octave_offsets_2;
	float2 octave_offsets_3;
	
	float2 octave_offsets_4;
	float2 octave_offsets_5;
	
	float2 octave_offsets_6;
	float2 octave_offsets_7;
	
	float2 octave_offsets_8;
	float2 octave_offsets_9;
	
	// 16 bytes
	float2 offset;
	float scale;
	int octaves;
	
	// 16 bytes
	float persistance;
	float lacunarity;
	float2 padding;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 screen_pos : TEXCOORD1;
};

// IMPROVED PERLIN NOISE GPU IMPLEMENTATION .........................................................................................................
float2 fade(float2 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}    

int perm(float x)  
{    
	// sample from permutations texture
	float perm_sample_location = x / 256.0;
	float perm_result = perm_texture.Sample(perm_sampler, perm_sample_location).r;
	int perm_return = floor(perm_result * 256);
	return perm_return;
}    

float grad(int x, float2 p)  
{
	int h = x & 15;
	float u = h < 4 ? p.x : p.y;
	float v = h < 4 ? p.y : p.x;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}    

// Improved Perlin noise 
float inoise(float2 p)  
{
	// define integer part of input position
	float2 P = fmod(floor(p), 256.0);
	
	// define the fractional part of input position
	p -= floor(p);
	
	// prevent second order discontinuities using 
	// the polinomial:  6t^5 - 15t^4 + 10t^3
	float2 f = fade(p); 
	
	// Hash coordinates for corners of unit square
	float A =	perm(P.x    ) + P.y;	float AA = perm(A);    float AB = perm(A + 1);
	float B =	perm(P.x + 1) + P.y;    float BA = perm(B);    float BB = perm(B + 1); 
	
	// blend the four corners together
	return
	lerp(
		lerp(
			grad(perm(AA), p),
			grad(perm(BA), p + float2(-1, 0)),
		f.x),
		lerp(
			grad(perm(AB), p + float2(0, -1)),
			grad(perm(BB), p + float2(-1, -1)),
		f.x),
	f.y);
} 

float normaliseHeight(float data, float data_min, float data_max)
{
	if (data < data_min)
		data = data_min;
	
	if (data > data_max)
		data = data_max;
	
	return (data - data_min) / (data_max - data_min);
}

// MAIN FUNCTION ....................................................................................................................................
float4 main(InputType input) : SV_TARGET
{
	float2 octave_offsets[10] =
	{
		octave_offsets_0,
		octave_offsets_1,
	
		octave_offsets_2,
		octave_offsets_3,
	
		octave_offsets_4,
		octave_offsets_5,
	
		octave_offsets_6,
		octave_offsets_7,
	
		octave_offsets_8,
		octave_offsets_9
	};
	
	// create normalised device coordinates
	float2 ndc;
	ndc.x = (((input.screen_pos.x / input.screen_pos.w) / 2) + 0.5);
	ndc.y = (((-input.screen_pos.y / input.screen_pos.w) / 2) + 0.5);
	
	
	
	float max_noise_height = 0;

	float min_noise_height = 0;
	
	float amplitude = 1;
	float freaquency = 1;

	float noise_height = 0;
	
	for (int i = 0; i < octaves; i++)
	{
		// apply user defined variables
		float2 noise_inputs = ndc / scale * freaquency + octave_offsets[i] + offset;
		//noise_inputs.x += offset.x;
		//noise_inputs.y += offset.y;
	
		// generate noise
		float noise_val = inoise(noise_inputs);
		float2 max_min_noise_val = float2(-0.8, 0.8);
		
		min_noise_height += max_min_noise_val.x * amplitude;
		max_noise_height += max_min_noise_val.y * amplitude;
		noise_height += noise_val * amplitude;
		amplitude *= persistance;
		freaquency *= lacunarity;
	}
	
	//noise_height = (noise_height + 1) / 2;
	
	noise_height = normaliseHeight(noise_height, min_noise_height, max_noise_height);
	
	return float4(noise_height, noise_height, noise_height, 1);
}

