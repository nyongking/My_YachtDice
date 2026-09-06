#include "GameEnginePch.h"
#include "FontManager.h"

#include "DirectXTK/SpriteFont.h"

namespace GameEngine
{
	bool FontManager::Initialize(ID3D11Device* device)
	{
		if (!device)
			return false;

		m_device = device;
		return true;
	}

	DirectX::SpriteFont* FontManager::LoadFont(const std::string& key, const std::wstring& path)
	{
		auto it = m_fonts.find(key);
		if (it != m_fonts.end())
			return it->second.get();

		if (!m_device)
			return nullptr;

		try
		{
			auto font = std::make_unique<DirectX::SpriteFont>(m_device, path.c_str());
			auto* raw = font.get();
			m_fonts[key] = std::move(font);

			if (!m_defaultFont)
				m_defaultFont = raw;

			return raw;
		}
		catch (...)
		{
			return nullptr;
		}
	}

	DirectX::SpriteFont* FontManager::GetFont(const std::string& key) const
	{
		auto it = m_fonts.find(key);
		if (it != m_fonts.end())
			return it->second.get();
		return nullptr;
	}

	void FontManager::Clear()
	{
		m_fonts.clear();
		m_defaultFont = nullptr;
	}
}
