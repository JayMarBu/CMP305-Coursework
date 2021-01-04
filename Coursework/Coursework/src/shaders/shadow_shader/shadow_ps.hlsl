// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

static const int cascade_number = 3;

Texture2D depthmap_texture[cascade_number] : register(t0);

SamplerState shadow_sampler : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambient_colour;
};

cbuffer ShadowBuffer : register(b1)
{
	float4 cascade_depths;
	float near;
	float far;
	bool render_cascades;
	float padding;
	float4 depth_map_size;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewVector : TEXCOORD1;
	float4 lightViewPos[cascade_number] : TEXCOORD2;
};

bool hasDepthData(float2 uv)
{
	return !(uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f);
}

float rand(float n){return frac(sin(n)*43758.5453123);}

float simple2DNoise(float2 n)
{
	const float2 d = float2(0.0, 1.0);
	float2 b = floor(n);
	float2 f = smoothstep(float2(0,0), float2(1,0), frac(n));
	return lerp(lerp(rand(b), rand(b + d.yx), f.x), lerp(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

// PCF Shadows
float3 getShadowColour(Texture2D s_map, float depth_map_s, float s_map_depth, float2 uv, float4 l_view_pos, float bias)
{
	float texel_size = 1.0f / (depth_map_s);
	
	// create sample coordinates
	float2 shadow_coords[9];
	shadow_coords[0] = uv;
	shadow_coords[1] = uv + float2(-texel_size, 0);
	shadow_coords[2] = uv + float2(texel_size, 0);
	shadow_coords[3] = uv + float2(0, -texel_size);
	shadow_coords[4] = uv + float2(0, texel_size);
	shadow_coords[5] = uv + float2(-texel_size, -texel_size);
	shadow_coords[6] = uv + float2(texel_size, -texel_size);
	shadow_coords[7] = uv + float2(-texel_size, texel_size);
	shadow_coords[8] = uv + float2(texel_size, texel_size);

	// randomize sample coordinates
	float3 fShadowTerms[9];
	float3 PCF_shadow = 0.0f;
	for (int i = 0; i < 9; i++)
	{
		float2 randomised_uv = float2(
		shadow_coords[i].x + (normalize(simple2DNoise(shadow_coords[i])) * (texel_size)),
		shadow_coords[i].y + (normalize(simple2DNoise(shadow_coords[i])) * (texel_size))
		);

		// sample from depth map
		float depth_value = s_map.Sample(shadow_sampler, randomised_uv).r;
		float l_depth_value = l_view_pos.z / l_view_pos.w;
		l_depth_value -= bias;
		
		// add to total
		fShadowTerms[i] = (l_depth_value > depth_value) ? ambient_colour : float3(1, 1, 1);
		PCF_shadow += fShadowTerms[i];
	}
	
	// average total
	PCF_shadow /= 9.0f;
	
	return PCF_shadow;
}

float2 getProjectiveCoords(float4 l_view_pos)
{
	float2 prodj_tex = l_view_pos.xy / l_view_pos.w;
	prodj_tex *= float2(0.5f, -0.5f);
	prodj_tex += float2(0.5f, 0.5f);
	return prodj_tex;
}

float4 main(InputType input):SV_TARGET
{
	
	float shadow_bias = 0.005f;
	float4 textureColour;
	float4 output_colour = float4(1, 1, 1, 1);
	
	float depth = input.position.w;
	
	float2 p_tex_coord;
	
	// determine cascade region
	if (depth <= far * cascade_depths[0])
	{
		// get projected coordinates
		p_tex_coord = getProjectiveCoords(input.lightViewPos[0]);
		
		
		if (hasDepthData(p_tex_coord))
		{
			// sample the depth texture
			float4 shadow_colour;
			shadow_colour.xyz = getShadowColour(depthmap_texture[0], depth_map_size[0],cascade_depths[0], p_tex_coord, input.lightViewPos[0], shadow_bias);
			output_colour.xyz = shadow_colour.xyz;
		}
		
		// render debug colour
		if (render_cascades)
			output_colour *= float4(1, 0, 0, 1);
	}
	else if (depth > far * cascade_depths[0] && depth <= far * cascade_depths[1])
	{
		// determine cascade region
		p_tex_coord = getProjectiveCoords(input.lightViewPos[1]);
		
		if (hasDepthData(p_tex_coord))
		{
			// sample the depth texture
			float4 shadow_colour;
			shadow_colour.xyz = getShadowColour(depthmap_texture[1], depth_map_size[1], cascade_depths[1], p_tex_coord, input.lightViewPos[1], shadow_bias);
			output_colour.xyz = shadow_colour.xyz;
		}
		
		// render debug colour
		if (render_cascades)
			output_colour *= float4(0, 1, 0, 1);
	}
	else if (depth > far * cascade_depths[1] && depth <= far * cascade_depths[2])
	{
		// determine cascade region
		p_tex_coord = getProjectiveCoords(input.lightViewPos[2]);
		
		if (hasDepthData(p_tex_coord))
		{
			// sample the depth texture
			float4 shadow_colour;
			shadow_colour.xyz = getShadowColour(depthmap_texture[2], depth_map_size[2], cascade_depths[1], p_tex_coord, input.lightViewPos[2], shadow_bias);
			output_colour.xyz = shadow_colour.xyz;
		}
		
		// render debug colour
		if (render_cascades)
			output_colour *= float4(0, 0, 1, 1);
	}
	else
	{
		output_colour = float4(1, 1, 1, 0);
	}
	return output_colour;
}