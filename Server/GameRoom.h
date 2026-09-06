#pragma once
#include "DiceSimulation.h"

class GameSession;

class GameRoom : public std::enable_shared_from_this<GameRoom>
{
public:
	void Enter(std::shared_ptr<GameSession> session);
	void Leave(std::shared_ptr<GameSession> session);

	void Tick(float dt);
	void HandleThrowRequest(uint32 playerId, const std::vector<int>& heldIndices);
	void HandleSelectScore(uint32 playerId, int category);
	void HandleInjectRequest(uint32 playerId, int diceIndex, int face);
	void HandleClearInject(uint32 playerId, int diceIndex);
	void HandleCupShake(uint32 playerId);
	void HandleCupFlip(uint32 playerId);
	void HandleDiceSelect(uint32 playerId, int diceIndex, int slotIndex);
	void HandleDiceReturn(uint32 playerId, int diceIndex);
	void HandleDiceToCup(uint32 playerId);

	void BroadCast(RefSendBuffer sendBuffer);
	void BroadcastExcept(RefSendBuffer sendBuffer, uint32 excludePlayerId);

private:
	void BroadcastSnapshot();
	void OnSimulationSettled();
	void FillDiceTransform(Protocol::DiceTransform* proto, const Vec3& pos, const Quat& rot);

	// Turn management (called with lock held)
	void AdvanceTurn();
	void BroadcastTurnChange();
	bool CheckGameOver() const;

private:
	struct PlayerTurnData
	{
		bool filledCategories[12] = {};
	};

	static constexpr int MAX_ROUNDS = 12;

	USE_LOCK;
	uint32 m_nextPlayerId = 1;
	exmap<uint32, std::shared_ptr<GameSession>> m_sessions;

	DiceSimulation m_diceSimulation;
	bool m_initialized   = false;
	bool m_isSimulating  = false;
	int  m_rollCount     = 3;

	// Turn order
	std::vector<uint32>              m_turnOrder;
	exmap<uint32, PlayerTurnData>    m_playerTurnData;
	int                              m_currentTurnIndex = 0;
	int                              m_roundNumber = 1;
};

// 하나의 GGameRoom만 사용중...
extern std::shared_ptr<GameRoom> GGameRoom;
