// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D entity_texture : register(t0);
Texture2D shadow_texture : register(t1);

SamplerState diffuse_sampler : register(s0);
SamplerState shadow_sampler : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 ambient_colour;
	
	float4 diffuse;
	
	float3 direction;
	float specular_power;
	
	float4 spec_colour;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewVector : TEXCOORD1;
	float4 viewPos : TEXCOORD2;
	float clip : SV_ClipDistance0;
};

float4 calcSpecular(float3 lightDirection, float3 normal, float3 viewVector, float4 specularColour, float specularPower)
{
	// blinn-phong specular calculation
	float3 halfway = normalize(lightDirection + viewVector);
	float specularIntensity = pow(max(dot(normal, halfway), 0.0), specularPower);
	return saturate(specularColour * specularIntensity);
}

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(diffuse * intensity);
	return colour;
}

float2 getProjectiveCoords(float4 l_view_pos)
{
	float2 prodj_tex = l_view_pos.xy / l_view_pos.w;
	prodj_tex *= float2(0.5f, -0.5f);
	prodj_tex += float2(0.5f, 0.5f);
	return prodj_tex;
}

float4 main(InputType input) : SV_TARGET
{
	float2 projectTexCoord;
	float4 colour = ambient_colour;
	float4 textureColour;
	
	projectTexCoord = getProjectiveCoords(input.viewPos);
	float4 shadow_colour = shadow_texture.Sample(shadow_sampler, projectTexCoord);
	float4 diffuse_colour = calculateLighting(-direction, input.normal, diffuse);
	float4 specular_colour = calcSpecular(-direction, input.normal, input.viewVector, spec_colour, specular_power);
	
	colour = (ambient_colour + shadow_colour * (diffuse_colour + specular_colour));
	
	textureColour = entity_texture.Sample(diffuse_sampler, input.tex);
	colour = colour * textureColour;

	return colour;
}