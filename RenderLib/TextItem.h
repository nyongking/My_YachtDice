#pragma once
#include <string>

namespace Render
{
	enum class TextAlignment
	{
		Left,
		Center,
		Right
	};

	// UIPass text rendering command (processed by SpriteBatch/SpriteFont)
	struct TextCommand
	{
		std::wstring text;
		float2       position = {};  // screen-space pixel position (top-left origin)
		float4       color    = { 1.f, 1.f, 1.f, 1.f };
		float        scale    = 1.f;
		float        rotation = 0.f;
		float        depth    = 0.f;
		TextAlignment alignment = TextAlignment::Left;
		void*        font     = nullptr;  // DirectX::SpriteFont*
	};
}
