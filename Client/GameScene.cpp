#include "ClientPch.h"
#include <cstdlib>
#include <ctime>
#include "GameScene.h"
#include "GameObject.h"
#include "RigidBodyComponent.h"
#include "RigidBody.h"
#include "Transform.h"
#include "Operation.h"
#include "EngineGlobal.h"
#include "PhysicsManager.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "FontManager.h"
#include "AudioManager.h"
#include "DiceCup.h"
#include "Dice.h"
#include "NetworkEvents.h"
#include "ClientSession.h"
#include "PhysicsConvert.h"

#include "Ray.h"

#ifdef _DEBUG
#include "imgui.h"
#endif

using namespace GameEngine;
using namespace DirectX;

NetworkEventQueue GNetEvents;

namespace
{
	constexpr int DICE_COUNT = 5;
	constexpr float DICE_SPACE = 1.5f;

	constexpr float3 CUTTER_POS = float3(-1.55f, 2.4f, 13.5f);
	constexpr float3 DICE_CENTER_POS = float3(1.45f, 7.5f, 8.5f);

	constexpr unsigned int DICE_RAYCAST_LAYER = 1u << 1;

	bool CheckAllDiceState(const std::vector<Dice*>& dices, Dice::State state)
	{
		if (dices.empty())
			return false;

		for (auto* d : dices)
		{
			if (!d || !d->CheckState(state))
				return false;
		}
		return true;
	}

	float3 GetShownPosition(int count)
	{
		float3 ret = DICE_CENTER_POS;
		int div = count / 2;

		if (0 == count % 2)
			ret.x -= div * DICE_SPACE - DICE_SPACE * 0.5f;
		else
			ret.x -= div * DICE_SPACE;

		return ret;
	}

	float3 GetChosenPosition(int index)
	{
		float3 ret = CUTTER_POS;
		ret.x += DICE_SPACE * index;
		return ret;
	}
}
void GameScene::Awake()
{
	// Font must be loaded before JSON deserialization (sync only)
	FontManager::GetInstance().LoadFont(
		"Default", L"Bin/Resource/Font/LiberationSans.spritefont");

	// Scene JSON deserialization (sync — creates GameObjects for Start())
	LoadFromFile("Bin/Data/Scene/GameScene.json");

	// Audio: load asynchronously on worker threads
	constexpr int32 ASYNC_AUDIO_COUNT = 13;
	BeginAsyncLoadTracking(ASYNC_AUDIO_COUNT);

	auto counter = m_asyncLoadCounter;
	auto onDone = [counter](AudioClip*) { counter->Signal(); };

	GAudioManager->LoadAsync("dice_hit1", L"./Bin/Resource/Audio/dice1.wav", onDone);
	GAudioManager->LoadAsync("dice_hit2", L"./Bin/Resource/Audio/dice2.wav", onDone);
	GAudioManager->LoadAsync("dice_hit3", L"./Bin/Resource/Audio/dice3.wav", onDone);
	GAudioManager->LoadAsync("dice_hit4", L"./Bin/Resource/Audio/dice4.wav", onDone);
	GAudioManager->LoadAsync("dice_hit5", L"./Bin/Resource/Audio/dice5.wav", onDone);
	GAudioManager->LoadAsync("dice_hit6", L"./Bin/Resource/Audio/dice6.wav", onDone);

	GAudioManager->LoadAsync("Roller1", L"./Bin/Resource/Audio/RollerAndDice.wav", onDone);
	GAudioManager->LoadAsync("Roller2", L"./Bin/Resource/Audio/RollerAndDice2.wav", onDone);

	GAudioManager->LoadAsync("Score", L"./Bin/Resource/Audio/GetScore.wav", onDone);
	GAudioManager->LoadAsync("NonScore", L"./Bin/Resource/Audio/NonScore.wav", onDone);

	GAudioManager->LoadAsync("ScoreNext", L"./Bin/Resource/Audio/GetScoreNext.wav", onDone);
	GAudioManager->LoadAsync("NonScoreNext", L"./Bin/Resource/Audio/NonScoreNext.wav", onDone);

	GAudioManager->LoadAsync("BGM", L"./Bin/Resource/Audio/SunnyDay.wav", onDone);

	GAudioManager->SetBGMVolume(0.7f);

	srand(static_cast<unsigned>(time(nullptr)));
}

void GameScene::Start()
{
	__super::Start();

	const char* names[DICE_COUNT] = { "FallingCube", "FallingCube2", "FallingCube3", "FallingCube4", "FallingCube5" };

	for (auto& go : m_gameObjects)
	{
		for (int i = 0; i < DICE_COUNT; ++i)
		{
			if (go->GetName() == names[i])
			{
				auto dice = go->GetComponent<Dice>();
				if (nullptr != dice)
				{
					m_allDice[i] = dice;
					m_activeDice.push_back(dice);
				}
			}
		}

		if (go->GetName() == "DiceCup")
			m_diceCup = go->GetComponent<DiceCup>();

		if (go->GetName() == "HoverTL")
			m_hoverTL = go.get();
	}

	// ScoreboardUI — player count updated dynamically via S_PLAYER_JOINED
	m_scoreboardUI.Initialize();
	m_scoreboardUI.SetPlayerSize(1);
	//GAudioManager->PlayBGM("BGM");

	SetState(GameState::IDLE);
}


void GameScene::Update(float dt)
{
	ProcessNetworkEvents();
	UpdateInput();
	UpdateState(dt);

#ifdef _DEBUG
	{
		auto session = GClientSession.lock();
		if (session && session->GetPlayerId() == 1)
			DrawDicePanel();
	}
	DrawNetworkPanel();
#endif

	Scene::Update(dt);
}

void GameScene::SetState(GameState state)
{
	switch (state)
	{
	case GameState::IDLE:

		if (m_throwCount <= 0 && m_isMyTurn)
		{
			NextTurn();
		}

		if (m_diceCup)
			m_diceCup->Idle();

		m_scoreboardUI.HideTemporaryScores();

		if (m_hoverTL)
		{
			m_hoverTL->SetActive(false);
		}

		PoseDices();

		break;

	case GameState::SHAKE:
		if (m_diceCup)
			m_diceCup->Shake();

		SendCupShake();
		PoseDices();

		break;
	case GameState::THROW:
		if (m_diceCup)
			m_diceCup->Flip();

		SendCupFlip();

		--m_throwCount;
		break;
	case GameState::SETTLE:
		// Wait for S_ROLL_SETTLED — Return is called in ProcessNetworkEvents
		break;

	case GameState::SELECT:
		DetermineTemporaryScore();

		RepositionActiveDice();
		if (m_throwCount <= 0)
		{
			FinalizeAllDice();
		}

		break;
	default:
		break;
	}

	m_gameState = state;
}

void GameScene::PoseDices()
{
	int activeCount = static_cast<int>(m_activeDice.size());

	for (int i = 0; i < activeCount; ++i)
	{
		if (!m_activeDice[i])
			continue;

		float3 pos = m_diceCup ? m_diceCup->GetPosition() : float3();

		float3 posoffset;
		XMStoreFloat3(&posoffset, XMVector3TransformCoord(LookVector(),
			XMMatrixRotationAxis(UpVector(), XMConvertToRadians(360.f) * i / activeCount)));

		pos.x += posoffset.x;
		pos.z += posoffset.z;

		int rotDir = 0;

		if (THROW_COUNT == m_throwCount)
			rotDir = RandInt(1, 6);
		else
			rotDir = m_activeDice[i]->GetTopFace();

		m_activeDice[i]->PoseInCup(rotDir, pos);
	}
}


void GameScene::NextTurn()
{
	for (auto* dice : m_chosedDice)
	{
		dice->SetSlot(-1);
		m_activeDice.push_back(dice);
	}

	m_chosedDice.clear();
	m_throwCount = THROW_COUNT;
}

void GameScene::UpdateState(float dt)
{
	switch (m_gameState)
	{
	case GameState::THROW:
		if (m_diceCup && m_diceCup->IsFlipFinished())
		{
			// Flip done — request server to start physics
			SendThrowRequest();

			// Switch dice to network-driven (kinematic, server controls position)
			for (auto* dice : m_activeDice)
			{
				if (dice) dice->SetNetworkDriven(true);
			}

			SetState(GameState::SETTLE);
		}

		break;
	case GameState::SETTLE:
		// Wait for S_ROLL_SETTLED from server (handled in ProcessNetworkEvents)
		break;
	case GameState::SELECT:
	{
		if (m_throwCount <= 0)
		{

		}
		else
			UpdateDiceSelectionUI();

		if (m_scoreboardUI.IsSelected())
		{
			SendSelectScore(m_scoreboardUI.GetSelectedCategory());
			m_throwCount = 0;
			SetState(GameState::IDLE);
		}
	}
		break;
	default:
		break;
	}

}

void GameScene::UpdateInput()
{
	auto& input = InputManager::GetInstance();

	// reset game (debug)
	if (input.IsKeyPressed(Keyboard::R))
	{
		SetState(GameState::IDLE);
		m_scoreboardUI.ResetGame();
	}

	// Block all throw input when it's not our turn
	if (!m_isMyTurn)
		return;

	// IDLE/SELECT → SHAKE → THROW: local flow (same as original)
	if (input.IsKeyPressed(Keyboard::Space))
	{
		if (m_throwCount > 0 && CheckState(GameState::IDLE))
			SetState(GameState::SHAKE);
		else if (CheckState(GameState::SHAKE))
			SetState(GameState::THROW);
		else if (0 != m_throwCount && CheckState(GameState::SELECT))
		{
			SendDiceToCup();
			SetState(GameState::IDLE);
		}
	}

	if (input.IsKeyPressed(Keyboard::S))
	{
		if (CheckState(GameState::SHAKE))
			SetState(GameState::IDLE);
		else if (THROW_COUNT > m_throwCount && CheckState(GameState::IDLE))
			SetState(GameState::SELECT);
	}
}

void GameScene::UpdateDiceSelectionUI()
{
	int activeCount = static_cast<int>(m_activeDice.size());
	int chosedCount = static_cast<int>(m_chosedDice.size());

	auto& input = InputManager::GetInstance();

	Ray ray = CameraManager::GetInstance().ScreenToWorldRay(DICE_RAYCAST_LAYER);
	RayHit hit;

	if (GPhysicsManager->Raycast(ray, hit))
	{
		int selectIndex = FindDiceIndex(m_activeDice, hit);
		int choseIndex  = FindDiceIndex(m_chosedDice, hit);

		if (m_chosedIndex != choseIndex && -1 != choseIndex)
		{
			if (m_hoverTL)
			{
				m_hoverTL->SetActive(true);

				float3 pos = m_chosedDice[choseIndex]->GetOwner()->GetTransform()->GetPosition();
				pos.y += 0.4f;
				m_hoverTL->GetTransform()->SetPosition(pos);
			}

			m_chosedIndex = choseIndex;
			m_selectedIndex = -1;
		}
		else if (m_selectedIndex != selectIndex && -1 != selectIndex)
		{
			if (m_hoverTL)
			{
				m_hoverTL->SetActive(true);

				float3 pos = m_activeDice[selectIndex]->GetOwner()->GetTransform()->GetPosition();
				pos.y += 0.4f;
				m_hoverTL->GetTransform()->SetPosition(pos);
			}

			m_selectedIndex = selectIndex;
			m_chosedIndex = -1;
		}
	}
	else if (-1 != m_selectedIndex || -1 != m_chosedIndex)
	{
		if (m_hoverTL)
		{
			m_hoverTL->SetActive(false);
		}
		m_selectedIndex = -1;
		m_chosedIndex = -1;
	}

	if (input.IsMouseButtonPressed(0))
	{
		if (-1 != m_selectedIndex)
			ChooseDice(m_selectedIndex);
		else if (-1 != m_chosedIndex)
			ReturnDice(m_chosedIndex);
	}
}

void GameScene::ChooseDice(int index)
{
	Dice* dice = m_activeDice[index];
	m_activeDice.erase(m_activeDice.begin() + index);

	int slot = FindEmptySlot();
	dice->SetSlot(slot);

	m_chosedDice.push_back(dice);

	dice->PoseAtCutter(GetChosenPosition(slot));

	RepositionActiveDice();

	for (int i = 0; i < DICE_COUNT; ++i)
	{
		if (m_allDice[i] == dice) { SendDiceSelect(i, slot); break; }
	}
}

void GameScene::ReturnDice(int index)
{
	Dice* dice = m_chosedDice[index];
	dice->SetSlot(-1);

	m_chosedDice.erase(m_chosedDice.begin() + index);
	m_activeDice.push_back(dice);

	RepositionActiveDice();

	for (int i = 0; i < DICE_COUNT; ++i)
	{
		if (m_allDice[i] == dice) { SendDiceReturn(i); break; }
	}
}

void GameScene::RepositionActiveDice()
{
	int activeCount = static_cast<int>(m_activeDice.size());
	float3 pos = GetShownPosition(activeCount);

	for (int i = 0; i < activeCount; ++i)
	{
		if (m_activeDice[i])
			m_activeDice[i]->PoseAtCamera(pos);

		pos.x += DICE_SPACE;
	}
}

void GameScene::FinalizeAllDice()
{
	int chosedCount = static_cast<int>(m_chosedDice.size());

	for (auto* dice : m_activeDice)
	{
		int slot = FindEmptySlot();
		dice->SetSlot(slot);

		m_chosedDice.push_back(dice);
		dice->PoseAtCutter(GetChosenPosition(slot));
		++chosedCount;
	}

	m_activeDice.clear();
}

void GameScene::DetermineTemporaryScore()
{
	std::vector<int> diceValue;

	for (auto* dice : m_activeDice)
	{
		if (nullptr != dice)
			diceValue.push_back(dice->GetTopFace());
	}
	for (auto* dice : m_chosedDice)
	{
		if (nullptr != dice)
			diceValue.push_back(dice->GetTopFace());
	}

	m_scoreboardUI.CalculateScores(diceValue);
	m_scoreboardUI.ShowTemporaryScores();
}

int GameScene::FindDiceIndex(const std::vector<Dice*>& dices, const RayHit& hit)
{
	for (int i = 0; i < static_cast<int>(dices.size()); ++i)
	{
		if (!dices[i]) continue;
		auto* rb = dices[i]->GetOwner()->GetComponent<RigidBodyComponent>();
		if (rb && rb->GetRigidBody() == hit.collider->body)
			return i;
	}
	return -1;
}

int GameScene::FindEmptySlot()
{
	bool used[DICE_COUNT] = {};
	for (auto* dice : m_chosedDice)
	{
		if (nullptr != dice && -1 != dice->GetSlot())
			used[dice->GetSlot()] = true;
	}
	for (int i = 0; i < DICE_COUNT; ++i)
	{
		if (!used[i])
			return i;
	}
	return -1;
}

void GameScene::SendThrowRequest()
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined())
		return;

	Protocol::C_THROW_DICE pkt;

	// Add held dice indices (dice in m_chosedDice)
	for (auto* dice : m_chosedDice)
	{
		for (int i = 0; i < DICE_COUNT; ++i)
		{
			if (m_allDice[i] == dice)
			{
				pkt.add_held_indices(i);
				break;
			}
		}
	}

	session->Send(PacketHelper::BuildPacket(PacketId::C_THROW_DICE, pkt));
}

void GameScene::SendSelectScore(int category)
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined())
		return;

	Protocol::C_SELECT_SCORE pkt;
	pkt.set_category(category);
	session->Send(PacketHelper::BuildPacket(PacketId::C_SELECT_SCORE, pkt));
}

void GameScene::SendCupShake()
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined()) return;
	Protocol::C_CUP_SHAKE pkt;
	session->Send(PacketHelper::BuildPacket(PacketId::C_CUP_SHAKE, pkt));
}

void GameScene::SendCupFlip()
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined()) return;
	Protocol::C_CUP_FLIP pkt;
	session->Send(PacketHelper::BuildPacket(PacketId::C_CUP_FLIP, pkt));
}

void GameScene::SendDiceSelect(int diceIndex, int slotIndex)
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined()) return;
	Protocol::C_DICE_SELECT pkt;
	pkt.set_dice_index(diceIndex);
	pkt.set_slot_index(slotIndex);
	session->Send(PacketHelper::BuildPacket(PacketId::C_DICE_SELECT, pkt));
}

void GameScene::SendDiceReturn(int diceIndex)
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined()) return;
	Protocol::C_DICE_RETURN pkt;
	pkt.set_dice_index(diceIndex);
	session->Send(PacketHelper::BuildPacket(PacketId::C_DICE_RETURN, pkt));
}

void GameScene::SendDiceToCup()
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined()) return;
	Protocol::C_DICE_TO_CUP pkt;
	session->Send(PacketHelper::BuildPacket(PacketId::C_DICE_TO_CUP, pkt));
}

void GameScene::SendInjectRequest(int diceIndex, int face)
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined())
		return;

	Protocol::C_INJECT_ORIENTATION pkt;
	pkt.set_dice_index(diceIndex);
	pkt.set_target_face(face);
	session->Send(PacketHelper::BuildPacket(PacketId::C_INJECT_ORIENTATION, pkt));
}

void GameScene::SendClearInject(int diceIndex)
{
	auto session = GClientSession.lock();
	if (!session || !session->IsJoined())
		return;

	Protocol::C_CLEAR_INJECT pkt;
	pkt.set_dice_index(diceIndex);
	session->Send(PacketHelper::BuildPacket(PacketId::C_CLEAR_INJECT, pkt));
}

void GameScene::ProcessNetworkEvents()
{
	// S_JOIN_RESULT follow-up: set player count from server (handles late-joining clients)
	NetEvent_JoinComplete joinComplete;
	while (GNetEvents.PopJoinComplete(joinComplete))
	{
		m_playerCount = joinComplete.playerCount;
		m_scoreboardUI.SetPlayerSize(m_playerCount);
	}

	// S_PLAYER_JOINED: another player connected — update scoreboard player count
	NetEvent_PlayerJoined playerJoined;
	while (GNetEvents.PopPlayerJoined(playerJoined))
	{
		++m_playerCount;
		m_scoreboardUI.SetPlayerSize(m_playerCount);
	}

	// S_DICE_SELECT: opponent moved a die to cutter
	NetEvent_DiceSelect diceSelect;
	while (GNetEvents.PopDiceSelect(diceSelect))
	{
		Dice* dice = m_allDice[diceSelect.diceIndex];
		if (!dice) continue;

		// move from m_activeDice to m_chosedDice
		auto it = std::find(m_activeDice.begin(), m_activeDice.end(), dice);
		if (it != m_activeDice.end()) m_activeDice.erase(it);

		dice->SetSlot(diceSelect.slotIndex);
		m_chosedDice.push_back(dice);
		dice->PoseAtCutter(GetChosenPosition(diceSelect.slotIndex));
		RepositionActiveDice();
	}

	// S_DICE_RETURN: opponent returned a die from cutter
	NetEvent_DiceReturn diceReturn;
	while (GNetEvents.PopDiceReturn(diceReturn))
	{
		Dice* dice = m_allDice[diceReturn.diceIndex];
		if (!dice) continue;

		// move from m_chosedDice to m_activeDice
		auto it = std::find(m_chosedDice.begin(), m_chosedDice.end(), dice);
		if (it != m_chosedDice.end()) m_chosedDice.erase(it);

		dice->SetSlot(-1);
		m_activeDice.push_back(dice);
		RepositionActiveDice();
	}

	// S_DICE_TO_CUP: turn player went SELECT -> IDLE (about to re-shake), move dice to cup
	NetEvent_DiceToCup diceToCup;
	while (GNetEvents.PopDiceToCup(diceToCup))
	{
		if (!m_isMyTurn) PoseDices();
	}

	// S_CUP_SHAKE: opponent started shaking — play cup shake animation + move dice into cup
	NetEvent_CupShake cupShake;
	while (GNetEvents.PopCupShake(cupShake))
	{
		if (m_diceCup) m_diceCup->Shake();
		if (!m_isMyTurn) PoseDices();
	}

	// S_CUP_FLIP: opponent threw — play cup flip animation
	NetEvent_CupFlip cupFlip;
	while (GNetEvents.PopCupFlip(cupFlip))
	{
		if (m_diceCup) m_diceCup->Flip();
	}

	// S_ROLL_START: physics simulation started
	NetEvent_RollStart rollStart;
	while (GNetEvents.PopRollStart(rollStart))
	{
		m_throwCount = rollStart.rollCount;

		if (!m_isMyTurn)
		{
			// Opponent's throw — switch only active (non-held) dice to network-driven
			for (auto* dice : m_activeDice)
			{
				if (dice) dice->SetNetworkDriven(true);
			}
			m_gameState = GameState::SETTLE;
		}
	}

	// S_DICE_SNAPSHOT: apply server dice transforms (skip held/cutter dice)
	NetEvent_DiceSnapshot snapshot;
	while (GNetEvents.PopSnapshot(snapshot))
	{
		bool held[DICE_COUNT] = {};
		for (auto* dice : m_chosedDice)
			for (int i = 0; i < DICE_COUNT; ++i)
				if (m_allDice[i] == dice) { held[i] = true; break; }

		for (int i = 0; i < DICE_COUNT; ++i)
		{
			if (m_allDice[i] && !held[i])
				m_allDice[i]->SetNetworkTransform(
					snapshot.diceTransforms[i].pos,
					snapshot.diceTransforms[i].rot);
		}
	}

	// S_SCORE_SELECTED must be processed BEFORE S_TURN_CHANGE
	// so m_nowPlayer still points to the scoring player when RecordOpponentScore runs
	NetEvent_ScoreSelected scoreSelected;
	while (GNetEvents.PopScoreSelected(scoreSelected))
	{
		m_scoreboardUI.RecordOpponentScore(scoreSelected.category);
	}

	// S_TURN_CHANGE: update whose turn it is (advances m_nowPlayer — must come after ScoreSelected)
	NetEvent_TurnChange turnChange;
	while (GNetEvents.PopTurnChange(turnChange))
	{
		m_currentTurnPlayerId = turnChange.currentPlayerId;

		auto session = GClientSession.lock();
		bool prevIsMyTurn = m_isMyTurn;
		m_isMyTurn = !session || (session->GetPlayerId() == m_currentTurnPlayerId);

		if (m_diceCup) m_diceCup->SetOwnerTurn(m_isMyTurn);

		// Always sync scoreboard zone and turn counter from server data
		m_scoreboardUI.SetCurrentPlayer(m_currentTurnPlayerId, turnChange.turnNumber);

		if (m_isMyTurn && !prevIsMyTurn)
		{
			// My turn just started — rebuild state for new turn
			m_throwCount = THROW_COUNT;

			m_activeDice.clear();
			for (auto* dice : m_chosedDice)
				dice->SetSlot(-1);
			m_chosedDice.clear();
			for (int i = 0; i < DICE_COUNT; ++i)
			{
				if (m_allDice[i]) m_activeDice.push_back(m_allDice[i]);
			}

			SetState(GameState::IDLE);
		}
	}

	// S_GAME_OVER
	NetEvent_GameOver gameOver;
	while (GNetEvents.PopGameOver(gameOver))
	{
		SetState(GameState::IDLE);
		m_scoreboardUI.ResetGame();
	}

	// S_ROLL_SETTLED: finalize dice positions
	NetEvent_RollSettled settled;
	while (GNetEvents.PopSettled(settled))
	{
		if (!CheckState(GameState::SETTLE))
			continue;

		m_throwCount = settled.rollCount;

		if (m_isMyTurn)
		{
			// My throw settled — skip held dice, rebuild active list, go to SELECT
			bool heldDice[DICE_COUNT] = {};
			for (auto* dice : m_chosedDice)
			{
				for (int i = 0; i < DICE_COUNT; ++i)
				{
					if (m_allDice[i] == dice) { heldDice[i] = true; break; }
				}
			}

			for (int i = 0; i < DICE_COUNT; ++i)
			{
				if (m_allDice[i] && !heldDice[i])
					m_allDice[i]->ApplyServerSettle(
						settled.diceValues[i],
						settled.finalTransforms[i].pos,
						settled.finalTransforms[i].rot);
			}

			m_activeDice.clear();
			for (int i = 0; i < DICE_COUNT; ++i)
			{
				if (m_allDice[i] && m_allDice[i]->GetSlot() == -1)
					m_activeDice.push_back(m_allDice[i]);
			}

			SetState(GameState::SELECT);
		}
		else
		{
			// Opponent's throw settled — apply only active (non-held) dice, reposition to camera
			bool held[DICE_COUNT] = {};
			for (auto* dice : m_chosedDice)
				for (int i = 0; i < DICE_COUNT; ++i)
					if (m_allDice[i] == dice) { held[i] = true; break; }

			for (int i = 0; i < DICE_COUNT; ++i)
			{
				if (m_allDice[i] && !held[i])
					m_allDice[i]->ApplyServerSettle(
						settled.diceValues[i],
						settled.finalTransforms[i].pos,
						settled.finalTransforms[i].rot);
			}

			// Pre-compute scores so RecordOpponentScore can use them when S_SCORE_SELECTED arrives
			m_scoreboardUI.CalculateScores(
				std::vector<int>(settled.diceValues, settled.diceValues + DICE_COUNT));

			if (m_throwCount <= 0)
				FinalizeAllDice();
			else
				RepositionActiveDice();

			m_gameState = GameState::IDLE;
		}
	}
}

#ifdef _DEBUG

void GameScene::DrawNetworkPanel()
{
	ImGui::Begin("Network Log");

	auto session = GClientSession.lock();
	if (session)
	{
		if (session->IsJoined())
		{
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Connected (Player %d)", session->GetPlayerId());
		}
		else
		{
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Connecting...");
		}

		ImGui::Separator();

		const auto& logs = session->GetLogs();
		for (const auto& log : logs)
		{
			ImGui::TextWrapped("%s", log.c_str());
		}

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
	}
	else
	{
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Not connected");
	}

	ImGui::Separator();
	ImGui::Text("Dice State:");
	const char* stateNames[] = { "IDLE", "MOVE", "THROWN", "NET_DRIVEN" };
	for (int i = 0; i < DICE_COUNT; ++i)
	{
		if (!m_allDice[i]) continue;
		auto* tr = m_allDice[i]->GetOwner()->GetTransform();
		float3 pos = tr->GetPosition();
		int state = static_cast<int>(m_allDice[i]->CheckState(Dice::State::IDLE) ? 0 :
			m_allDice[i]->CheckState(Dice::State::MOVE) ? 1 :
			m_allDice[i]->CheckState(Dice::State::THROWN) ? 2 : 3);
		ImGui::Text("D%d [%s] pos(%.1f, %.1f, %.1f) face:%d",
			i, stateNames[state], pos.x, pos.y, pos.z, m_allDice[i]->GetTopFace());
	}

	ImGui::End();
}

void GameScene::DrawDicePanel()
{
	int activeCount = static_cast<int>(m_activeDice.size());

	ImGui::Begin("Dice Control");

	static int dirIndex[DICE_COUNT] = { 0, 0, 0, 0, 0 };
	static int topFaceResult[DICE_COUNT] = { 0, 0, 0, 0, 0 };

	for (int i = 0; i < activeCount; ++i)
	{
		ImGui::PushID(i);
		ImGui::Combo("Direction", &dirIndex[i], " 1\0 2\0 3\0 4\0 5\0 6\0");
		ImGui::PopID();
		ImGui::Separator();
	}

	if (ImGui::Button("Inject Orientation", ImVec2(-1, 40)))
	{
		for (int i = 0; i < activeCount; ++i)
		{
			if (!m_activeDice[i]) continue;
			for (int j = 0; j < DICE_COUNT; ++j)
			{
				if (m_allDice[j] == m_activeDice[i])
				{
					SendInjectRequest(j, dirIndex[i] + 1);
					break;
				}
			}
		}
	}

	if (ImGui::Button("Clear Inject", ImVec2(-1, 30)))
	{
		SendClearInject(-1);
	}

	ImGui::Separator();

	for (int i = 0; i < activeCount; ++i)
	{
		if (!m_activeDice[i])
			continue;

		ImGui::PushID(100 + i);
		char label[64];
		snprintf(label, sizeof(label), "Dice %d TopFace", i + 1);
		if (ImGui::Button(label))
			topFaceResult[i] = m_activeDice[i]->GetTopFace();

		ImGui::SameLine();
		ImGui::Text("= %d", topFaceResult[i]);
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::Text("Audio Test");
	for (int i = 1; i <= 6; ++i)
	{
		char btn[32];
		snprintf(btn, sizeof(btn), "dice_hit%d", i);
		if (ImGui::Button(btn, ImVec2(80, 0)))
			GAudioManager->PlaySFX(btn);
		if (i < 6) ImGui::SameLine();
	}

	ImGui::End();
}
#endif
