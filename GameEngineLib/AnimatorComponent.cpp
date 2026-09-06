#include "GameEnginePch.h"
#include "AnimatorComponent.h"
#include "Transform.h"
#include "GameObject.h"

namespace GameEngine
{
	void AnimatorComponent::Start()
	{
		m_transform = GetOwner()->GetComponent<Transform>();

		if (!m_startAnimName.empty())
			Play(m_startAnimName);
	}

	void AnimatorComponent::Play(const std::string& name)
	{
		auto it = m_animations.find(name);
		if (it == m_animations.end())
			return;

		m_currentName = name;
		m_current = &it->second;
		m_current->Reset();
		m_blending = false;

		if (m_transform && !m_current->keyframes.empty())
		{
			m_transform->SetPosition(m_current->keyframes[0].pos);
			m_transform->SetRotation(m_current->keyframes[0].rot);
			m_transform->SetScale(m_current->keyframes[0].scale);
		}
	}

	void AnimatorComponent::Transition(const std::string& name, float blendDuration)
	{
		auto it = m_animations.find(name);
		if (it == m_animations.end())
			return;

		if (m_transform)
		{
			m_blendStartPos   = m_transform->GetPosition();
			m_blendStartRot   = m_transform->GetRotation();
			m_blendStartScale = m_transform->GetScale();
		}

		m_currentName = name;
		m_current = &it->second;
		m_current->Reset();

		m_blending      = true;
		m_blendElapsed  = 0.f;
		m_blendDuration = blendDuration;
	}

	void AnimatorComponent::Stop()
	{
		if (m_current)
		{
			m_current->finished = true;
			m_current = nullptr;
			m_currentName.clear();
			m_blending = false;
		}
	}

	bool AnimatorComponent::IsFinished() const
	{
		return !m_current || m_current->finished;
	}

	bool AnimatorComponent::IsPlaying() const
	{
		return m_current && !m_current->finished;
	}

	void AnimatorComponent::Update(float dt)
	{
		if (!m_current || m_current->finished || !m_transform)
			return;

		m_current->Tick(dt);

		float3 animPos   = m_current->GetPosition();
		float3 animRot   = m_current->GetRotation();
		float3 animScale = m_current->GetScale();

		if (m_blending)
		{
			m_blendElapsed += dt;
			float t = m_blendElapsed / m_blendDuration;

			if (t >= 1.f)
			{
				m_blending = false;
			}
			else
			{
				animPos   = Lerp(m_blendStartPos, animPos, t);
				animRot   = Lerp(m_blendStartRot, animRot, t);
				animScale = Lerp(m_blendStartScale, animScale, t);
			}
		}

		m_transform->SetPosition(animPos);
		m_transform->SetRotation(animRot);
		m_transform->SetScale(animScale);
	}

	// --- Serialization ---

	MyJson AnimatorComponent::Serialize() const
	{
		MyJson j;
		j["type"] = GetTypeName();

		MyJson animArray = MyJson::array();

		for (auto& [name, anim] : m_animations)
		{
			MyJson animJson;
			animJson["name"] = name;

			if (anim.loopState == KeyframeAnimation::LOOP)
				animJson["loop"] = "loop";
			else if (anim.loopState == KeyframeAnimation::PINGPONG)
				animJson["loop"] = "pingpong";
			else
				animJson["loop"] = "none";

			MyJson kfArray = MyJson::array();
			for (auto& kf : anim.keyframes)
			{
				MyJson kfJson;
				kfJson["pos"]      = { kf.pos.x, kf.pos.y, kf.pos.z };
				kfJson["rot"]      = { kf.rot.x, kf.rot.y, kf.rot.z };
				kfJson["scale"]    = { kf.scale.x, kf.scale.y, kf.scale.z };
				kfJson["duration"] = kf.duration;
				kfArray.push_back(kfJson);
			}
			animJson["keyframes"] = kfArray;

			animArray.push_back(animJson);
		}

		j["animations"] = animArray;
		return j;
	}

	void AnimatorComponent::Deserialize(const MyJson& j)
	{
		if (!j.contains("animations"))
			return;

		for (auto& animJson : j["animations"])
		{
			KeyframeAnimation anim;

			std::string name = animJson.value("name", "");

			if (animJson.contains("loop"))
			{
				std::string loop = animJson["loop"].get<std::string>();
				if      (loop == "loop")     anim.loopState = KeyframeAnimation::LOOP;
				else if (loop == "pingpong") anim.loopState = KeyframeAnimation::PINGPONG;
			}

			if (animJson.contains("keyframes"))
			{
				for (auto& kfJson : animJson["keyframes"])
				{
					Keyframe kf;
					kf.pos      = { kfJson["pos"][0], kfJson["pos"][1], kfJson["pos"][2] };
					kf.rot      = { kfJson["rot"][0], kfJson["rot"][1], kfJson["rot"][2] };
					if (kfJson.contains("scale"))
						kf.scale = { kfJson["scale"][0], kfJson["scale"][1], kfJson["scale"][2] };
					kf.duration = kfJson.value("duration", 0.f);
					anim.keyframes.push_back(kf);
				}
			}

			m_animations[name] = std::move(anim);
		}

		if (j.contains("Start"))
			m_startAnimName = j["Start"].get<std::string>();
	}
}
