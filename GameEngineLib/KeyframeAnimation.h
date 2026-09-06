#pragma once

// Keyframe: pos/rot at this point, duration = time to reach NEXT keyframe.
// Last keyframe's duration is unused (it's the destination).
struct Keyframe
{
	float3 pos;
	float3 rot;
	float3 scale = { 1.f, 1.f, 1.f };
	float duration;
};

struct KeyframeAnimation
{
	enum LoopState
	{
		NON_LOOP,
		LOOP,
		PINGPONG
	};

	int					  current   = 0;
	float				  elapsed   = 0.f;
	float				  speed     = 1.f;
	bool				  finished  = false;
	int					  direction = 1;       // 1=forward, -1=reverse (PINGPONG)
	LoopState			  loopState = NON_LOOP;
	std::vector<Keyframe> keyframes;

	void Start(std::vector<Keyframe>&& keys, float _speed = 1.f);
	void SetSpeed(float _speed) { speed = _speed; }
	void SetLoopState(LoopState state) { loopState = state; }

	void Reset();
	bool IsPlaying() const { return !finished && keyframes.size() > 1; }

	// returns true when animation is complete (NON_LOOP) or looped
	bool Tick(float dt);

	float3 GetPosition() const;
	float3 GetRotation() const;
	float3 GetScale() const;
};
