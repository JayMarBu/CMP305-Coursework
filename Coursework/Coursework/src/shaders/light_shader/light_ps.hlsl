// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D entity_texture0 : register(t0);
Texture2D entity_texture1 : register(t1);
Texture2D entity_texture2 : register(t2);
Texture2D entity_texture3 : register(t3);
Texture2D shadow_texture : register(t4);

SamplerState diffuse_sampler : register(s0);
SamplerState shadow_sampler : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 ambient_colour;
	
	float4 diffuse;
	
	float3 direction;
	
	float water_level;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewVector : TEXCOORD1;
	float4 viewPos : TEXCOORD2;
	float clip : SV_ClipDistance0;
	float height : TEXCOORD3;
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
	//float4 specular_colour = calcSpecular(-direction, input.normal, input.viewVector, spec_colour, specular_power);
	
	colour = (ambient_colour + shadow_colour * (diffuse_colour));
	
	float4 rock_colour = entity_texture0.Sample(diffuse_sampler, input.tex);
	float4 moss_colour = entity_texture1.Sample(diffuse_sampler, input.tex);
	float4 grass_colour = entity_texture2.Sample(diffuse_sampler, input.tex);
	float4 sand_colour = entity_texture3.Sample(diffuse_sampler, input.tex);
	
	// Calculate the slope of this point.
	float slope = 1.0f - input.normal.y;
	float blendAmount;
	if (slope < 0.8)
	{
		blendAmount = slope / 0.7f;
		if (input.height < water_level)
		{
			textureColour = sand_colour;
		}
		else if (input.height < water_level + 1.f)
		{
			textureColour = lerp(sand_colour, grass_colour, input.height - water_level);
		}
		else
		{
			textureColour = grass_colour;
		}
	}
	if ((slope < 0.9) && (slope >= 0.7f))
	{
		blendAmount = (slope - 0.7f) * (1.0f / (0.9f - 0.7f));
		
		if (input.height < water_level)
		{
			textureColour = lerp(sand_colour, rock_colour, blendAmount);
		}
		else if (input.height < water_level + 1.f)
		{
			textureColour = lerp(lerp(sand_colour, grass_colour, input.height - water_level), rock_colour, blendAmount);
		}
		else
		{
			textureColour = lerp(grass_colour, rock_colour, blendAmount);
		}
		
	}

	if (slope >= 0.9)
	{
		textureColour = rock_colour;
	}
	
	colour = colour * textureColour;

	return colour;
}