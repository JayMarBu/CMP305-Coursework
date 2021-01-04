#pragma once
#include "DXF.h"
#include <string>

namespace gpfw
{
	// TRANSFORM STRUCT .............................................................................................................................
	struct Transform
	{
		// trivial transform values
		XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
		XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };

		// complex transform values
		// derived from the trivial values
		XMMATRIX matrix;
		XMVECTOR quaternion;
	};

	// ENTITY STRUCT ................................................................................................................................
	struct Entity
	{
		// mesh pointer
		BaseMesh* mesh;

		// texture identifier string
		const wchar_t* texture = L"Default";

		// transform to hold entity's world position
		Transform transform;

		// name for debug purposes
		// TODO: probably redundant now
		const char* name;

		// pointer to application class' texture manager
		// for ease of access
		TextureManager* texture_manager;
	};

	// ENTITY RELATED METHODS .......................................................................................................................
	namespace entity
	{
		// ENTITY CREATION AND DESTRUCTION METHODS ................................................
		Entity CreateEntity(BaseMesh* mesh, const char* name, TextureManager* texture_manager, const wchar_t* texture = L"Default");
		Entity* CreateEntityP(BaseMesh* mesh, const char* name, TextureManager* texture_manager, const wchar_t* texture = L"Default");

		void ReleaseEntity(Entity& e);
		void ReleaseEntity(Entity* e);

		// NON-TRIVIAL GETTERS & SETTERS ..........................................................
		inline ID3D11ShaderResourceView* GetTexture(Entity e) { return e.texture_manager->getTexture(e.texture); }

		inline void SetPosition(Entity& e, XMFLOAT3 p) { e.transform.position = p; }
		inline void SetPosition(Entity& e, float x, float y, float z) { e.transform.position = { x,y,z }; }

		inline void SetRotation(Entity& e, XMFLOAT3 r);
		inline void SetRotation(Entity& e, float x, float y, float z);

		inline void SetScale(Entity& e, XMFLOAT3 s) { e.transform.scale = s; }
		inline void SetScale(Entity& e, float x, float y, float z) { e.transform.scale = { x,y,z }; }

		// TRANSFORMATION METHODS .................................................................
		inline XMMATRIX MatScale(Entity& e) { return XMMatrixScaling(e.transform.scale.x, e.transform.scale.y, e.transform.scale.z); }
		inline XMMATRIX MatScale(Transform& t) { return XMMatrixScaling(t.scale.x, t.scale.y, t.scale.z); }

		inline XMMATRIX MatPosition(Entity& e) { return XMMatrixTranslation(e.transform.position.x, e.transform.position.y, e.transform.position.z); }
		inline XMMATRIX MatPosition(Transform& t) { return XMMatrixTranslation(t.position.x, t.position.y, t.position.z); }

		inline XMMATRIX MatRotation(Entity& e) { return XMMatrixRotationQuaternion(e.transform.quaternion); }
		inline XMMATRIX MatRotation(Transform& t) { return XMMatrixRotationQuaternion(t.quaternion); }

		XMVECTOR ToQuaternion(XMFLOAT3);
		XMVECTOR ToQuaternion(float x, float y, float z);

		XMMATRIX GetTransformMatrix(Entity& e);
	}

	// WATER INFORMATION STRUCT .....................................................................................................................
	struct WaterInfo
	{
		// RENDERABLE MEMBERS .....................................................................
		// entity representing water plane
		Entity water_plane;

		// render textures and texture id strings
		// for the normal and derivitive normal maps
		RenderTexture* reflection_texture;
		RenderTexture* refraction_texture;
		const wchar_t* DuDv_map;
		const wchar_t* normal_map;

		// VARIABLE MEMBERS .......................................................................
		// the height of the water in world space
		float water_level = 3.0f;

		// the texture scale for the water's
		// normal and derivative normal maps
		int tiling = 30.0f;

		// the magnitude of the distortion
		// applied by the distortion maps
		float distortion_strength = 0.1f;

		// the scrolling and warping speed
		// of the distortion
		XMFLOAT2 move_speed = XMFLOAT2(0.03f, 0.04f);

		// the current offset of the distortion textures
		// defined by the speed * delta time
		XMFLOAT2 move_factor;

		// the ratio to blend the reflected and
		// refracted textures together to influence the strength
		// of the Fresnel effect
		float reflection_strength = 0.5f;

		// the strength of specular highlights on the waters
		// surface
		float reflectivity = 0.6f;

		// the colour of the water and how intensely the
		// colour blends with the reflections
		// & refractions
		float colour_intensity = 0.2f;
		XMFLOAT3 water_colour;

		// offset for the clipping plane to avoid
		// distortion around coast lines
		float ref_offset = 0.2f;


		// WATER RELATED METHODS ..................................................................
		// method to raise water level
		void setWaterLevel(float w)
		{
			water_level = w;
			entity::SetPosition(water_plane, 0, water_level, 0);
		}

		// method to release shader resources
		void ReleaseRenderTextures()
		{
			if (reflection_texture != nullptr)
			{
				delete reflection_texture;
				reflection_texture = nullptr;
			}

			if (refraction_texture != nullptr)
			{
				delete refraction_texture;
				refraction_texture = nullptr;
			}
		}
	};
}



