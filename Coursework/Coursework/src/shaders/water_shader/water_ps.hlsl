SamplerState water_sampler : register(s0);

Texture2D reflection_texture : register(t0);
Texture2D refraction_texture : register(t1);
Texture2D DuDv_map_texture : register(t2);
Texture2D normal_map_texture : register(t3);

cbuffer WaterBuffer : register(b0)
{
	float wave_strength;
	float2 move_factor;
	float colour_intensity;
	float reflection_strength;
	float3 water_colour;
}

cbuffer SceneBuffer : register(b1)
{
	float4 light_colour;
	float specular_power;
	float3 padding;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 clipspace_coords : TEXCOORD1;
	float3 cam_to_pos_vector : TEXCOORD2;
	float3 pos_to_light_vector : TEXCOORD3;
};

float4 calcSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4 specularColour, float specularPower)
{
	// blinn-phong specular calculation
	float3 halfway = normalize(lightDirection + viewVector);
	float specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower);
	return saturate(specularColour * specularIntensity);
}

float4 main(InputType input) : SV_TARGET
{
	// GET NORMALISED DEVICE COORDINATES ............................................................................................................
	float2 ndc;
	ndc.x = ((input.clipspace_coords.x / input.clipspace_coords.w) / 2) + 0.5;
	ndc.y = ((input.clipspace_coords.y / input.clipspace_coords.w) / 2) + 0.5;
	
	// DISTORT NDC ..................................................................................................................................
	float2 distoretedUV = DuDv_map_texture.Sample(water_sampler, float2(input.tex.x + move_factor.x, input.tex.y)).rg * 2.0 - 1.0;
	distoretedUV = input.tex + float2(distoretedUV.x, distoretedUV.y + move_factor.y);
	float2 final_distortion = (DuDv_map_texture.Sample(water_sampler, distoretedUV).rg * 2.0 - 1.0)*wave_strength;
	
	// CREATE UV MAPS ...............................................................................................................................
	float2 reflection_uv = clamp(ndc + final_distortion,0.001,0.999);
	float2 refraction_uv = clamp(float2(ndc.x, 1 - ndc.y) + final_distortion, 0.001, 0.999);
	
	// SAMPLE REFLECTION AND REFRACTION TEXTURES ....................................................................................................
	float4 reflection = reflection_texture.Sample(water_sampler, reflection_uv);
	float4 refraction = refraction_texture.Sample(water_sampler, refraction_uv);
	
	// SAMPLE AND GENERATE NORMALS ..................................................................................................................
	float4 normal_map_colour = normal_map_texture.Sample(water_sampler, distoretedUV);
	float3 normal = float3(normal_map_colour.r * 2.0 - 1.0, normal_map_colour.b, normal_map_colour.g * 2.0 - 1.0);
	normal = normalize(normal);
	
	// SIMULATE FRESNAL EFFECT ......................................................................................................................
	float3 view_vector = normalize(input.cam_to_pos_vector);
	float reflection_balance = dot(view_vector, float3(0,1,0));
	reflection_balance = pow(reflection_balance, reflection_strength);
	
	// GENERATE SPECULAR HIGHLIGHTS .................................................................................................................
	float3 specular_highlights = calcSpecular(normalize(input.pos_to_light_vector), normal, view_vector, light_colour, specular_power).rgb;
	
	// COMBINE ALL DATA TO CREATE FINAL COLOUR ......................................................................................................
	float4 out_colour = lerp(lerp(reflection, refraction, reflection_balance), float4(water_colour, 1), colour_intensity) + float4(specular_highlights, 0);
	return out_colour;

}