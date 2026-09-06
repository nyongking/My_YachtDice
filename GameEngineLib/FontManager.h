#pragma once
#include <string>
#include <unordered_map>
#include <memory>

#include "DirectXTK/SpriteFont.h"

namespace GameEngine
{
	class FontManager
	{
	public:
		static FontManager& GetInstance()
		{
			static FontManager instance;
			return instance;
		}

		bool Initialize(ID3D11Device* device);

		// Load a .spritefont file. Returns cached font if already loaded.
		DirectX::SpriteFont* LoadFont(const std::string& key, const std::wstring& path);

		// Get a previously loaded font by key
		DirectX::SpriteFont* GetFont(const std::string& key) const;

		// Get the default font (first loaded, or nullptr)
		DirectX::SpriteFont* GetDefaultFont() const { return m_defaultFont; }

		void Clear();

	private:
		FontManager() = default;
		FontManager(const FontManager&) = delete;
		FontManager& operator=(const FontManager&) = delete;

		ID3D11Device* m_device = nullptr;
		DirectX::SpriteFont* m_defaultFont = nullptr;
		std::unordered_map<std::string, std::unique_ptr<DirectX::SpriteFont>> m_fonts;
	};
}
