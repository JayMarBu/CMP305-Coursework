Texture1D perm_texture : register(t0);
Texture1D grad_texture : register(t1);

SamplerState perm_sampler : register(s0);
SamplerState grad_sampler : register(s1);

cbuffer NoiseParameters : register(b0)
{
	float2 offset;
	float scale;
	float padding;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 position2 : TEXCOORD1;
};

// GPU GEMS IMPLEMENTATION ..........................................................................................................................
float3 fade(float3 t)
{
	return t * t * t * (t * (t * 6 - 15) + 10); // new curve
	//  return t * t * (3 - 2 * t); // old curve  
}    

int perm(float x)  
{    
	float perm_sample_location = x / 256.0;
	float perm_result = perm_texture.Sample(perm_sampler, perm_sample_location).r;
	int perm_return = floor(perm_result * 256);
	return perm_return;
	//return perm_texture.Sample(perm_sampler, perm_sample_location) * 256;
	//return perm_texture.Sample(perm_sampler, x);
}    

float grad(int x, float3 p)  
{    
	//return grad_texture.Sample(grad_sampler, (x % 15)).xyz;
	//return dot(grad_texture.Sample(grad_sampler, x).xyz, p);
	int h = x & 15;
	float u = h < 4 ? p.x : p.y;
	float v = h < 4 ? p.y : p.x;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}    

// 3D version  
float inoise(float3 p)  
{
	float3 P = fmod(floor(p), 256.0);
	p -= floor(p);
	float3 f = fade(p); 
	
	// HASH COORDINATES FOR 6 OF THE 8 CUBE CORNERS
	float A = perm(P.x) + P.y;    float AA = perm(A);    float AB = perm(A + 1);
	float B =  perm(P.x + 1) + P.y;    float BA = perm(B);    float BB = perm(B + 1); 
	
	// AND ADD BLENDED RESULTS FROM 8 CORNERS OF CUBE
	return
	lerp(
		lerp(
			grad(perm(AA), p),
			grad(perm(BA), p + float3(-1, 0, 0)),
		f.x),
		lerp(
			grad(perm(AB), p + float3(0, -1, 0)),
			grad(perm(BB), p + float3(-1, -1, 0)),
		f.x),
	f.y);
} 

// MAIN FUNCTION ....................................................................................................................................
float4 main(InputType input) : SV_TARGET
{
	float2 ndc;
	ndc.x = (((input.position2.x / input.position2.w) / 2) + 0.5);
	ndc.y = (((-input.position2.y / input.position2.w) / 2) + 0.5);
	
	float2 noise_inputs = ndc/scale;
	noise_inputs.x += offset.x;
	noise_inputs.y += offset.y;
	
	float noise_val = inoise(float3(noise_inputs.x,noise_inputs.y,1));
	//float noise_val = simplexNoise(noise_inputs);
	
	return float4(noise_val, noise_val, noise_val, 1);
}

