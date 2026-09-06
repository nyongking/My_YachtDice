#include "ServerPch.h"
#include "DiceSimulation.h"
#include <random>

const Vec3 DiceSimulation::UP_WORLD = { 0.f, 1.f, 0.f };

const Vec3 DiceSimulation::DIRECTIONS[6] =
{
	{0.f, 0.f, 0.f},     // face 1
	{90.f, 0.f, 0.f},    // face 2
	{0.f, 0.f, -90.f},   // face 3
	{0.f, 0.f, 90.f},    // face 4
	{-90.f, 0.f, 0.f},   // face 5
	{180.f, 0.f, 0.f}    // face 6
};

static unsigned int MakeMask(std::initializer_list<int> layers)
{
	unsigned int mask = 0;
	for (int l : layers) mask |= (1u << l);
	return mask;
}

void DiceSimulation::SetupBoxCollider(BoxCollider& col, RigidBody* body,
	const Vec3& halfExtents, const Vec3& posOffset, const Vec3& rotOffsetEuler,
	unsigned int layer, unsigned int layerMask, bool isTrigger)
{
	col.halfExtents = halfExtents;
	col.posOffset = posOffset;
	col.rotOffset = QuatFromEulerXYZ(rotOffsetEuler.x, rotOffsetEuler.y, rotOffsetEuler.z);
	col.body = body;
	col.layer = layer;
	col.layerMask = layerMask;
	col.isTrigger = isTrigger;
	col.enabled = true;
	m_world.AddCollider(&col);
}

void DiceSimulation::Initialize()
{
	m_world.SetGravity(Vec3(0.f, -20.f, 0.f));

	// Desk
	m_deskBody.SetBodyType(RigidBody::BodyType::Static);
	m_deskBody.SetRestitution(0.1f);
	m_deskBody.SetFriction(1.0f);
	m_deskBody.SetPosition(Vec3(0.f, -30.f, 20.f));
	m_deskBody.SetOrientation(QuatFromEulerXYZ(90.f, 90.f, 0.f));

	m_world.AddRigidBody(&m_deskBody);

	SetupBoxCollider(m_deskCollider, &m_deskBody,
		Vec3(20.f, 30.f, 3.f), Vec3(10.8f, 0.f, -23.f), Vec3(0.f, 0.f, 0.f),
		2, MakeMask({1}));
		
	// Cup
	m_cupBody.SetBodyType(RigidBody::BodyType::Kinematic);
	m_cupBody.SetRestitution(0.3f);
	m_cupBody.SetFriction(0.8f);
	m_cupBody.SetPosition(Vec3(4.0f, 3.5f, 8.5f));
	m_cupBody.SetOrientation(QuatFromEulerXYZ(90.f, 0.f, 125.f));
	m_world.AddRigidBody(&m_cupBody);

	struct CupColData { Vec3 half; Vec3 off; Vec3 rot; };
	const CupColData cupData[CUP_COLLIDER_COUNT] = {
		{ {1.5f, 0.4f, 1.9f}, { 0.0f,  2.2f, -0.1f}, {0,0,0} },
		{ {1.5f, 0.4f, 1.9f}, {-1.48f, 1.48f,-0.1f}, {0,0,45} },
		{ {1.5f, 0.4f, 1.9f}, {-2.2f,  0.0f, -0.1f}, {0,0,90} },
		{ {1.5f, 0.4f, 1.9f}, {-1.48f,-1.48f,-0.1f}, {0,0,135} },
		{ {1.5f, 0.4f, 1.9f}, { 0.0f, -2.2f, -0.1f}, {0,0,0} },
		{ {1.5f, 0.4f, 1.9f}, { 1.48f,-1.48f,-0.1f}, {0,0,45} },
		{ {1.5f, 0.4f, 1.9f}, { 2.2f,  0.0f, -0.1f}, {0,0,90} },
		{ {1.5f, 0.4f, 1.9f}, { 1.48f, 1.48f,-0.1f}, {0,0,135} },
		{ {2.0f, 2.0f, 0.5f}, {0.f, 0.f, 2.0f},  {0,0,0} },
	};

	for (int i = 0; i < CUP_COLLIDER_COUNT; ++i)
	{
		SetupBoxCollider(m_cupColliders[i], &m_cupBody,
			cupData[i].half, cupData[i].off, cupData[i].rot,
			2, MakeMask({1}));
	}


	// cutter
	m_cutterBody.SetBodyType(RigidBody::BodyType::Static);
	m_cutterBody.SetRestitution(0.3f);
	m_cutterBody.SetFriction(1.5f);
	m_cutterBody.SetPosition(Vec3(1.5f, 0.f, 8.5f));
	m_cutterBody.SetOrientation(QuatFromEulerXYZ(90.f, 0.f, 0.f));
	m_world.AddRigidBody(&m_cutterBody);

	struct CutColData { Vec3 half; Vec3 off; Vec3 rot; bool trigger; unsigned int layer; };
	const CutColData cutData[CUTTER_COLLIDER_COUNT] = {
		{ {6.22f, 1.0f, 3.0f}, { 0.0f,  5.2f, 0.f}, {0,0,0},   false, 3 },
		{ {6.22f, 1.0f, 3.0f}, {-5.2f,  0.0f, 0.f}, {0,0,90},  false, 3 },
		{ {6.22f, 1.0f, 3.0f}, { 0.0f, -5.2f, 0.f}, {0,0,0},   false, 3 },
		{ {6.22f, 1.0f, 3.0f}, { 5.2f,  0.0f, 0.f}, {0,0,90},  false, 3 },
		{ {6.22f, 6.22f, 1.0f}, {0.f, 0.f, 3.0f},   {0,0,0},   false, 3 },
		{ {4.0f,  4.0f,  1.0f}, {0.f, 0.f, 1.5f},   {0,0,0},   true,  3 },
		{ {6.22f, 6.22f, 1.0f}, {0.f, 0.f, -4.0f},  {0,0,0},   false, 4 },
	};

	for (int i = 0; i < CUTTER_COLLIDER_COUNT; ++i)
	{
		SetupBoxCollider(m_cutterColliders[i], &m_cutterBody,
			cutData[i].half, cutData[i].off, cutData[i].rot,
			cutData[i].layer, MakeMask({1}), cutData[i].trigger);
	}

	/*{
		Vec3 dp = m_deskBody.GetPosition(); Quat dq = m_deskBody.GetOrientation();
		std::cout << "[DiceSim] Desk  pos(" << dp.x << "," << dp.y << "," << dp.z
		          << ") quat(" << dq.x << "," << dq.y << "," << dq.z << "," << dq.w << ")" << std::endl;

		Vec3 cp = m_cupBody.GetPosition(); Quat cq = m_cupBody.GetOrientation();
		std::cout << "[DiceSim] Cup   pos(" << cp.x << "," << cp.y << "," << cp.z
		          << ") quat(" << cq.x << "," << cq.y << "," << cq.z << "," << cq.w << ")" << std::endl;

		Vec3 tp = m_cutterBody.GetPosition(); Quat tq = m_cutterBody.GetOrientation();
		std::cout << "[DiceSim] Cutter pos(" << tp.x << "," << tp.y << "," << tp.z
		          << ") quat(" << tq.x << "," << tq.y << "," << tq.z << "," << tq.w << ")" << std::endl;
	}*/

	// Dice 5개 설정
	for (int i = 0; i < DICE_COUNT; ++i)
	{
		m_diceBodies[i].SetBodyType(RigidBody::BodyType::Dynamic);
		m_diceBodies[i].SetMass(BODY_MASS);
		m_diceBodies[i].SetRestitution(DICE_RESTITUTION);
		m_diceBodies[i].SetFriction(DICE_FRICTION);
		m_diceBodies[i].SetLinearDamping(DICE_LINEAR_DAMP);
		m_diceBodies[i].SetAngularDamping(DICE_ANGULAR_DAMP);

		// Inertia for solid cube
		float side = DICE_HALF_EXTENT * 2.f;
		float s2 = side * side;
		float factor = BODY_MASS / 12.f;
		Mat3 inertia;
		inertia.MakeZero();
		inertia.m[0][0] = factor * (s2 + s2);
		inertia.m[1][1] = factor * (s2 + s2);
		inertia.m[2][2] = factor * (s2 + s2);
		m_diceBodies[i].SetInertia(inertia);

		m_world.AddRigidBody(&m_diceBodies[i]);

		m_diceColliders[i].halfExtents = Vec3(DICE_HALF_EXTENT, DICE_HALF_EXTENT, DICE_HALF_EXTENT);
		m_diceColliders[i].body = &m_diceBodies[i];
		m_diceColliders[i].layer = 1;
		m_diceColliders[i].layerMask = MakeMask({1, 2, 3});
		m_diceColliders[i].isTrigger = false;
		m_diceColliders[i].enabled = false;
		m_world.AddCollider(&m_diceColliders[i]);
	}
}

void DiceSimulation::SetInjection(int index, int face)
{
	if (index < 0 || index >= DICE_COUNT) return;
	if (face < 1 || face > 6) return;

	m_injection[index].active     = true; // Injection 여부는 활성화
	m_injection[index].injecting  = false; // 바로 Injection이 실행되지는 않음, SpeedThreshold > Speed일 경우 Injection
	m_injection[index].targetFace = face;
}

void DiceSimulation::ClearInjection(int index)
{
	if (index == -1) 
	{
		for (int i = 0; i < DICE_COUNT; ++i)
		{
			m_injection[i] = {};
			m_diceBodies[i].ClearOrientationTarget();
		}
	}
	else if (index >= 0 && index < DICE_COUNT)
	{
		m_injection[index] = {};
		m_diceBodies[index].ClearOrientationTarget();
	}
}

void DiceSimulation::UpdateInjections()
{
	for (int i = 0; i < DICE_COUNT; ++i)
	{
		if (!m_injection[i].active)   continue;
		if (m_injection[i].injecting) continue; // 이미 Injection 진행중

		float speed = Vec3Length(m_diceBodies[i].GetLinearVelocity());
		// 실제 Injection 주입
		if (speed < INJECT_SPEED_THRESH)
		{
			m_injection[i].injecting = true;

			const Vec3& dir = DIRECTIONS[m_injection[i].targetFace - 1];
			Quat target = QuatFromEuler(dir.x, dir.y, dir.z);
			m_diceBodies[i].SetOrientationTarget(target, INJECT_KP, INJECT_KD);
		}
	}
}

void DiceSimulation::StartThrow(uint32 seed, const std::vector<int>& heldIndices)
{
	std::mt19937 rng(seed);
	std::uniform_real_distribution<float> distPos(-0.5f, 0.5f);
	std::uniform_real_distribution<float> distSpread(-2.f, 2.f);
	std::uniform_real_distribution<float> distAngular(-15.f, 15.f);
	std::uniform_int_distribution<int>    distFace(1, 6);

	float side = DICE_HALF_EXTENT * 2.f;
	float s2 = side * side;
	float factor = BODY_MASS / 12.f;
	Mat3 diceInertia; diceInertia.MakeZero();
	diceInertia.m[0][0] = factor * (s2 + s2);
	diceInertia.m[1][1] = factor * (s2 + s2);
	diceInertia.m[2][2] = factor * (s2 + s2);

	Vec3 cupPos(4.0f, 3.5f, 8.5f);

	Vec3 spawnCenter = cupPos;
	spawnCenter.x -= 2.0f;
	spawnCenter.y -= 1.0f;

	Vec3 throwDir = Vec3Normalize(Vec3(-0.5f, -1.f, 0.f));

	// 멈춘 주사위는 넘어감,
	bool held[DICE_COUNT] = {};
	for (int idx : heldIndices)
	{
		if (idx >= 0 && idx < DICE_COUNT)
			held[idx] = true;
	}

	for (int i = 0; i < DICE_COUNT; ++i)
	{
		if (held[i])
		{
			// Held dice: keep at current position, make kinematic
			m_diceBodies[i].SetBodyType(RigidBody::BodyType::Kinematic);
			m_diceColliders[i].enabled = false;
			continue;
		}

		// Kinematic으로 설정되어 있을 경우 Dynamic으로 바꾸며 MassofInertia 설정
		m_diceBodies[i].SetBodyType(RigidBody::BodyType::Dynamic);
		m_diceBodies[i].SetMass(BODY_MASS);
		m_diceBodies[i].SetInertia(diceInertia); 
		m_diceColliders[i].enabled = true;
		m_diceColliders[i].layerMask = MakeMask({1, 2, 3});

		Vec3 startPos = spawnCenter;
		startPos.x += distPos(rng);
		startPos.y += distPos(rng) - 0.1f;
		startPos.z += distPos(rng);
		m_diceBodies[i].SetPosition(startPos);

		int initFace = distFace(rng);
		Quat initRot = QuatFromEuler(
			DIRECTIONS[initFace - 1].x,
			DIRECTIONS[initFace - 1].y,
			DIRECTIONS[initFace - 1].z);
		m_diceBodies[i].SetOrientation(initRot);

		float throwSpeed = 100.f;
		Vec3 impulse = throwDir * throwSpeed;
		impulse.x += distSpread(rng);
		impulse.y += distSpread(rng);
		impulse.z += distSpread(rng);

		Vec3 angImpulse(distAngular(rng), distAngular(rng), distAngular(rng));

		m_diceBodies[i].SetLinearVelocity(Vec3{0,0,0});
		m_diceBodies[i].SetAngularVelocity(Vec3{0,0,0});
		m_diceBodies[i].ApplyImpulseAtPoint(impulse, startPos + Vec3(distPos(rng) * 0.2f, 0.2f, 0.f));
		m_diceBodies[i].SetAngularVelocity(angImpulse);
		m_diceBodies[i].WakeUp();
	}

	m_accumulator = 0.f;
	m_stepCounter = 0;
	m_simulating = true;

	// Debug: print initial state
	std::cout << "[DiceSim] StartThrow — Cup pos: ("
	          << m_cupBody.GetPosition().x << ", "
	          << m_cupBody.GetPosition().y << ", "
	          << m_cupBody.GetPosition().z << ")" << std::endl;
	for (int i = 0; i < DICE_COUNT; ++i)
	{
		Vec3 p = m_diceBodies[i].GetPosition();
		std::cout << "[DiceSim]   Dice " << i << " pos: ("
		          << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
	}
}

bool DiceSimulation::Step(float dt)
{
	if (!m_simulating)
		return false;

	bool shouldBroadcast = false;

	while (m_accumulator >= FIXED_DT)
	{
		UpdateInjections();     // arm orientation targets before physics step
		
		m_world.Step(FIXED_DT);
		
		ProcessTriggers();
		m_accumulator -= FIXED_DT;
		++m_stepCounter;

		if (m_stepCounter % SNAPSHOT_INTERVAL == 0)
			shouldBroadcast = true;

		// Debug: 위치 기록
 	//	if (m_stepCounter % 60 == 0)
		//{
		//	std::cout << "[DiceSim] Step " << m_stepCounter << ":";
		//	for (int d = 0; d < DICE_COUNT; ++d)
		//	{
		//		Vec3 p = m_diceBodies[d].GetPosition();
		//		printf(" D%d(%.1f,%.1f,%.1f)", d, p.x, p.y, p.z);
		//	}
		//	std::cout << std::endl;
		//}
	}

	return shouldBroadcast;
}

bool DiceSimulation::IsSettled() const
{
	if (!m_simulating)
		return false;

	for (int i = 0; i < DICE_COUNT; ++i)
	{
		float speed    = Vec3Length(m_diceBodies[i].GetLinearVelocity());
		float rotSpeed = Vec3Length(m_diceBodies[i].GetAngularVelocity());

		if (speed >= SETTLE_THRESHOLD || rotSpeed >= SETTLE_THRESHOLD)
			return false;
	}

	return true;
}

std::array<DiceSimulation::DiceTransformData, DiceSimulation::DICE_COUNT> DiceSimulation::GetDiceTransforms() const
{
	std::array<DiceTransformData, DICE_COUNT> result;
	for (int i = 0; i < DICE_COUNT; ++i)
	{
		result[i].pos = m_diceBodies[i].GetPosition();
		result[i].rot = m_diceBodies[i].GetOrientation();
	}
	return result;
}

std::array<int, DiceSimulation::DICE_COUNT> DiceSimulation::GetTopFaces() const
{
	std::array<int, DICE_COUNT> faces;
	for (int i = 0; i < DICE_COUNT; ++i)
		faces[i] = DetermineTopFace(i);
	return faces;
}

int DiceSimulation::DetermineTopFace(int index) const
{
	Quat q = m_diceBodies[index].GetOrientation();

	int topFace = 1;
	float maxDot = -FLOAT__MAX;

	for (int i = 0; i < 6; ++i)
	{
		Quat ref = QuatFromEuler(DIRECTIONS[i].x, DIRECTIONS[i].y, DIRECTIONS[i].z);
		Vec3 localNormal = QuatRotateVec3(QuatConjugate(ref), UP_WORLD);
		Vec3 worldNormal = QuatRotateVec3(q, localNormal);

		float dot = Vec3Dot(worldNormal, UP_WORLD);
		if (dot > maxDot)
		{
			maxDot = dot;
			topFace = i + 1;
		}
	}

	return topFace;
}

void DiceSimulation::ProcessTriggers()
{
	const auto& triggers = m_world.GetTriggers();

	for (const auto& contact : triggers)
	{
		Collider* colA = contact.colA;
		Collider* colB = contact.colB;

		Collider* diceCol = nullptr;

		if (colA && colA->isTrigger && colA->layer == 3 && colB && colB->layer == 1)
			diceCol = colB;
		else if (colB && colB->isTrigger && colB->layer == 3 && colA && colA->layer == 1)
			diceCol = colA;

		if (diceCol)
		{
			// Enable layer bit 4 so dice collides with cutter floor (layer 4)
			diceCol->layerMask |= (1u << 4);
		}
	}
}

