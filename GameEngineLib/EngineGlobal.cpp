#include "GameEnginePch.h"
#include "EngineGlobal.h"
#include "RenderDevice.h"
#include "RenderPipeline.h"
#include "DeferredLightingMaterial.h"

#include "GeometryManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ComponentRegistry.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "ModelComponent.h"
#include "QuadComponent.h"
#include "PhysicsManager.h"
#include "RigidBodyComponent.h"
#include "CameraController.h"
#include "InputManager.h"
#include "UIMaterial.h"
#include "UIImageComponent.h"
#include "UIComponentRegistry.h"
#include "BillboardComponent.h"
#include "AnimatorComponent.h"
#include "FontManager.h"
#include "UITextComponent.h"
#include "AudioManager.h"
#include "AudioSourceComponent.h"
#include "AudioListenerComponent.h"
#include "SceneManager.h"

#ifdef _DEBUG
#include "WireframeMaterial.h"
#endif

#include <objbase.h>

namespace GameEngine
{
	// 내부 소유
	static std::shared_ptr<GeometryManager> s_geometryMgr;
	static std::shared_ptr<ShaderManager>   s_shaderMgr;
	static std::shared_ptr<MaterialManager> s_materialMgr;
	static std::shared_ptr<TextureManager>  s_textureMgr;
	static std::shared_ptr<ModelManager>    s_modelMgr;
	static std::shared_ptr<PhysicsManager>  s_physicsMgr;
	static std::shared_ptr<AudioManager>    s_audioMgr;

	// 파이프라인 내장 머티리얼 수명 관리
	static std::unique_ptr<Render::DeferredLightingMaterial> s_lightingMat;
	static std::unique_ptr<UIMaterial> s_uiMat;
#ifdef _DEBUG
	static std::unique_ptr<WireframeMaterial> s_wireframeMat;
#endif

	// 외부 접근용 raw pointer
	GeometryManager* GGeometryManager = nullptr;
	ShaderManager*   GShaderManager   = nullptr;
	MaterialManager* GMaterialManager = nullptr;
	TextureManager*  GTextureManager  = nullptr;
	ModelManager*    GModelManager    = nullptr;
	PhysicsManager*  GPhysicsManager  = nullptr;
	AudioManager*    GAudioManager    = nullptr;

#ifdef _DEBUG
	bool GDebugDrawColliders = false;
	Render::Material* GDebugWireframeMaterial = nullptr;
#endif

	bool InitEngine(const std::string& shaderBasePath, HWND hwnd)
	{
		// WICTextureLoader는 COM 초기화 필요
		CoInitializeEx(nullptr, COINIT_MULTITHREADED);

		s_geometryMgr = std::shared_ptr<GeometryManager>(new GeometryManager());
		s_shaderMgr   = std::shared_ptr<ShaderManager>(new ShaderManager());
		s_materialMgr = std::shared_ptr<MaterialManager>(new MaterialManager());
		s_textureMgr  = std::shared_ptr<TextureManager>(new TextureManager());
		s_modelMgr    = std::shared_ptr<ModelManager>(new ModelManager());

		GGeometryManager = s_geometryMgr.get();
		GShaderManager   = s_shaderMgr.get();
		GMaterialManager = s_materialMgr.get();
		GTextureManager  = s_textureMgr.get();
		GModelManager    = s_modelMgr.get();

		auto device  = Render::RenderDevice::GetInstance().GetDevice();
		auto context = Render::RenderDevice::GetInstance().GetContext();

		if (!GGeometryManager->Initialize(device, context))
			return false;

		if (!GShaderManager->Initialize(device, context, shaderBasePath))
			return false;

		if (!GMaterialManager->Initialize(device))
			return false;

		if (!GTextureManager->Initialize(device, context))
			return false;

		if (!GModelManager->Initialize(device))
			return false;

		// Physics
		s_physicsMgr = std::shared_ptr<PhysicsManager>(new PhysicsManager());
		GPhysicsManager = s_physicsMgr.get();
		if (!GPhysicsManager->Initialize())
			return false;

		// Audio
		s_audioMgr = std::shared_ptr<AudioManager>(new AudioManager());
		GAudioManager = s_audioMgr.get();
		if (!GAudioManager->Initialize())
			return false;

		// Input
		if (hwnd)
			InputManager::GetInstance().Initialize(hwnd);

		// 내장 컴포넌트 등록 — 씬 역직렬화 시 타입명으로 생성 가능
		ComponentRegistry::Register<CameraComponent>   ("CameraComponent");
		ComponentRegistry::Register<LightComponent>    ("LightComponent");
		ComponentRegistry::Register<ModelComponent>    ("ModelComponent");
		ComponentRegistry::Register<QuadComponent>     ("QuadComponent");
		ComponentRegistry::Register<RigidBodyComponent>("RigidBodyComponent");
		ComponentRegistry::Register<CameraController>("CameraController");
		ComponentRegistry::Register<BillboardComponent>("BillboardComponent");
		ComponentRegistry::Register<AnimatorComponent>("AnimatorComponent");
		ComponentRegistry::Register<AudioSourceComponent>("AudioSourceComponent");
		ComponentRegistry::Register<AudioListenerComponent>("AudioListenerComponent");
		UIComponentRegistry::Register<UIImageComponent>("UIImageComponent");
		UIComponentRegistry::Register<UITextComponent>("UITextComponent");

		// FontManager
		FontManager::GetInstance().Initialize(device.Get());

		// ── 파이프라인 내장 머티리얼 생성 ──
		auto* lightingSG = GShaderManager->Get("DeferredLighting");
		if (lightingSG)
		{
			s_lightingMat = std::make_unique<Render::DeferredLightingMaterial>();
			if (s_lightingMat->Initialize(lightingSG, device.Get()))
				Render::RenderPipeline::GetInstance().SetLightingMaterial(s_lightingMat.get());
		}

#ifdef _DEBUG
		auto* wireSG = GShaderManager->Get("Wireframe");
		if (wireSG)
		{
			s_wireframeMat = std::make_unique<WireframeMaterial>();
			s_wireframeMat->Initialize(wireSG, device.Get());
			GDebugWireframeMaterial = s_wireframeMat.get();
		}
#endif

		// ── UI 머티리얼 등록 ──
		auto* uiSG = GShaderManager->Get("UI");
		if (uiSG)
		{
			// LoadSync이 Initialize를 호출하므로 미초기화 상태로 전달
			s_uiMat = std::make_unique<UIMaterial>();
			GMaterialManager->LoadSync("UIMaterial", std::make_unique<UIMaterial>(), "UI");
		}

		return true;
	}

	void ReleaseEngine()
	{
		// Scene must be destroyed first — components hold references to managers (Audio, Physics, etc.)
		SceneManager::GetInstance().Clear();

#ifdef _DEBUG
		GDebugWireframeMaterial = nullptr;
		s_wireframeMat.reset();
#endif
		FontManager::GetInstance().Clear();
		s_uiMat.reset();
		s_lightingMat.reset();

		if (GAudioManager) GAudioManager->Shutdown();
		s_audioMgr.reset();
		GAudioManager = nullptr;

		s_physicsMgr.reset();
		s_modelMgr.reset();
		s_textureMgr.reset();
		s_materialMgr.reset();
		s_shaderMgr.reset();
		s_geometryMgr.reset();

		GPhysicsManager  = nullptr;
		GModelManager    = nullptr;
		GTextureManager  = nullptr;
		GMaterialManager = nullptr;
		GShaderManager   = nullptr;
		GGeometryManager = nullptr;

		CoUninitialize();
	}
}
