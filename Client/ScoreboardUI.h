#pragma once
#include "ScoreType.h"

namespace GameEngine { class UITextComponent; class UICanvas; class UIImageComponent; }
namespace
{
	constexpr int TOTAL_TURN = 12;
	constexpr int MAX_PLAYER = 2;
}

class ScoreboardUI
{
public:

public:
	void SetPlayerSize(int playerSize);
	void SetCurrentPlayer(int playerId, int turnNumber);
	void RecordOpponentScore(int zoneIndex);
	void ResetGame();

	bool IsSelected() const { return m_isSelectScore; }
	int  GetSelectedCategory() const { return m_selectedCategory; }
	void CalculateScores(std::vector<int> dices);

	void ShowTemporaryScores();
	void HideTemporaryScores();

public:
	void Initialize();

private:
	void DetermineScore(int index);

private:
	int								m_turn = 1;
	int								m_nowPlayer = 1;
	int								m_selectedCategory = -1;
	int								m_playerSize = 1;
	bool							m_isSelectScore = false;
	bool							m_isCanSelect = false;
	std::vector<int>				m_tempScores;
	std::vector<std::vector<int>>	m_playerScores;

	GameEngine::UITextComponent*				m_inputTurn = nullptr;
	std::vector<std::vector<GameEngine::UITextComponent*>>	m_playerScoreTexts;
	std::vector<GameEngine::UIImageComponent*>	m_zones;

	GameEngine::UITextComponent* m_scoreText = nullptr;
};

