#pragma once
#include <string>

namespace Render { class Material; }

namespace GameEngine
{
	class GeometryManager;
	class ShaderManager;
	class MaterialManager;
	class TextureManager;
	class ModelManager;
	class PhysicsManager;
	class AudioManager;
	class InputManager;
	class FontManager;

	extern GeometryManager* GGeometryManager;
	extern ShaderManager*   GShaderManager;
	extern MaterialManager* GMaterialManager;
	extern TextureManager*  GTextureManager;
	extern ModelManager*    GModelManager;
	extern PhysicsManager*  GPhysicsManager;
	extern AudioManager*    GAudioManager;

#ifdef _DEBUG
	extern bool GDebugDrawColliders;
	extern Render::Material* GDebugWireframeMaterial;
#endif

	bool InitEngine(const std::string& shaderBasePath, HWND hwnd = nullptr);
	void ReleaseEngine();
}
