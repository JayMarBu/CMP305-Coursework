#include "CustomCamera.h"

CustomCamera::CustomCamera()
{
	position = XMFLOAT3(0.f, 0.f, 0.f);
	rotation = XMFLOAT3(0.f, 0.f, 0.f);

	//lookSpeed = 4.0f;

	// Generate ortho matrix
	XMVECTOR up, position, lookAt;
	up = XMVectorSet(0.0f, 1.0, 0.0, 1.0f);
	position = XMVectorSet(0.0f, 0.0, -10.0, 1.0f);
	lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0);
	orthoMatrix = XMMatrixLookAtLH(position, lookAt, up);
}

void CustomCamera::initCascadeMatrices(float fov, float aspect_ratio, float near_plane, float far_plane, float depths[CASCADE_COUNT])
{
	float current_near = near_plane;

	// build view frustrum for each cascade region
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		cascade_matrices_[i] = XMMatrixPerspectiveFovLH(fov, aspect_ratio, current_near, far_plane * depths[i]);
		current_near = far_plane * depths[i];
	}
}

void CustomCamera::setWindow(int width, int height, HWND window)
{
	winWidth = width; winHeight = height; wnd = window;
}

void CustomCamera::setPosition(float px, float py, float pz)
{
	position.x = px;
	position.y = py;
	position.z = pz;
}

void CustomCamera::setRotation(float rx, float ry, float rz)
{
	rotation.x = rx;
	rotation.y = ry;
	rotation.z = rz;
}

DirectX::XMMATRIX CustomCamera::getCascadeMatrix(const unsigned int index)
{
	return cascade_matrices_[index];
}

void CustomCamera::drawGui()
{
}

void CustomCamera::update()
{
	XMVECTOR up, positionv, lookAt, right;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// Setup the vectors
	up = XMVectorSet(0.0f, 1.0, 0.0, 1.0f);
	positionv = XMLoadFloat3(&position);
	lookAt = XMVectorSet(0.0, 0.0, 1.0f, 1.0f);
	right = XMVectorSet(1.0, 0.0, 0.0, 1.0);

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = rotation.x * 0.0174532f;
	yaw = rotation.y * 0.0174532f;
	roll = rotation.z * 0.0174532f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
	up = XMVector3TransformCoord(up, rotationMatrix);
	right = XMVector3TransformCoord(right, rotationMatrix);
	XMStoreFloat3(&right_vector, right);

	// Translate the rotated camera position to the location of the viewer.
	lookAt = positionv + lookAt;

	// Finally create the view matrix from the three updated vectors.
	viewMatrix = XMMatrixLookAtLH(positionv, lookAt, up);
}
