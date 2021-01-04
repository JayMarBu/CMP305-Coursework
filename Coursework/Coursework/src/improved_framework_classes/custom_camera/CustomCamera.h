// A replicated and improved version of the frameworks 
// base camera class allowing for greater flexibility in
// child classes.

#pragma once
#include <Input.h>
#include <directxmath.h>
#include "imGUI/imgui.h"
#include "imGUI/imgui_impl_dx11.h"
#include "imGUI/imgui_impl_win32.h"
#include "shaders/shader_utils.h"

using namespace DirectX;

class CustomCamera
{
public:
	CustomCamera();

	// METHODS FROM BASE CAMERA CLASS ...............................................................................................................
	
	void setInput(Input* in) { input = in; }
	void setWindow(int width, int height, HWND window);
	void setFrameTime(float ft) { frameTime = ft; }

	void setPosition(float px, float py, float pz);
	void setRotation(float rx, float ry, float rz);

	XMFLOAT3 getPosition() { return position; }
	XMFLOAT3 getRotation() { return rotation; }
	XMFLOAT3 getRightVector() { return right_vector; }

	XMMATRIX getViewMatrix() { return viewMatrix; }
	XMMATRIX getOrthoViewMatrix() { return orthoMatrix; }

	void update();

	// ADDED FUNCTIONS ..............................................................................................................................
	// methods to create and get cascade regions from camera projection-view frustrum
	void initCascadeMatrices(float fov, float aspect_ratio, float near_plane, float far_plane, float depths[CASCADE_COUNT]);
	XMMATRIX getCascadeMatrix(const unsigned int index);

	// method to draw camera related debug options to gui
	virtual void drawGui();
protected:
	XMFLOAT3 position;		///< float3 for position
	XMFLOAT3 rotation;		///< float3 for rotation (angles)
	XMFLOAT3 right_vector;
	XMMATRIX viewMatrix;	///< matrix for current view
	XMMATRIX orthoMatrix;	///< current orthographic matrix
	float frameTime;		///<  time variables
	Input* input;
	int winWidth, winHeight;///< stores window width and height
	HWND wnd;				///< handle to the window

	// frustrum slices for cascade mapping
	XMMATRIX cascade_matrices_[CASCADE_COUNT];
};

