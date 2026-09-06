#pragma once
#include <array>
#include <vector>

// 서버에서 주사위의 물리 시뮬레이션을 실행을 도와주는 곳
class DiceSimulation
{
public:
	static constexpr int DICE_COUNT = 5;

	struct DiceTransformData
	{
		Vec3 pos;
		Quat rot;
	};

	void Initialize(); // 실제 물리 환경 설정, 
	void StartThrow(uint32 seed, const std::vector<int>& heldIndices = {});

	bool Step(float dt); // Snapshot broadcast 여부 반환
	bool IsSimulating() const { return m_simulating; }
	bool IsSettled() const;

	std::array<DiceTransformData, DICE_COUNT> GetDiceTransforms() const;
	std::array<int, DICE_COUNT> GetTopFaces() const;

	void SetInjection(int index, int face);
	void ClearInjection(int index);  // -1 == index -> 모든 Dice Clear Injection

private:
	int  DetermineTopFace(int index) const;
	void ProcessTriggers();
	void UpdateInjections();

	void SetupBoxCollider(BoxCollider& col, RigidBody* body,
		const Vec3& halfExtents, const Vec3& posOffset, const Vec3& rotOffsetEuler,
		unsigned int layer, unsigned int layerMask, bool isTrigger = false);

	static const Vec3 DIRECTIONS[6];
	static const Vec3 UP_WORLD;

	// Dice physics constants
	static constexpr float FIXED_DT            = 1.f / 60.f;
	static constexpr float BODY_MASS            = 5.f;
	static constexpr float DICE_HALF_EXTENT     = 0.43f;
	static constexpr float DICE_RESTITUTION     = 0.7f;
	static constexpr float DICE_FRICTION        = 0.4f;
	static constexpr float DICE_LINEAR_DAMP     = 0.06f;
	static constexpr float DICE_ANGULAR_DAMP    = 0.08f;
	static constexpr float SETTLE_THRESHOLD     = 0.4f;
	static constexpr int   SNAPSHOT_INTERVAL    = 2; // FIXED_DT * SNAPSHOT_INTERVAL -> 1/30초마다 Broadcast

	


	PhysicsWorld m_world;

	// Dice (5)
	RigidBody    m_diceBodies[DICE_COUNT];
	BoxCollider  m_diceColliders[DICE_COUNT];

	// Desk (Static, 1 collider)
	RigidBody    m_deskBody;
	BoxCollider  m_deskCollider;

	// DiceCup (Kinematic, fixed at Flip position, 10 colliders)
	RigidBody    m_cupBody;
	static constexpr int CUP_COLLIDER_COUNT = 9;
	BoxCollider  m_cupColliders[CUP_COLLIDER_COUNT];

	// Cutter (Static, 7 colliders)
	RigidBody    m_cutterBody;
	static constexpr int CUTTER_COLLIDER_COUNT = 7;
	BoxCollider  m_cutterColliders[CUTTER_COLLIDER_COUNT];

	// Orientation injection (host-only, latch pattern)
	struct InjectionState
	{
		bool active    = false;   // Host의 Injection 요청
		bool injecting = false;   // 현재 Injection 진행중인가?
		int  targetFace = 0;      // 1~6
	};

	static constexpr float INJECT_SPEED_THRESH = 10.f;
	static constexpr float INJECT_KP           = 25.f;
	static constexpr float INJECT_KD           = 5.5f;

	InjectionState m_injection[DICE_COUNT];

	bool  m_simulating = false;
	float m_accumulator = 0.f;
	int   m_stepCounter = 0;

};
