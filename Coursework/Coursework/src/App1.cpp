// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include <string>
#include <cmath>

App1::App1()
{

}
// INIT FUNCTIONS ...................................................................................................................................
void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	initSceneInfo(screenWidth, screenHeight);
	initShadows(screenWidth, screenHeight);
	initCam(hwnd, screenWidth, screenHeight);
	initShaders(hwnd);
	initLights();
	initRenderTextures(screenWidth, screenHeight);
	initTextures();
	initWorld();
	initDebug(screenWidth, screenHeight);
}

#pragma region INIT_METHODS

void App1::initSceneInfo(int screenWidth, int screenHeight)
{
	fov_ = (float)XM_PI / 4.0f;
	aspect_ratio_ = (float)screenWidth / (float)screenHeight;
	s_width_ = screenWidth;
	s_height_ = screenHeight;
}

void App1::initShadows(int screenWidth, int screenHeight)
{
	// initialise cascade map
	gpfw::cascade::InitCascadeMap(
		cascade_info_,
		renderer->getDevice(), renderer->getDeviceContext(),
		screenWidth, screenHeight
	);

	cascade_info_.near_clipping_plane = SCREEN_NEAR;
	cascade_info_.far_clipping_plane = SCREEN_DEPTH;

	cascade_info_.cascade_depths[0] = 0.2f;
	cascade_info_.cascade_depths[1] = 0.5f;
	cascade_info_.cascade_depths[2] = 1.0f;
}

void App1::initCam(HWND hwnd, int screenWidth, int screenHeight)
{
	// initialise camera
	cam_.setInput(input);
	cam_.setWindow(screenWidth, screenHeight, hwnd);
	cam_.setMoveSpeed(10.0f);
	cam_.setRotSpeed(50.0f);
	cam_.setPosition(0.0f, 7.0f, -10.0f);

	cam_.initCascadeMatrices(fov_, aspect_ratio_, SCREEN_NEAR, SCREEN_DEPTH, cascade_info_.cascade_depths);
}

void App1::initShaders(HWND hwnd)
{
	// initialise shaders
	light_shader_ = new LightShader(renderer->getDevice(), hwnd);
	depth_shader_ = new DepthShader(renderer->getDevice(), hwnd);
	texture_shader_ = new TextureShader(renderer->getDevice(), hwnd);
	shadow_shader_ = new ShadowShader(renderer->getDevice(), hwnd);
	h_blur_shader_ = new HorizontalBlurShader(renderer->getDevice(), hwnd);
	v_blur_shader_ = new VerticalBlurShader(renderer->getDevice(), hwnd);
	water_shader_ = new WaterShader(renderer->getDevice(), hwnd);
	noise_shader_ = new NoiseShader(renderer->getDevice(), hwnd);

	generate_terrain_cs_ = new GenerateTerrainCShader(renderer->getDevice(), hwnd, pow(TERRAIN_SIZE, 2));
}

void App1::initLights()
{
	// initialise directional light
	gpfw::light::SetLight(lighting_info_, new Light());
	gpfw::light::SetDiffuseColour(lighting_info_, 1.0f, 1.0f, 1.0f, 1.0f);
	gpfw::light::SetDirection(lighting_info_, 0.0f, -0.7f, 0.7f);
	gpfw::light::SetSpecularColour(lighting_info_, 1.0f, 1.0f, 1.0f, 1.0f);
	gpfw::light::SetSpecularPower(lighting_info_, 100.0f);
	gpfw::light::SetAmbientColour(lighting_info_, 0.392f, 0.392f, 0.392f, 1.0f);
}

void App1::initRenderTextures(int screenWidth, int screenHeight)
{
	// initialise texture to render shadows to
	hard_shadow_mask_texture_ = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[0].name = "shadow_texture";
	render_texture_names_[0].render_texture = hard_shadow_mask_texture_;
	render_texture_names_[0].render_type = debug_render_texture_name::RenderType::RenderTarget;

	// initialise blur render targets
	float downsample_width = screenWidth / 2;
	float downsample_height = screenHeight / 2;

	h_blur_texture_ = new RenderTexture(renderer->getDevice(), downsample_width, downsample_height, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[1].name = "h_blur_texture";
	render_texture_names_[1].render_texture = h_blur_texture_;
	render_texture_names_[1].render_type = debug_render_texture_name::RenderType::RenderTarget;

	v_blur_texture_ = new RenderTexture(renderer->getDevice(), downsample_width, downsample_height, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[2].name = "v_blur_texture";
	render_texture_names_[2].render_texture = v_blur_texture_;
	render_texture_names_[2].render_type = debug_render_texture_name::RenderType::RenderTarget;

	downsample_texture_ = new RenderTexture(renderer->getDevice(), downsample_width, downsample_height, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[3].name = "downsample_texture_";
	render_texture_names_[3].render_texture = downsample_texture_;
	render_texture_names_[3].render_type = debug_render_texture_name::RenderType::RenderTarget;

	soft_shadow_mask_texture_ = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[4].name = "upsample_texture_";
	render_texture_names_[4].render_texture = soft_shadow_mask_texture_;
	render_texture_names_[4].render_type = debug_render_texture_name::RenderType::RenderTarget;

	downsample_mesh_ = new OrthoMesh(
		renderer->getDevice(), renderer->getDeviceContext(),
		downsample_width, downsample_height
	);

	upsample_mesh_ = new OrthoMesh(
		renderer->getDevice(), renderer->getDeviceContext(),
		screenWidth, screenHeight
	);

	// initialise ocean render targets
	ocean_.reflection_texture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[5].name = "water_reflection_texture_";
	render_texture_names_[5].render_texture = ocean_.reflection_texture;
	render_texture_names_[5].render_type = debug_render_texture_name::RenderType::RenderTarget;

	ocean_.refraction_texture = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[6].name = "water_refraction_texture_";
	render_texture_names_[6].render_texture = ocean_.refraction_texture;
	render_texture_names_[6].render_type = debug_render_texture_name::RenderType::RenderTarget;

	noise_texture_ = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	render_texture_names_[7].name = "noise texture";
	render_texture_names_[7].render_texture = noise_texture_;
	render_texture_names_[7].render_type = debug_render_texture_name::RenderType::RenderTarget;

	noise_mesh_ = new OrthoMesh(
		renderer->getDevice(), renderer->getDeviceContext(),
		screenWidth, screenWidth
	);

	noise_texture_custom_ = new NoiseTexture(renderer->getDevice(), screenWidth, screenWidth);
	render_texture_names_[8].name = "noise texture custom";
	render_texture_names_[8].render_texture = noise_texture_custom_;
	render_texture_names_[8].render_type = debug_render_texture_name::RenderType::NoiseTextureCustom;
}

void App1::initTextures()
{
	// load textures
	textureMgr->loadTexture(L"brick", L"res/textures/brick1.dds");
	textureMgr->loadTexture(L"grid", L"res/textures/DefaultDiffuse.png");
	textureMgr->loadTexture(L"terrain_height_map", L"res/textures/Output.png");
	textureMgr->loadTexture(L"rocks", L"res/textures/rock_ground_diff_1k.png");
	textureMgr->loadTexture(L"water_DuDv", L"res/textures/Water_001_DuDv.png");
	textureMgr->loadTexture(L"water_normal", L"res/textures/Water_001_NORM.png");
	textureMgr->loadTexture(L"white", L"res/textures/white.png");

	//noise_texture_ = new NoiseTexture(renderer->getDevice(), s_width_, s_height_);
}

void App1::initWorld()
{
	// initialise terrain
	int plane_size = gpfw::terrain::getSideLength();
	gpfw::DeviceInfo d_info;
	d_info.context = renderer->getDeviceContext();
	d_info.device = renderer->getDevice();

	terrain_ = gpfw::terrain::CreateTerrainChunk(d_info, textureMgr, L"rocks", TERRAIN_SIZE, 100);

	// initialise ocean
	ocean_.water_plane = gpfw::entity::CreateEntity(new TerrainMesh(renderer->getDevice(), renderer->getDeviceContext(), 1, 500), "water_plane", textureMgr, nullptr);
	ocean_.setWaterLevel(3);
	ocean_.DuDv_map = L"water_DuDv";
	ocean_.normal_map = L"water_normal";
	ocean_.move_speed.x = 0.04f;
	ocean_.move_speed.y = 0.05f;
	ocean_.tiling = 1.0f;
	ocean_.distortion_strength = 0.07f;
	ocean_.water_colour = XMFLOAT3(0.0, 0.3, 0.5);
	ocean_.reflection_strength = 1.8f;
}

void App1::initDebug(int screenWidth, int screenHeight)
{
	// initialise debug information
	float mesh_width = screenWidth / 4;
	float mesh_height = screenWidth / 4;

	debug_ortho_mesh_ = new OrthoMesh(
		renderer->getDevice(), renderer->getDeviceContext(),
		mesh_width, mesh_height,
		screenWidth / 2 - (mesh_width / 2) - 5 ,
		screenHeight / 2 - (mesh_height / 2) - 5
	);

	debug_transform_ = XMMatrixTranslation((float)(-(int)cascade_info_.scene_width-5), 0, 0);

	if (render_texture_names_[0].render_type == debug_render_texture_name::RenderType::ShadowMap)
		debug_render_ptr_ = ((ShadowMap*)render_texture_names_[0].render_texture)->getDepthMapSRV();
	else
		debug_render_ptr_ = ((RenderTexture*)render_texture_names_[0].render_texture)->getShaderResourceView();
}
#pragma endregion

// DECONSTRUCTOR ....................................................................................................................................
App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// RELEASE RENDER DATA ........................................................................

	// release lights
	gpfw::light::ReleaseLight(lighting_info_);

	// release entities
	for each (gpfw::Entity* e in entities_)
	{
		gpfw::entity::ReleaseEntity(e);
	}
	entities_.clear();

	// release terrain
	gpfw::terrain::ReleaseTerrainChunk(terrain_);

	// release cascade map
	gpfw::cascade::ReleaseCascadeMaps(cascade_info_);

	// release ocean
	ocean_.ReleaseRenderTextures();

	// RELEASE SHADERS ............................................................................
	if (light_shader_)
	{
		delete light_shader_;
		light_shader_ = nullptr;
	}

	if (depth_shader_)
	{
		delete depth_shader_;
		depth_shader_ = nullptr;
	}

	if (texture_shader_)
	{
		delete texture_shader_;
		texture_shader_ = nullptr;
	}

	if (shadow_shader_)
	{
		delete shadow_shader_;
		shadow_shader_ = nullptr;
	}

	if (h_blur_shader_)
	{
		delete h_blur_shader_;
		h_blur_shader_ = nullptr;
	}

	if (v_blur_shader_)
	{
		delete v_blur_shader_;
		v_blur_shader_ = nullptr;
	}

	if (water_shader_)
	{
		delete water_shader_;
		water_shader_ = nullptr;
	}

	if (generate_terrain_cs_)
	{
		delete generate_terrain_cs_;
		generate_terrain_cs_ = nullptr;
	}

	// RELEASE RENDER TEXTURES ....................................................................
	if (hard_shadow_mask_texture_)
	{
		delete hard_shadow_mask_texture_;
		hard_shadow_mask_texture_ = nullptr;
	}

	if (downsample_texture_)
	{
		delete downsample_texture_;
		downsample_texture_ = nullptr;
	}

	if (h_blur_texture_)
	{
		delete h_blur_texture_;
		h_blur_texture_ = nullptr;
	}

	if (v_blur_texture_)
	{
		delete v_blur_texture_;
		v_blur_texture_ = nullptr;
	}

	if (soft_shadow_mask_texture_)
	{
		delete soft_shadow_mask_texture_;
		soft_shadow_mask_texture_ = nullptr;
	}

	// RELEASE ORTHO MESHES .......................................................................
	if (downsample_mesh_)
	{
		delete downsample_mesh_;
		downsample_mesh_ = nullptr;
	}

	if (upsample_mesh_)
	{
		delete upsample_mesh_;
		upsample_mesh_ = nullptr;
	}

	// RELEASE DEBUG DATA .........................................................................
	if (debug_ortho_mesh_)
	{
		delete debug_ortho_mesh_;
		debug_ortho_mesh_ = nullptr;
	}

	if (debug_render_ptr_)
	{
		debug_render_ptr_->Release();
		debug_render_ptr_ = nullptr;
	}
}

// FRAME METHOD .....................................................................................................................................
bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
		return false;

	// update camera
	cam_.handleInput(timer->getTime());

	// update ocean movement
	ocean_.move_factor.x += timer->getTime()*ocean_.move_speed.x;
	ocean_.move_factor.x = (ocean_.move_factor.x >= 1)? ocean_.move_factor.x-1 : ocean_.move_factor.x;

	ocean_.move_factor.y += timer->getTime() * ocean_.move_speed.y;
	ocean_.move_factor.y = (ocean_.move_factor.y >= 1) ? ocean_.move_factor.y - 1 : ocean_.move_factor.y;

	// Render the graphics.
	result = render();
	if (!result)
		return false;

	return true;
}

// RENDER METHODS ...................................................................................................................................
bool App1::render()
{
	// Generate the view matrix based on the camera's position.
	cam_.update();

	CreateNoise();

	depthPass();
	shadowPass();
	waterPass();
	scenePass();

	return true;
}

// DEPTH PASS METHOD ................................................................................................................................
void App1::depthPass()
{
	for (int i = 0; i < CASCADE_COUNT; i++)
	{
		// Set the render target to be the render to texture.
		cascade_info_.cascade_maps[i]->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// calculate centre of cascade frustrum and the radius of the view circle
		XMFLOAT4 frustrum_centre = gpfw::cascade::CalcFrustrumCentre(cam_.getViewMatrix(), cam_.getCascadeMatrix(i), true);

		cascade_info_.s_map_pos[i] = XMFLOAT3(frustrum_centre.x, frustrum_centre.y, frustrum_centre.z);
		
		// set light "position" so the view matrix will align with the current cascade
		lighting_info_.light->setPosition(
			cascade_info_.s_map_pos[i].x - (lighting_info_.light->getDirection().x * frustrum_centre.w),
			cascade_info_.s_map_pos[i].y - (lighting_info_.light->getDirection().y * frustrum_centre.w),
			cascade_info_.s_map_pos[i].z - (lighting_info_.light->getDirection().z * frustrum_centre.w)
		);

		lighting_info_.light->generateOrthoMatrix(frustrum_centre.w * 2, frustrum_centre.w * 2, 0.1, (frustrum_centre.w)*2);
		lighting_info_.light->generateViewMatrix();

		// store matrices in CascadeInfo struct
		cascade_info_.view_matrices[i] = lighting_info_.light->getViewMatrix();
		cascade_info_.proj_matrices[i] = lighting_info_.light->getOrthoMatrix();

		// build shader info struct
		gpfw::ShaderInfo depth_s_info;
		depth_s_info.view = lighting_info_.light->getViewMatrix();
		depth_s_info.projection = lighting_info_.light->getOrthoMatrix();
		depth_s_info.world = renderer->getWorldMatrix();
		depth_s_info.d_info.context = renderer->getDeviceContext();

		// render geometry to depth texture
		depth_shader_->Render(depth_s_info, terrain_);

		for each (gpfw::Entity* e in entities_)
		{
			depth_shader_->Render(depth_s_info, *e);
		}
		
	}

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// SHADOW MASK PASS .................................................................................................................................
void App1::shadowPass()
{
	maskPass();
	downScalePass();
	hBlurPass();
	vBlurPass();
	upSamplePass();
}

#pragma region SHADOW_MASK_PASS

// RENDER SHADOWS TO MASK TEXTURE
void App1::maskPass()
{
	hard_shadow_mask_texture_->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);
	hard_shadow_mask_texture_->setRenderTarget(renderer->getDeviceContext());

	cam_.update();

	gpfw::ShaderInfo s_info;

	s_info.world = renderer->getWorldMatrix();
	s_info.view = cam_.getViewMatrix();
	s_info.projection = renderer->getProjectionMatrix();
	s_info.d_info.context = renderer->getDeviceContext();
	s_info.cam_pos = cam_.getPosition();

	for each (gpfw::Entity * e in entities_)
	{
		shadow_shader_->Render(s_info, lighting_info_, *e, cascade_info_);
	}
	shadow_shader_->Render(s_info, lighting_info_, terrain_, cascade_info_);

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// DOWNSCALE MASK
void App1::downScalePass()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	//float screenSizeX = (float)h_blur_texture_->getTextureWidth();
	downsample_texture_->setRenderTarget(renderer->getDeviceContext());
	downsample_texture_->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = downsample_texture_->getOrthoMatrix();

	// Render for Horizontal Blur
	renderer->setZBuffer(false);
	downsample_mesh_->sendData(renderer->getDeviceContext());
	texture_shader_->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, hard_shadow_mask_texture_->getShaderResourceView());
	texture_shader_->render(renderer->getDeviceContext(), downsample_mesh_->getIndexCount());

	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// HORIZONTAL BLUR
void App1::hBlurPass()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeX = (float)h_blur_texture_->getTextureWidth();
	h_blur_texture_->setRenderTarget(renderer->getDeviceContext());
	h_blur_texture_->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = h_blur_texture_->getOrthoMatrix();

	// Render for Horizontal Blur
	renderer->setZBuffer(false);
	downsample_mesh_->sendData(renderer->getDeviceContext());
	h_blur_shader_->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, downsample_texture_->getShaderResourceView(), screenSizeX);
	h_blur_shader_->render(renderer->getDeviceContext(), downsample_mesh_->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// VERTICAL BLUR
void App1::vBlurPass()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	float screenSizeY = (float)v_blur_texture_->getTextureHeight();
	v_blur_texture_->setRenderTarget(renderer->getDeviceContext());
	v_blur_texture_->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 1.0f, 1.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	// Get the ortho matrix from the render to texture since texture has different dimensions being that it is smaller.
	orthoMatrix = v_blur_texture_->getOrthoMatrix();

	// Render for Vertical Blur
	renderer->setZBuffer(false);
	downsample_mesh_->sendData(renderer->getDeviceContext());
	v_blur_shader_->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, h_blur_texture_->getShaderResourceView(), screenSizeY);
	v_blur_shader_->render(renderer->getDeviceContext(), downsample_mesh_->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// UPSAMPLE BLURRED TEXTURE
void App1::upSamplePass()
{
	XMMATRIX worldMatrix, baseViewMatrix, orthoMatrix;

	//float screenSizeX = (float)h_blur_texture_->getTextureWidth();
	soft_shadow_mask_texture_->setRenderTarget(renderer->getDeviceContext());
	soft_shadow_mask_texture_->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	worldMatrix = renderer->getWorldMatrix();
	baseViewMatrix = camera->getOrthoViewMatrix();
	orthoMatrix = soft_shadow_mask_texture_->getOrthoMatrix();

	// Render for Horizontal Blur
	renderer->setZBuffer(false);
	upsample_mesh_->sendData(renderer->getDeviceContext());
	texture_shader_->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, v_blur_texture_->getShaderResourceView());
	texture_shader_->render(renderer->getDeviceContext(), upsample_mesh_->getIndexCount());

	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}
#pragma endregion

// WATER PASS .......................................................................................................................................
void App1::waterPass()
{
	waterReflectionPass();
	waterRefractionPass();
}

// RENDER WATER REFLECTIONS
void App1::waterReflectionPass()
{
	ocean_.reflection_texture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);
	ocean_.reflection_texture->setRenderTarget(renderer->getDeviceContext());

	float reflection_distance = (cam_.getPosition().y - ocean_.water_level)*2;

	cam_.setPosition(cam_.getPosition().x, cam_.getPosition().y-reflection_distance, cam_.getPosition().z);
	cam_.setRotation(-cam_.getRotation().x, cam_.getRotation().y, cam_.getRotation().z);
	cam_.update();

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = cam_.getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	gpfw::ShaderInfo s_info;

	s_info.world = worldMatrix;
	s_info.view = viewMatrix;
	s_info.projection = projectionMatrix;
	s_info.d_info.context = renderer->getDeviceContext();
	s_info.cam_pos = cam_.getPosition();

	XMFLOAT4 clipping_plane = XMFLOAT4(0, 1, 0, -(ocean_.water_level-ocean_.ref_offset));

	for each (gpfw::Entity * e in entities_)
	{
		light_shader_->Render(s_info, lighting_info_, *e, textureMgr->getTexture(L"white"), clipping_plane);
	}
	light_shader_->Render(s_info, lighting_info_, terrain_, textureMgr->getTexture(L"white"), clipping_plane);

	cam_.setPosition(cam_.getPosition().x, cam_.getPosition().y + reflection_distance, cam_.getPosition().z);
	cam_.setRotation(-cam_.getRotation().x, cam_.getRotation().y, cam_.getRotation().z);
	cam_.update();

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// WATER REFRACTION PASS
void App1::waterRefractionPass()
{
	ocean_.refraction_texture->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);
	ocean_.refraction_texture->setRenderTarget(renderer->getDeviceContext());

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = cam_.getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	gpfw::ShaderInfo s_info;

	s_info.world = worldMatrix;
	s_info.view = viewMatrix;
	s_info.projection = projectionMatrix;
	s_info.d_info.context = renderer->getDeviceContext();
	s_info.cam_pos = cam_.getPosition();

	XMFLOAT4 clipping_plane = XMFLOAT4(0.0f, -1.0f, 0.0f, ocean_.water_level);

	for each (gpfw::Entity * e in entities_)
	{
		light_shader_->Render(s_info, lighting_info_, *e, soft_shadow_mask_texture_->getShaderResourceView(), clipping_plane);
	}
	light_shader_->Render(s_info, lighting_info_, terrain_, soft_shadow_mask_texture_->getShaderResourceView(), clipping_plane);

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// FINAL SCENE PASS ................................................................................................................................. 
void App1::scenePass()
{
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = cam_.getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	gpfw::ShaderInfo s_info;

	s_info.world = worldMatrix;
	s_info.view = viewMatrix;
	s_info.projection = projectionMatrix;
	s_info.d_info.context = renderer->getDeviceContext();
	s_info.cam_pos = cam_.getPosition();

	// RENDER SCENE ...............................................................................
	for each (gpfw::Entity * e in entities_)
	{
		light_shader_->Render(s_info, lighting_info_, *e, soft_shadow_mask_texture_->getShaderResourceView());
	}
	light_shader_->Render(s_info, lighting_info_, terrain_, soft_shadow_mask_texture_->getShaderResourceView());

	water_shader_->Render(s_info, ocean_, lighting_info_);

	// RENDER DEBUG TEXTURES ......................................................................
	if (render_depth_textures_)
		drawOrthoMeshes(s_info, debug_render_ptr_);


	if (render_cascade_textures_)
		gpfw::cascade::RenderOrthoMeshes(cascade_info_, texture_shader_, s_info, cam_.getOrthoViewMatrix(), renderer->getOrthoMatrix());

	gui();
	renderer->endScene();
}

void App1::CreateNoise()
{
	noise_texture_->clearRenderTarget(renderer->getDeviceContext(), 1.f, 0.f, 1.f, 1.0f);
	noise_texture_->setRenderTarget(renderer->getDeviceContext());

	gpfw::ShaderInfo s_info;
	s_info.world = renderer->getWorldMatrix();
	s_info.view = cam_.getOrthoViewMatrix();
	s_info.projection = noise_texture_->getProjectionMatrix();
	s_info.d_info.context = renderer->getDeviceContext();

	noise_shader_->Render(s_info, noise_mesh_, debug_noise_scale_, XMFLOAT2(noise_offest_[0], noise_offest_[1]));

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

// draw debug ortho mesh
void App1::drawOrthoMeshes(gpfw::ShaderInfo s_info, ID3D11ShaderResourceView* texture)
{
	XMMATRIX orthoViewMatrix = cam_.getOrthoViewMatrix();
	XMMATRIX orthoProjMatrix = renderer->getOrthoMatrix();

	if (render_cascade_textures_)
		s_info.world = debug_transform_;
	debug_ortho_mesh_->sendData(s_info.d_info.context);
	texture_shader_->setShaderParameters(s_info.d_info.context, s_info.world, orthoViewMatrix, orthoProjMatrix, texture);
	texture_shader_->render(s_info.d_info.context, debug_ortho_mesh_->getIndexCount());
}

// IMGUI METHODS ....................................................................................................................................
void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());

	// draw camera UI elements
	cam_.drawGui();

	// TERRAIN OPTIONS ............................................................................
	if (ImGui::CollapsingHeader("Terrain Options"))
	{
		ImGui::Text("Colour:");
		ImGui::ColorEdit3("##noise colour", debug_colours_);

		if (ImGui::Button("Colour texture"))
		{
			noise_texture_custom_->setColour(renderer->getDeviceContext(), debug_colours_[0], debug_colours_[1], debug_colours_[2], 1.0);
			//noise_texture_custom_->setNoise(renderer->getDeviceContext(), 0.3f);
		}
		ImGui::Text("Noise scale:");
		ImGui::SliderFloat("##noise scale", &debug_noise_scale_, 0.001f, 1.0f);
		ImGui::Text("Noise offset:");
		ImGui::SliderFloat2("##noise offset", noise_offest_,  -10.0f, 10.0f);
		if (ImGui::Button("Noise texture"))
		{
			//noise_texture_custom_->setColour(renderer->getDeviceContext(), debug_colours_[0], debug_colours_[1], debug_colours_[2], 1.0);
			noise_texture_custom_->setNoise(renderer->getDeviceContext(), debug_noise_scale_, XMFLOAT2(noise_offest_[0],noise_offest_[1]));
		}

		if (ImGui::Button("Generate Terrain Mesh"))
		{
			gpfw::DeviceInfo d_info;
			d_info.context = renderer->getDeviceContext();
			d_info.device = renderer->getDevice();
			gpfw::terrain::Generate(terrain_, d_info, generate_terrain_cs_, textureMgr->getTexture(L"terrain_height_map"));
		}
	}

	// OCEAN CONTROLS .............................................................................
	if (ImGui::CollapsingHeader("Ocean Options"))
	{
		ImGui::Text("Water Level:");
		if (ImGui::SliderFloat("##water level", &ocean_.water_level, 0.1f, 10))
		{
			ocean_.setWaterLevel(ocean_.water_level);
		}

		ImGui::Text("Distortion tiling:");
		ImGui::SliderInt("##water tiling", &ocean_.tiling, 0.1f, 100);

		ImGui::Text("Distortion strength:");
		ImGui::SliderFloat("##water distortion strength", &ocean_.distortion_strength, 0.01f, 1.f);

		ImGui::Text("Offset Speed:");
		float movespeeds[2] = { ocean_.move_speed.x, ocean_.move_speed.y };
		if (ImGui::SliderFloat2("##water move speed", movespeeds, 0.01f, 0.5f))
			ocean_.move_speed = XMFLOAT2(movespeeds[0], movespeeds[1]);
		ImGui::Text(("water move offset: (" + std::to_string(ocean_.move_factor.x) + ", " + std::to_string(ocean_.move_factor.y)+")").c_str());

		ImGui::Text("Reflection strength:");
		ImGui::SliderFloat("##water reflection strength", &ocean_.reflection_strength, 0.0f, 10.0f);

		ImGui::Text("Colour:");
		float colour_floats[3] = { ocean_.water_colour.x,ocean_.water_colour.y, ocean_.water_colour.z };
		if (ImGui::ColorEdit3("##water colour", colour_floats))
			ocean_.water_colour = XMFLOAT3(colour_floats[0], colour_floats[1], colour_floats[2]);

		ImGui::Text("Colour intensity:");
		ImGui::SliderFloat("##water colour intensity", &ocean_.colour_intensity, 0.0f, 1.0f);

		ImGui::Text("Water reflectivity:");
		ImGui::SliderFloat("##water reflectivity", &ocean_.reflectivity, 0.0f, 1.0f);
	}


	// DEBUG CONTROLS .............................................................................
	if (ImGui::CollapsingHeader("Debug Options"))
	{
		ImGui::Checkbox("Wireframe mode", &wireframeToggle);
		ImGui::Checkbox("Render Cascade Regions", &cascade_info_.show_cascades);
		ImGui::Checkbox("Draw cascade textures", &render_cascade_textures_);
		ImGui::Checkbox("Draw render textures", &render_depth_textures_);

		if (render_depth_textures_)
		{
			static const char* current_item = render_texture_names_[0].name;

			if (ImGui::BeginCombo("debug texture##combo", current_item))
			{
				for (int n = 0; n < IM_ARRAYSIZE(render_texture_names_); n++)
				{
					bool is_selected = (current_item == render_texture_names_[n].name);
					if (ImGui::Selectable(render_texture_names_[n].name, is_selected))
					{
						current_item = render_texture_names_[n].name;
						if(render_texture_names_[n].render_type == debug_render_texture_name::RenderType::ShadowMap)
							debug_render_ptr_ = ((ShadowMap*)render_texture_names_[n].render_texture)->getDepthMapSRV();
						else if (render_texture_names_[n].render_type == debug_render_texture_name::RenderType::RenderTarget)
							debug_render_ptr_ = ((RenderTexture*)render_texture_names_[n].render_texture)->getShaderResourceView();
						else if (render_texture_names_[n].render_type == debug_render_texture_name::RenderType::NoiseTextureCustom)
							debug_render_ptr_ = ((NoiseTexture*)render_texture_names_[n].render_texture)->getShaderResourceView();
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::drawLightGui()
{
	if (ImGui::CollapsingHeader("Light Options"))
	{
		ImGui::Indent(10);

		ImGui::Text("Position");
		float l_pos[3] = { lighting_info_.light->getPosition().x,lighting_info_.light->getPosition().y,lighting_info_.light->getPosition().z };
		ImGui::DragFloat3("##light position",l_pos);

		// light diffuse colour
		ImGui::Text("Diffuse Colour");
		float light_colour[3] = { lighting_info_.diffuse.x, lighting_info_.diffuse.y, lighting_info_.diffuse.z };
		if (ImGui::ColorEdit3("##light diffuse colour", light_colour))
			gpfw::light::SetDiffuseColour(lighting_info_, light_colour[0], light_colour[1], light_colour[2], 1.0f);

		// light directional gizmo
		drawLightGuiGizmo();

		// light specular
		ImGui::Text("Specular Colour");
		light_colour[0] = lighting_info_.spec_colour.x;
		light_colour[1] = lighting_info_.spec_colour.y;
		light_colour[2] = lighting_info_.spec_colour.z;
		if(ImGui::ColorEdit3("##light specular colour", light_colour))
			gpfw::light::SetSpecularColour(lighting_info_, light_colour[0], light_colour[1], light_colour[2], 1.0f);

		ImGui::Text("Specular Power");
		ImGui::SliderFloat("##light specular power", &lighting_info_.dir_spec_pow.w, 0.0f, 1000.0f);
		gpfw::light::SetSpecularPower(lighting_info_, lighting_info_.dir_spec_pow.w);

		ImGui::Indent(-10);
	}
}

void App1::drawLightGuiGizmo()
{
	ImGui::Text("Direction");
	
	XMFLOAT3 cam_angles = XMFLOAT3(cam_.getRotation().x * 0.0174532f, cam_.getRotation().y * 0.0174532f, cam_.getRotation().z * 0.0174532f);
	XMMATRIX gizmo_rotation_matrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&cam_angles));
	XMFLOAT3 light_dir_dx = gpfw::light::GetDirection(lighting_info_);
	light_dir_dx.z = -light_dir_dx.z;
	XMVECTOR light_dir_vecdx = XMLoadFloat3(&light_dir_dx);

	light_dir_vecdx = XMVector3Transform(light_dir_vecdx, gizmo_rotation_matrix);

	XMStoreFloat3(&light_dir_dx, light_dir_vecdx);

	vgm::Vec3 light_dir(light_dir_dx.x, light_dir_dx.y, light_dir_dx.z);

	if (ImGui::gizmo3D("##Light Direction Gizmo", light_dir, 100, imguiGizmo::modeDirection))
	{
		gizmo_rotation_matrix = XMMatrixInverse(nullptr, gizmo_rotation_matrix);
		light_dir_dx = XMFLOAT3(light_dir.x, light_dir.y, light_dir.z);
		light_dir_vecdx = XMLoadFloat3(&light_dir_dx);

		light_dir_vecdx = XMVector3Transform(light_dir_vecdx, gizmo_rotation_matrix);
		XMStoreFloat3(&light_dir_dx, light_dir_vecdx);

		if (light_dir_dx.y > -0.01f)
			light_dir_dx.y = -0.01f;

		gpfw::light::SetDirection(lighting_info_, light_dir_dx.x, light_dir_dx.y, -light_dir_dx.z);
	}

	float light_dir_test[3] = { gpfw::light::GetDirection(lighting_info_).x, gpfw::light::GetDirection(lighting_info_).y , gpfw::light::GetDirection(lighting_info_).z };

	ImGui::SameLine(145);
	ImGui::Text(("x: " + std::to_string(light_dir_test[0])).c_str());
	ImGui::SameLine(145);
	ImGui::Text(("\n\n\ny: " + std::to_string(light_dir_test[1])).c_str());
	ImGui::SameLine(145);
	ImGui::Text(("\n\n\n\n\n\nz: " + std::to_string(light_dir_test[2])).c_str());
}

gpfw::Entity* App1::getEntity(const char* search_name)
{
	for (int i = 0; i < entities_.size(); i++)
	{
		if (entities_[i]->name = search_name)
		{
			return entities_[i];
		}
	}
}

