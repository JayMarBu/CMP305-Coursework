#define TERRAIN_SIZE 256
#define LIST_INDEX (dt_id.y * TERRAIN_SIZE) + dt_id.x

// vertex structure
struct Vertex
{
	float3 position;
	float2 texture_coords;
	float3 normal;
};

// parameters for creating terrain
cbuffer terrain_parameters_buffer : register(b0)
{
	float2 data_min;
	float2 data_max;
}

// inputs
Texture2D in_heightmap : register(t0);
StructuredBuffer<Vertex> in_vertices : register(t1);
SamplerState in_sampler : register(s1);

// output
RWStructuredBuffer<Vertex> out_vertices : register(u0);

float2 NormalizeMeshCoord(float2 list_index, float2 data_min, float2 data_max)
{
	/*
	standard formula for normalising data
	
				 x[i] - min(x)
		n[i] = -----------------
				max(x) - min(x)
	
		where:
		x = the list of values
		i = index to list of values
		min(x) = the lowest bound of x
		max(x) = the highest bound of x
		n[i] = the nromalised equivelent of x[i]
	*/
	
	float2 n;
	
	n.x = (list_index.x - data_min.x) / (data_max.x - data_min.x);
	n.y = (list_index.y - data_min.y) / (data_max.y - data_min.y);
	
	return n;

}

[numthreads(TERRAIN_SIZE / 16, TERRAIN_SIZE / 16, 1)]
void main(uint3 gt_id : SV_GroupThreadID, uint3 dt_id : SV_DispatchThreadID)
{
	// GENERATE VERTICES ............................................................................................................................
	// set initial vertex positions
	out_vertices[LIST_INDEX].position = in_vertices[LIST_INDEX].position;
	
	// normalise the coordinates of the mesh into texture space
	// in order to map the height texture to the mesh
	float2 out_height_uv = NormalizeMeshCoord(in_vertices[LIST_INDEX].position.xz, data_min, data_max);
	
	// if the data is within texture space	
	if ((out_height_uv.x >= 0 && out_height_uv.x <= 1) && (out_height_uv.y >= 0 && out_height_uv.y <= 1))
	{
		// set the height of the vertex from the height map
		float4 sampled_heightmap_point = in_heightmap.SampleLevel(in_sampler, out_height_uv, 0);
		out_vertices[LIST_INDEX].position.y = 10*sampled_heightmap_point.r;
	}
	
	// GENERATE NORMALS .............................................................................................................................
	float mapSize = 256;

	float texelSize = 1.0 / (mapSize);
	
	float4 h;
	h[0] = in_heightmap.SampleLevel(in_sampler, out_height_uv + texelSize * float2(0, -1), 0).r * float4(1,10,1,1); // top
	h[1] = in_heightmap.SampleLevel(in_sampler, out_height_uv + texelSize * float2(-1, 0), 0).r * float4(1, 10, 1, 1); //left
	h[2] = in_heightmap.SampleLevel(in_sampler, out_height_uv + texelSize * float2(1, 0), 0).r * float4(1, 10, 1, 1); // right
	h[3] = in_heightmap.SampleLevel(in_sampler, out_height_uv + texelSize * float2(0, 1), 0).r * float4(1, 10, 1, 1); // bottom
	
	float3 n;
	n.z = (h[0] - h[3]);
	n.x = (h[1] - h[2]);
	n.y = 2 * texelSize;
	
	out_vertices[LIST_INDEX].normal = normalize(n);//

	// GENERATE TEXTURE COORDINATES .................................................................................................................
	out_vertices[LIST_INDEX].texture_coords = in_vertices[LIST_INDEX].texture_coords;
}