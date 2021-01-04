#include "entities/entity.h"

namespace gpfw
{
	// ENTITY NAMESPACE METHODS .....................................................................................................................
	namespace entity
	{
		// ENTITY CREATION AND DESTRUCTION METHODS ................................................
		// Create entity by reference
		Entity CreateEntity(BaseMesh* mesh, const char* name, TextureManager* texture_manager, const wchar_t* texture)
		{
			Entity e;
			e.mesh = mesh;
			e.name = name;
			e.texture_manager = texture_manager;
			e.texture = texture;

			SetPosition(e, 0.0f, 0.0f, 0.0f);
			SetRotation(e, 0.0f, 0.0f, 0.0f);
			SetScale(e, 1.0f, 1.0f, 1.0f);

			return e;
		}

		// Create entity pointer
		gpfw::Entity* CreateEntityP(BaseMesh* mesh, const char* name, TextureManager* texture_manager, const wchar_t* texture /*= L"Default"*/)
		{
			Entity* e = new Entity();
			e->mesh = mesh;
			e->name = name;
			e->texture_manager = texture_manager;
			e->texture = texture;

			SetPosition(*e, 0.0f, 0.0f, 0.0f);
			SetRotation(*e, 0.0f, 0.0f, 0.0f);
			SetScale(*e, 1.0f, 1.0f, 1.0f);

			return e;
		}

		void ReleaseEntity(Entity& e)
		{
			if (e.mesh != nullptr)
			{
				delete e.mesh;
				e.mesh = nullptr;
			}

			e.texture_manager = nullptr;
		}

		void ReleaseEntity(Entity* e)
		{
			if (e != nullptr)
			{
				if (e->mesh != nullptr)
				{
					delete e->mesh;
					e->mesh = nullptr;
				}

				e->texture_manager = nullptr;
			}
		}

		// NON-TRIVIAL GETTERS & SETTERS ..........................................................
		// set rotation in both vector and quaternion form
		void SetRotation(Entity& e, XMFLOAT3 r)
		{
			e.transform.rotation = r;
			e.transform.quaternion = ToQuaternion(e.transform.rotation);
		}

		void SetRotation(Entity& e, float x, float y, float z)
		{
			SetRotation(e, XMFLOAT3(x, y, z));
		}

		// convert from Euler angles to quaternion
		XMVECTOR ToQuaternion(XMFLOAT3 r)
		{
			XMConvertToRadians(r.x);
			XMConvertToRadians(r.y);
			XMConvertToRadians(r.z);

			float cy = cos(r.z * 0.5);
			float sy = sin(r.z * 0.5);
			float cp = cos(r.y * 0.5);
			float sp = sin(r.y * 0.5);
			float cr = cos(r.x * 0.5);
			float sr = sin(r.x * 0.5);

			XMVECTOR q;

			q = XMVectorSet(
				sr * cp * cy - cr * sp * sy,
				cr * sp * cy + sr * cp * sy,
				cr * cp * sy - sr * sp * cy,
				cr * cp * cy + sr * sp * sy
			);

			return q;
		}

		XMVECTOR ToQuaternion(float roll, float pitch, float yaw)
		{
			return ToQuaternion(XMFLOAT3(roll, pitch, yaw));
		}

		// create and return full world space transform matrix
		// from entity transform component
		XMMATRIX GetTransformMatrix(Entity& e)
		{
			e.transform.matrix = MatScale(e);
			e.transform.matrix = XMMatrixMultiply(e.transform.matrix, MatRotation(e));
			e.transform.matrix = XMMatrixMultiply(e.transform.matrix, MatPosition(e));

			return e.transform.matrix;
		}
	}
}


