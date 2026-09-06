#pragma once
#include <cmath>
#include <string>

using MyJson = nlohmann::json;

namespace GameEngine
{
	struct UIAnimation
	{
		enum class Type { None, Pulse };

		Type  type      = Type::None;
		float amplitude = 0.1f;
		float speed     = 3.0f;
		float elapsed   = 0.f;

		float Update(float dt)
		{
			if (type == Type::None)
				return 1.f;

			elapsed += dt;
			return 1.f + sinf(elapsed * speed) * amplitude;
		}

		void Reset()
		{
			elapsed = 0.f;
		}

		MyJson Serialize() const
		{
			MyJson j;
			switch (type)
			{
			case Type::Pulse: j["type"] = "pulse"; break;
			default:          return {};
			}
			j["amplitude"] = amplitude;
			j["speed"]     = speed;
			return j;
		}

		void Deserialize(const MyJson& j)
		{
			std::string t = j.value("type", "none");
			if (t == "pulse") type = Type::Pulse;
			else              type = Type::None;

			amplitude = j.value("amplitude", 0.1f);
			speed     = j.value("speed", 3.0f);
			elapsed   = 0.f;
		}
	};
}
