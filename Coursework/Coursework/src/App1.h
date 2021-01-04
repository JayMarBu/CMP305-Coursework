// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
#include "improved_framework_classes/custom_camera/FreeCamera.h"
#include "shaders/light_shader/light_shader.h"
#include "shaders/depth_shader/depth_shader.h"
#include "shaders/texture_shader/texture_shader.h"
#include "shaders/shadow_shader/shadow_shader.h"
#include "shaders/blur_shaders/horizontal_blur/horizontal_blur_shader.h"
#include "shaders/blur_shaders/vertical_blur/vertical_blur_shader.h"
#include "shaders/water_shader/water_shader.h"
#include "shaders/motion_blur_shader/motion_blur_shader.h"
#include "shaders/compute_shaders/generate_terrain_cshader.h"
#include "imGuIZMO.quat/imGuIZMOquat.h"
#include "ImGuizmo-master/ImGuizmo.h"
#include "improved_framework_classes/TerrainMesh.h"
#include "entities/entity.h"
#include "entities/terrain.h"



class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	// INITIALISATION METHODS .......................................................................................................................
	void initSceneInfo(int screenWidth, int screenHeight);
	void initShadows(int screenWidth, int screenHeight);
	void initCam(HWND hwnd, int screenWidth, int screenHeight);
	void initShaders(HWND hwnd);
	void initLights();
	void initRenderTextures(int screenWidth, int screenHeight);
	void initTextures();
	void initEntities();
	void initDebug(int screenWidth, int screenHeight);

	// RENDER METHODS ...............................................................................................................................
	bool render();

	void depthPass();
	void shadowPass();
	void waterPass();
	void scenePass();

	void maskPass();
	void downScalePass();
	void hBlurPass();
	void vBlurPass();
	void upSamplePass();

	void waterReflectionPass();
	void waterRefractionPass();

	void drawOrthoMeshes(gpfw::ShaderInfo s_info, ID3D11ShaderResourceView* texture);

	void postProcessPass();

	// GUI METHODS ..................................................................................................................................
	void gui();

	void drawLightGui();
	void drawLightGuiGizmo();

	// MISC METHODS .................................................................................................................................
	gpfw::Entity* getEntity(const char* name);

private:
	// CAMERA .......................................................................................................................................
	FreeCamera cam_;

	// SHADERS ......................................................................................................................................
	LightShader* light_shader_;
	DepthShader* depth_shader_;
	TextureShader* texture_shader_;
	ShadowShader* shadow_shader_;
	HorizontalBlurShader* h_blur_shader_;
	VerticalBlurShader* v_blur_shader_;
	WaterShader* water_shader_;
	MotionBlurShader* motion_blur_shader_;

	GenerateTerrainCShader* generate_terrain_cs_;

	// RENDER TEXTURES ..............................................................................................................................
	RenderTexture* hard_shadow_mask_texture_;
	RenderTexture* downsample_texture_;
	RenderTexture* h_blur_texture_;
	RenderTexture* v_blur_texture_;
	RenderTexture* soft_shadow_mask_texture_;
	RenderTexture* post_process_texture_;

	// RENDER DATA ..................................................................................................................................
	gpfw::LightingInfo lighting_info_;

	std::vector<gpfw::Entity*> entities_;

	gpfw::TerrainChunk terrain_;

	gpfw::CascadeInfo cascade_info_;

	gpfw::WaterInfo ocean_;

	OrthoMesh* downsample_mesh_;
	OrthoMesh* upsample_mesh_;

	float fov_;
	float aspect_ratio_;
	float s_width_;
	float s_height_;

	XMFLOAT3 prev_cam_pos_;
	XMFLOAT3 prev_cam_rot_;
	int blur_distance_ = 1;

	// DEBUG DATA ...................................................................................................................................
	ID3D11ShaderResourceView* debug_render_ptr_;
	OrthoMesh* debug_ortho_mesh_;
	XMMATRIX debug_transform_;

	bool render_depth_textures_ = false;
	bool render_cascade_textures_ = false;
	
	struct debug_render_texture_name
	{
		enum RenderType
		{
			RenderTarget,
			ShadowMap
		};

		const char* name;
		void* render_texture = nullptr;
		
		RenderType render_type;
	};

	debug_render_texture_name render_texture_names_[7];
};

#endif