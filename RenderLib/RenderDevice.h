#pragma once

namespace Render
{
	class RenderDevice
	{
#pragma region Singleton
	public:
		static RenderDevice& GetInstance()
		{
			static RenderDevice instance;

			return instance;
		}
#pragma endregion Singleton

		friend class Renderer;

	public:
		RenderDevice();
		~RenderDevice();

	public:
		bool Initialize();

		RefCom<ID3D11Device>        GetDevice()  { return m_device; }
		RefCom<ID3D11DeviceContext> GetContext() { return m_context; }

	private:
		RefCom<ID3D11Device>		m_device;
		RefCom<ID3D11DeviceContext>	m_context;
	};


}

