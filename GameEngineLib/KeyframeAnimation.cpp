#include "GameEnginePch.h"
#include "KeyframeAnimation.h"

void KeyframeAnimation::Start(std::vector<Keyframe>&& keys, float _speed)
{
	keyframes = std::move(keys);
	speed = _speed;
	Reset();
}

void KeyframeAnimation::Reset()
{
	current   = 0;
	elapsed   = 0.f;
	finished  = false;
	direction = 1;
}

bool KeyframeAnimation::Tick(float dt)
{
	if (finished || keyframes.size() < 2)
		return true;

	int lastIndex = static_cast<int>(keyframes.size()) - 1;

	elapsed += dt * speed * direction;

	if (1 == direction)
	{
		while (elapsed >= keyframes[current].duration && keyframes[current].duration > 0.f)
		{
			elapsed -= keyframes[current].duration * direction;

			int next = current + direction;

			if (next >= lastIndex)
			{
				switch (loopState)
				{
				case NON_LOOP:
					current = (direction > 0) ? lastIndex : 0;
					elapsed = 0.f;
					finished = true;
					return true;

				case LOOP:
					current = 0;
					//elapsed = 0.f;
					break;

				case PINGPONG:
					direction = -direction;
					elapsed = keyframes[current].duration - elapsed;
					break;
				}
			}
			else
			{
				current = next;
			}
		}
	}
	else if (-1 == direction)
	{
		while (elapsed < 0.f)
		{
			int prev = current - 1;

			if (prev < 0)
			{
				switch (loopState)
				{
				case NON_LOOP:
					current = 0;
					elapsed = 0.f;
					finished = true;
					return true;

				case LOOP:
					current = lastIndex - 1;
					elapsed += keyframes[current].duration;
					break;

				case PINGPONG:
					direction = -direction;
					elapsed = -elapsed;
					break;
				}
			}
			else
			{
				current = prev;
				elapsed += keyframes[current].duration;
			}
		}
	}

	

	return false;
}

float3 KeyframeAnimation::GetPosition() const
{
	if (keyframes.empty())
		return {};

	if (finished || keyframes.size() < 2)
		return keyframes.back().pos;

	int nextIdx = current + 1;
	if (nextIdx >= static_cast<int>(keyframes.size()))
		return keyframes[current].pos;

	float t = elapsed / keyframes[current].duration;
	return Lerp(keyframes[current].pos, keyframes[nextIdx].pos, t);
}

float3 KeyframeAnimation::GetRotation() const
{
	if (keyframes.empty())
		return {};

	if (finished || keyframes.size() < 2)
		return keyframes.back().rot;

	int nextIdx = current + 1;
	if (nextIdx >= static_cast<int>(keyframes.size()))
		return keyframes[current].rot;

	float t = elapsed / keyframes[current].duration;
	return Lerp(keyframes[current].rot, keyframes[nextIdx].rot, t);
}

float3 KeyframeAnimation::GetScale() const
{
	if (keyframes.empty())
		return { 1.f, 1.f, 1.f };

	if (finished || keyframes.size() < 2)
		return keyframes.back().scale;

	int nextIdx = current + 1;
	if (nextIdx >= static_cast<int>(keyframes.size()))
		return keyframes[current].scale;

	float t = elapsed / keyframes[current].duration;
	return Lerp(keyframes[current].scale, keyframes[nextIdx].scale, t);
}
