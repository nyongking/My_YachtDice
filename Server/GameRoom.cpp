#include "ServerPch.h"
#include "GameRoom.h"
#include "GameSession.h"
#include <random>

std::shared_ptr<GameRoom> GGameRoom;

void GameRoom::Enter(std::shared_ptr<GameSession> session)
{
	WRITE_LOCK;

	uint32 playerId = m_nextPlayerId++;
	session->SetPlayerId(playerId);
	session->SetGameRoom(shared_from_this());
	m_sessions[playerId] = session;

	std::cout << "[GameRoom] Player " << playerId << " entered. Total: " << m_sessions.size() << std::endl;

	if (!m_initialized)
	{
		m_diceSimulation.Initialize();
		m_initialized = true;
	}

	m_turnOrder.push_back(playerId);
	m_playerTurnData[playerId] = PlayerTurnData{};

	Protocol::S_JOIN_RESULT joinResult;
	joinResult.set_success(true);
	joinResult.set_player_id(playerId);
	joinResult.set_player_count(static_cast<int>(m_sessions.size()));
	session->Send(PacketHelper::BuildPacket(PacketId::S_JOIN_RESULT, joinResult));
		
	Protocol::S_PLAYER_JOINED playerJoined;
	playerJoined.set_player_id(playerId);
	playerJoined.set_player_name(session->GetPlayerName());
	RefSendBuffer sendBuffer = PacketHelper::BuildPacket(PacketId::S_PLAYER_JOINED, playerJoined);

	for (auto& [id, s] : m_sessions)
	{
		if (id != playerId)
			s->Send(sendBuffer);
	}

	BroadcastTurnChange();
}

void GameRoom::Leave(std::shared_ptr<GameSession> session)
{
	WRITE_LOCK;

	uint32 playerId = session->GetPlayerId();
	m_sessions.erase(playerId);
	session->SetGameRoom(nullptr);

	std::cout << "[GameRoom] Player " << playerId << " left. Total: " << m_sessions.size() << std::endl;

	Protocol::S_PLAYER_LEFT playerLeft;
	playerLeft.set_player_id(playerId);
	RefSendBuffer sendBuffer = PacketHelper::BuildPacket(PacketId::S_PLAYER_LEFT, playerLeft);

	for (auto& [id, s] : m_sessions)
	{
		s->Send(sendBuffer);
	}
}

void GameRoom::Tick(float dt)
{
	WRITE_LOCK;

	if (!m_isSimulating)
		return;

	// shouldBroadCast: Snapshot 전송 여부
	bool shouldBroadcast = m_diceSimulation.Step(dt);

	if (shouldBroadcast)
		BroadcastSnapshot();

	if (m_diceSimulation.IsSettled())
		OnSimulationSettled();
}

void GameRoom::HandleThrowRequest(uint32 playerId, const std::vector<int>& heldIndices)
{
	WRITE_LOCK;

	if (m_isSimulating)
	{
		std::cout << "[GameRoom] Throw rejected: simulation in progress." << std::endl;
		return;
	}

	if (m_rollCount <= 0)
	{
		std::cout << "[GameRoom] Throw rejected: no rolls remaining." << std::endl;
		return;
	}

	if (!m_turnOrder.empty() && m_turnOrder[m_currentTurnIndex] != playerId)
	{
		std::cout << "[GameRoom] Throw rejected: not player " << playerId << "'s turn." << std::endl;
		return;
	}

	std::random_device rd;
	uint32 seed = rd();

	m_diceSimulation.StartThrow(seed, heldIndices);
	m_isSimulating = true;
	--m_rollCount;

	std::cout << "[GameRoom] Throw by player " << playerId
	          << ". Rolls remaining: " << m_rollCount << std::endl;

	Protocol::S_ROLL_START rollStart;
	rollStart.set_roll_count(m_rollCount);
	BroadCast(PacketHelper::BuildPacket(PacketId::S_ROLL_START, rollStart));
}

void GameRoom::BroadcastSnapshot()
{
	auto transforms = m_diceSimulation.GetDiceTransforms();

	Protocol::S_DICE_SNAPSHOT snapshot;
	for (int i = 0; i < DiceSimulation::DICE_COUNT; ++i)
	{
		FillDiceTransform(snapshot.add_dice_transforms(),
			transforms[i].pos, transforms[i].rot);
	}

	BroadCast(PacketHelper::BuildPacket(PacketId::S_DICE_SNAPSHOT, snapshot));
}

void GameRoom::OnSimulationSettled()
{
	m_isSimulating = false;

	m_diceSimulation.ClearInjection(-1);

	auto faces = m_diceSimulation.GetTopFaces();
	auto transforms = m_diceSimulation.GetDiceTransforms();

	std::cout << "[GameRoom] Dice settled. Faces:";
	for (int i = 0; i < DiceSimulation::DICE_COUNT; ++i)
		std::cout << " " << faces[i];
	std::cout << std::endl;

	Protocol::S_ROLL_SETTLED settled;
	settled.set_roll_count(m_rollCount);
	for (int i = 0; i < DiceSimulation::DICE_COUNT; ++i)
	{
		settled.add_dice_values(faces[i]);
		FillDiceTransform(settled.add_final_transforms(),
			transforms[i].pos, transforms[i].rot);
	}

	BroadCast(PacketHelper::BuildPacket(PacketId::S_ROLL_SETTLED, settled));
}

void GameRoom::BroadCast(RefSendBuffer sendBuffer)
{
	for (auto& [id, session] : m_sessions)
		session->Send(sendBuffer);
}

void GameRoom::BroadcastExcept(RefSendBuffer sendBuffer, uint32 excludePlayerId)
{
	for (auto& [id, session] : m_sessions)
	{
		if (id != excludePlayerId)
			session->Send(sendBuffer);
	}
}

void GameRoom::FillDiceTransform(Protocol::DiceTransform* proto, const Vec3& pos, const Quat& rot)
{
	proto->set_px(pos.x); proto->set_py(pos.y); proto->set_pz(pos.z);
	proto->set_qx(rot.x); proto->set_qy(rot.y); proto->set_qz(rot.z); proto->set_qw(rot.w);
}

void GameRoom::HandleCupShake(uint32 playerId)
{
	WRITE_LOCK;

	if (m_turnOrder.empty() || m_turnOrder[m_currentTurnIndex] != playerId)
		return;

	Protocol::S_CUP_SHAKE pkt;
	BroadcastExcept(PacketHelper::BuildPacket(PacketId::S_CUP_SHAKE, pkt), playerId);
}

void GameRoom::HandleCupFlip(uint32 playerId)
{
	WRITE_LOCK;

	if (m_turnOrder.empty() || m_turnOrder[m_currentTurnIndex] != playerId)
		return;

	Protocol::S_CUP_FLIP pkt;
	BroadcastExcept(PacketHelper::BuildPacket(PacketId::S_CUP_FLIP, pkt), playerId);
}

void GameRoom::HandleInjectRequest(uint32 playerId, int diceIndex, int face)
{
	WRITE_LOCK;

	if (playerId != 1)
	{
		std::cout << "[GameRoom] InjectOrientation rejected: not host (player " << playerId << ")." << std::endl;
		return;
	}

	m_diceSimulation.SetInjection(diceIndex, face);
	std::cout << "[GameRoom] Inject dice " << diceIndex << " -> face " << face << std::endl;
}

void GameRoom::HandleClearInject(uint32 playerId, int diceIndex)
{
	WRITE_LOCK;

	if (playerId != 1)
		return;

	m_diceSimulation.ClearInjection(diceIndex);
	std::cout << "[GameRoom] ClearInject dice " << diceIndex << std::endl;
}

void GameRoom::HandleDiceSelect(uint32 playerId, int diceIndex, int slotIndex)
{
	WRITE_LOCK;

	if (m_turnOrder.empty() || m_turnOrder[m_currentTurnIndex] != playerId)
		return;

	Protocol::S_DICE_SELECT pkt;
	pkt.set_dice_index(diceIndex);
	pkt.set_slot_index(slotIndex);
	BroadcastExcept(PacketHelper::BuildPacket(PacketId::S_DICE_SELECT, pkt), playerId);
}

void GameRoom::HandleDiceReturn(uint32 playerId, int diceIndex)
{
	WRITE_LOCK;

	if (m_turnOrder.empty() || m_turnOrder[m_currentTurnIndex] != playerId)
		return;

	Protocol::S_DICE_RETURN pkt;
	pkt.set_dice_index(diceIndex);
	BroadcastExcept(PacketHelper::BuildPacket(PacketId::S_DICE_RETURN, pkt), playerId);
}

void GameRoom::HandleDiceToCup(uint32 playerId)
{
	WRITE_LOCK;

	if (m_turnOrder.empty() || m_turnOrder[m_currentTurnIndex] != playerId)
		return;

	Protocol::S_DICE_TO_CUP pkt;
	BroadcastExcept(PacketHelper::BuildPacket(PacketId::S_DICE_TO_CUP, pkt), playerId);
}

void GameRoom::HandleSelectScore(uint32 playerId, int category)
{
	WRITE_LOCK;

	if (m_turnOrder.empty())
		return;

	if (m_turnOrder[m_currentTurnIndex] != playerId)
	{
		std::cout << "[GameRoom] SelectScore rejected: not player " << playerId << "'s turn." << std::endl;
		return;
	}

	if (category < 0 || category >= MAX_ROUNDS)
	{
		std::cout << "[GameRoom] SelectScore rejected: invalid category " << category << std::endl;
		return;
	}

	auto& data = m_playerTurnData[playerId];
	if (data.filledCategories[category])
	{
		std::cout << "[GameRoom] SelectScore rejected: category " << category << " already filled." << std::endl;
		return;
	}

	data.filledCategories[category] = true;
	std::cout << "[GameRoom] Player " << playerId << " selected category " << category << std::endl;

	// Notify non-turn clients about the score selection so they can update their UI
	Protocol::S_SCORE_SELECTED scoreSelected;
	scoreSelected.set_player_id(static_cast<int32>(playerId));
	scoreSelected.set_category(category);
	BroadcastExcept(PacketHelper::BuildPacket(PacketId::S_SCORE_SELECTED, scoreSelected), playerId);

	if (CheckGameOver())
	{
		Protocol::S_GAME_OVER gameOver;
		gameOver.set_winner_id(0); // Clients determine winner from local scoreboards
		BroadCast(PacketHelper::BuildPacket(PacketId::S_GAME_OVER, gameOver));
		return;
	}

	AdvanceTurn();
}

void GameRoom::AdvanceTurn()
{
	m_currentTurnIndex = (m_currentTurnIndex + 1) % static_cast<int>(m_turnOrder.size());
	if (m_currentTurnIndex == 0) ++m_roundNumber;
	m_rollCount = 3;
	m_isSimulating = false;

	BroadcastTurnChange();
}

void GameRoom::BroadcastTurnChange()
{
	uint32 nextPlayerId = m_turnOrder[m_currentTurnIndex];

	Protocol::S_TURN_CHANGE pkt;
	pkt.set_current_player_id(static_cast<int32>(nextPlayerId));
	pkt.set_turn_number(m_roundNumber);
	BroadCast(PacketHelper::BuildPacket(PacketId::S_TURN_CHANGE, pkt));

	std::cout << "[GameRoom] Turn -> player " << nextPlayerId << " (round " << m_roundNumber << ")" << std::endl;
}

bool GameRoom::CheckGameOver() const
{
	int totalNeeded = static_cast<int>(m_turnOrder.size()) * MAX_ROUNDS;
	int filled = 0;
	for (const auto& [pid, data] : m_playerTurnData)
	{
		for (int i = 0; i < MAX_ROUNDS; ++i)
			if (data.filledCategories[i]) ++filled;
	}
	return filled >= totalNeeded;
}
