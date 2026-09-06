#pragma once
#include "Scene.h"
#include "ScoreboardUI.h"

namespace GameEngine { class GameObject; }
namespace
{
	constexpr int THROW_COUNT = 3;
}

struct RayHit;
class ScoreboardUI;

class GameScene : public GameEngine::Scene
{
	enum class GameState
	{
		IDLE,
		SHAKE,
		THROW,
		SETTLE,
		SELECT,
	};

public:
	void Awake() override;
	void Start() override;
	void Update(float dt) override;

private:
	void SetState(GameState state);
	bool CheckState(GameState state) const { return m_gameState == state; }

	void PoseDices();
	void NextTurn();

	void UpdateState(float dt);
	void UpdateInput();

	void UpdateDiceSelectionUI();
	void ChooseDice(int index);
	void ReturnDice(int index);
	void RepositionActiveDice();
	void FinalizeAllDice();

	void DetermineTemporaryScore();

	int  FindDiceIndex(const std::vector<class Dice*>& dices, const RayHit& hit);
	int  FindEmptySlot();

	void ProcessNetworkEvents();
	void SendThrowRequest();
	void SendSelectScore(int category);
	void SendInjectRequest(int diceIndex, int face);
	void SendClearInject(int diceIndex);
	void SendDiceSelect(int diceIndex, int slotIndex);
	void SendDiceReturn(int diceIndex);
	void SendDiceToCup();
	void SendCupShake();
	void SendCupFlip();

#ifdef _DEBUG
	void DrawDicePanel();
	void DrawNetworkPanel();
#endif

private:
	class Dice* m_allDice[5] = {};  // fixed index reference for network
	std::vector<class Dice*> m_activeDice;
	std::vector<class Dice*> m_chosedDice;

	class GameEngine::GameObject* m_hoverTL = nullptr;
	class DiceCup*  m_diceCup = nullptr;
	class ScoreboardUI m_scoreboardUI;

	GameState       m_gameState = GameState::IDLE;
	int             m_throwCount = THROW_COUNT;
	int             m_selectedIndex = -1;
	int             m_chosedIndex = -1;

	int             m_currentTurnPlayerId = 0;
	bool            m_isMyTurn = true;  // single-player default: always our turn
	int             m_playerCount = 1;
};
