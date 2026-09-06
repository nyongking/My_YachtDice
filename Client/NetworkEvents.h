#pragma once
#include <vector>
#include <queue>
#include "Vec3.h"
#include "Quat.h"

struct DiceTransformData
{
	Vec3 pos;
	Quat rot;
};

struct NetEvent_RollStart
{
	int rollCount;
};

struct NetEvent_DiceSnapshot
{
	DiceTransformData diceTransforms[5];
};

struct NetEvent_RollSettled
{
	int rollCount;
	int diceValues[5];
	DiceTransformData finalTransforms[5];
};

struct NetEvent_PlayerJoined
{
	int playerId;
};

struct NetEvent_CupShake {};
struct NetEvent_CupFlip {};
struct NetEvent_DiceToCup {};

struct NetEvent_DiceSelect
{
	int diceIndex;
	int slotIndex;
};
struct NetEvent_DiceReturn
{
	int diceIndex;
};

struct NetEvent_JoinComplete
{
	int playerCount;
};

struct NetEvent_TurnChange
{
	int currentPlayerId;
	int turnNumber;
};

struct NetEvent_GameOver
{
	int winnerId;
};

struct NetEvent_ScoreSelected
{
	int playerId;
	int category;
};

class NetworkEventQueue
{
public:
	void PushRollStart(const NetEvent_RollStart& e)       { m_rollStarts.push(e); }
	void PushSnapshot(const NetEvent_DiceSnapshot& e)     { m_snapshots.push(e); }
	void PushSettled(const NetEvent_RollSettled& e)       { m_settleds.push(e); }
	void PushPlayerJoined(const NetEvent_PlayerJoined& e) { m_playerJoins.push(e); }
	void PushCupShake(const NetEvent_CupShake& e)         { m_cupShakes.push(e); }
	void PushCupFlip(const NetEvent_CupFlip& e)           { m_cupFlips.push(e); }
	void PushDiceToCup(const NetEvent_DiceToCup& e)       { m_diceToCups.push(e); }
	void PushDiceSelect(const NetEvent_DiceSelect& e)     { m_diceSelects.push(e); }
	void PushDiceReturn(const NetEvent_DiceReturn& e)     { m_diceReturns.push(e); }
	void PushJoinComplete(const NetEvent_JoinComplete& e) { m_joinCompletes.push(e); }
	void PushTurnChange(const NetEvent_TurnChange& e)     { m_turnChanges.push(e); }
	void PushGameOver(const NetEvent_GameOver& e)         { m_gameOvers.push(e); }
	void PushScoreSelected(const NetEvent_ScoreSelected& e) { m_scoreSelects.push(e); }

	bool PopRollStart(NetEvent_RollStart& out)
	{
		if (m_rollStarts.empty()) return false;
		out = m_rollStarts.front(); m_rollStarts.pop(); return true;
	}
	bool PopPlayerJoined(NetEvent_PlayerJoined& out)
	{
		if (m_playerJoins.empty()) return false;
		out = m_playerJoins.front(); m_playerJoins.pop(); return true;
	}
	bool PopCupShake(NetEvent_CupShake& out)
	{
		if (m_cupShakes.empty()) return false;
		out = m_cupShakes.front(); m_cupShakes.pop(); return true;
	}
	bool PopCupFlip(NetEvent_CupFlip& out)
	{
		if (m_cupFlips.empty()) return false;
		out = m_cupFlips.front(); m_cupFlips.pop(); return true;
	}
	bool PopDiceToCup(NetEvent_DiceToCup& out)
	{
		if (m_diceToCups.empty()) return false;
		out = m_diceToCups.front(); m_diceToCups.pop(); return true;
	}
	bool PopDiceSelect(NetEvent_DiceSelect& out)
	{
		if (m_diceSelects.empty()) return false;
		out = m_diceSelects.front(); m_diceSelects.pop(); return true;
	}
	bool PopDiceReturn(NetEvent_DiceReturn& out)
	{
		if (m_diceReturns.empty()) return false;
		out = m_diceReturns.front(); m_diceReturns.pop(); return true;
	}
	bool PopSnapshot(NetEvent_DiceSnapshot& out)
	{
		if (m_snapshots.empty()) return false;
		out = m_snapshots.front(); m_snapshots.pop(); return true;
	}
	bool PopSettled(NetEvent_RollSettled& out)
	{
		if (m_settleds.empty()) return false;
		out = m_settleds.front(); m_settleds.pop(); return true;
	}
	bool PopJoinComplete(NetEvent_JoinComplete& out)
	{
		if (m_joinCompletes.empty()) return false;
		out = m_joinCompletes.front(); m_joinCompletes.pop(); return true;
	}
	bool PopTurnChange(NetEvent_TurnChange& out)
	{
		if (m_turnChanges.empty()) return false;
		out = m_turnChanges.front(); m_turnChanges.pop(); return true;
	}
	bool PopGameOver(NetEvent_GameOver& out)
	{
		if (m_gameOvers.empty()) return false;
		out = m_gameOvers.front(); m_gameOvers.pop(); return true;
	}
	bool PopScoreSelected(NetEvent_ScoreSelected& out)
	{
		if (m_scoreSelects.empty()) return false;
		out = m_scoreSelects.front(); m_scoreSelects.pop(); return true;
	}

private:
	std::queue<NetEvent_RollStart>    m_rollStarts;
	std::queue<NetEvent_DiceSnapshot> m_snapshots;
	std::queue<NetEvent_RollSettled>  m_settleds;
	std::queue<NetEvent_PlayerJoined> m_playerJoins;
	std::queue<NetEvent_CupShake>     m_cupShakes;
	std::queue<NetEvent_CupFlip>      m_cupFlips;
	std::queue<NetEvent_DiceToCup>    m_diceToCups;
	std::queue<NetEvent_DiceSelect>   m_diceSelects;
	std::queue<NetEvent_DiceReturn>   m_diceReturns;
	std::queue<NetEvent_JoinComplete> m_joinCompletes;
	std::queue<NetEvent_TurnChange>   m_turnChanges;
	std::queue<NetEvent_GameOver>       m_gameOvers;
	std::queue<NetEvent_ScoreSelected>  m_scoreSelects;
};

extern NetworkEventQueue GNetEvents;
