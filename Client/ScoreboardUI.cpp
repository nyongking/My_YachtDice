#include "ClientPch.h"
#include "ScoreboardUI.h"
#include "UIManager.h"
#include "UITextComponent.h"
#include "UIImageComponent.h"
#include "UICanvas.h"

#include "AudioManager.h"

using namespace GameEngine;
namespace
{
	constexpr const char* SCORE_NAMES[ScoreType::END] =
	{
		"InputOne_P",
		"InputTwo_P",
		"InputThree_P",
		"InputFour_P",
		"InputFive_P",
		"InputSix_P",
		"InputScore_P",
		"InputBonus_P",
		"InputChance_P",
		"InputFoK_P",
		"InputFH_P",
		"InputSS_P",
		"InputLS_P",
		"InputYcht_P",
		"InputTotal_P",
	};

	constexpr const char* ZONE_NAMES[12] =
	{
		"ZoneOne",
		"ZoneTwo",
		"ZoneThree",
		"ZoneFour",
		"ZoneFive",
		"ZoneSix",
		"ZoneChance",
		"ZoneFok",
		"ZoneFH",
		"ZoneSS",
		"ZoneLS",
		"ZoneYcht",
	};

	constexpr const wchar_t* SCORE_TEXT[5] =
	{
		L"Four of a Kind !",
		L"Full House !",
		L"Small Straight !",
		L"Large Straight !",
		L"Yacht !"
	};

	constexpr float ZONE_X_START = -66.f;
	constexpr float ZONE_X_SPACE = 88.f;
	constexpr float4 ZONE_COLOR = float4(228.f / 255.f, 99.f / 255.f, 20.f / 255.f, 200.f / 255.f);
	constexpr float4 ZONE_HIDE = float4(228.f / 255.f, 99.f / 255.f, 20.f / 255.f, 0.f);
	constexpr float4 TEMP_TEXT_COLOR = float4(60.f / 255.f, 45.f / 255.f, 45.f / 255.f, 1.f);
}

void ScoreboardUI::SetCurrentPlayer(int playerId, int turnNumber)
{
	m_nowPlayer = playerId;

	for (int i = 0; i < 12; ++i)
	{
		if (m_zones[i])
			m_zones[i]->SetPositionX(ZONE_X_START + ZONE_X_SPACE * (m_nowPlayer - 1));
	}

	if (m_turn != turnNumber)
	{
		m_turn = turnNumber;
		if (m_inputTurn && m_turn <= TOTAL_TURN)
			m_inputTurn->SetText(std::to_wstring(m_turn) + L"/" + std::to_wstring(TOTAL_TURN));
	}
}

void ScoreboardUI::SetPlayerSize(int playerSize)
{
	if (playerSize <= MAX_PLAYER) m_playerSize = playerSize;
	ResetGame();
}

void ScoreboardUI::ResetGame()
{
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		for (int j = ScoreType::ONE; j <= ScoreType::SIX; ++j)
		{
			auto* text = m_playerScoreTexts[i][j];
			if (text)
				text->SetText(L"");
		}

		if (m_playerScoreTexts[i][SCORE])
		{
			if (i < m_playerSize)
				m_playerScoreTexts[i][SCORE]->SetNumber(0);
			else
				m_playerScoreTexts[i][SCORE]->SetText(L"");
		}
		if (m_playerScoreTexts[i][BONUS])
		{
			if (i < m_playerSize)
				m_playerScoreTexts[i][BONUS]->SetText(std::to_wstring(0) + L"/" + std::to_wstring(Score::BONUS_TARGET));
			else
				m_playerScoreTexts[i][BONUS]->SetText(L"");

		}
		for (int j = ScoreType::CHANCE; j <= ScoreType::YACHT; ++j)
		{
			auto* text = m_playerScoreTexts[i][j];
			if (text)
				text->SetText(L"");
		}
		if (m_playerScoreTexts[i][TOTAL])
		{
			if (i < m_playerSize)
				m_playerScoreTexts[i][TOTAL]->SetNumber(0);
			else
				m_playerScoreTexts[i][TOTAL]->SetText(L"");

		}

		for (int j = ScoreType::ONE; j < ScoreType::END; ++j)
		{
			m_playerScores[i][j] = -1;
		}
		m_playerScores[i][ScoreType::BONUS] = 0;
		m_playerScores[i][ScoreType::SCORE] = 0;
		m_playerScores[i][ScoreType::TOTAL] = 0;

		m_turn = 1;
		if (m_inputTurn)
		{
			m_inputTurn->SetText(std::to_wstring(m_turn) + L"/" + std::to_wstring(TOTAL_TURN));
		}
	}
}

void ScoreboardUI::CalculateScores(std::vector<int> dices)
{
	const int diceSize = static_cast<int>(dices.size());

	m_tempScores.assign(ScoreType::END, 0);

	std::vector<int> diceCount(6, 0);

	for (int i = 0; i < diceSize; ++i)
	{
		++diceCount[dices[i] - 1];
	}

	for (int i = ScoreType::ONE; i <= ScoreType::SIX; ++i)
	{
		m_tempScores[i] = diceCount[i] * (i + 1);
		m_tempScores[ScoreType::CHANCE] += m_tempScores[i];
	}

	bool isThree = false;
	bool isTwo = false;
	int isNumber = 0;
	for (int i = 0; i < 6; ++i)
	{
		if (0 < diceCount[i])
		{
			++isNumber;
		}
		else
		{
			isNumber = 0;
			continue;
		}

		if (2 == diceCount[i])
			isTwo = true;
		else if (3 == diceCount[i])
			isThree = true;

		if (5 == diceCount[i])
		{
			m_tempScores[ScoreType::YACHT] = Score::YACHT_SCORE;
		}
		if (4 <= diceCount[i])
		{
			m_tempScores[ScoreType::FOUR_OF_KIND] = m_tempScores[ScoreType::CHANCE];
		}


		if (4 == isNumber)
		{
			m_tempScores[ScoreType::SMALL_STRAIGHT] = Score::SMALL_STRAIGHT_SCORE;
		}
		if (5 == isNumber)
		{
			m_tempScores[ScoreType::LARGE_STRAIGHT] = Score::LARGE_STRAIGHT_SCORE;
		}
	}

	if (isTwo && isThree)
	{
		m_tempScores[ScoreType::FULL_HOUSE] = Score::FULL_HOUSE_SCORE;
	}

}

void ScoreboardUI::ShowTemporaryScores()
{
	for (int i = ScoreType::ONE; i <= ScoreType::SIX; ++i)
	{
		if (-1 == m_playerScores[m_nowPlayer - 1][i])
		{
			if (m_playerScoreTexts[m_nowPlayer - 1][i])
			{
				m_playerScoreTexts[m_nowPlayer - 1][i]->SetColor(TEMP_TEXT_COLOR);
				m_playerScoreTexts[m_nowPlayer - 1][i]->SetNumber(m_tempScores[i]);
			}

			if (m_zones[i])
			{
				m_zones[i]->SetColor(ZONE_COLOR);
				m_zones[i]->SetInteractive(true);
			}
		}
	}

	bool isScore = false;
	for (int i = ScoreType::CHANCE; i <= ScoreType::YACHT; ++i)
	{
		if (-1 == m_playerScores[m_nowPlayer - 1][i])
		{
			if (m_playerScoreTexts[m_nowPlayer - 1][i])
			{
				m_playerScoreTexts[m_nowPlayer - 1][i]->SetColor(TEMP_TEXT_COLOR);
				m_playerScoreTexts[m_nowPlayer - 1][i]->SetNumber(m_tempScores[i]);
			}

			if (m_zones[i - 2])
			{
				m_zones[i - 2]->SetColor(ZONE_COLOR);
				m_zones[i - 2]->SetInteractive(true);
			}

			if (0 != m_tempScores[i] && ScoreType::CHANCE != i)
			{
				if (m_scoreText)
				{
					m_scoreText->SetText(SCORE_TEXT[i - 9]);
					isScore = true;
				}
			}
		}
	}

	if (isScore)
		GAudioManager->PlaySFX("Score", 1.f);
	else
		GAudioManager->PlaySFX("NonScore", 0.7f);



	m_isCanSelect = true;
}

void ScoreboardUI::HideTemporaryScores()
{
	for (int i = ScoreType::ONE; i <= ScoreType::SIX; ++i)
	{
		if (-1 == m_playerScores[m_nowPlayer - 1][i])
		{
			if (m_playerScoreTexts[m_nowPlayer - 1][i])
				m_playerScoreTexts[m_nowPlayer - 1][i]->SetText(L"");

			if (m_zones[i])
			{
				m_zones[i]->SetColor(ZONE_HIDE);
				m_zones[i]->SetInteractive(false);
			}
		}
	}

	for (int i = ScoreType::CHANCE; i <= ScoreType::YACHT; ++i)
	{
		if (-1 == m_playerScores[m_nowPlayer - 1][i])
		{
			if (m_playerScoreTexts[m_nowPlayer - 1][i])
				m_playerScoreTexts[m_nowPlayer - 1][i]->SetText(L"");

			if (m_zones[i - 2])
			{
				m_zones[i - 2]->SetColor(ZONE_HIDE);
				m_zones[i - 2]->SetInteractive(false);
			}
		}
	}

	if (m_scoreText)
		m_scoreText->SetText(L"");

	m_isCanSelect = false;
	m_isSelectScore = false;
}

void ScoreboardUI::Initialize()
{
	{
		auto* canvas = UIManager::GetInstance().FindCanvas("ScorePanel");

		if (canvas)
		{
			m_inputTurn = static_cast<UITextComponent*>(canvas->FindComponent("InputTurn"));

			m_playerScoreTexts.resize(MAX_PLAYER);
			for (int i = 1; i <= MAX_PLAYER; ++i)
			{
				for (int j = ScoreType::ONE; j < ScoreType::END; ++j)
				{
					std::string name = std::string(SCORE_NAMES[j]) + std::to_string(i);
					auto* text = static_cast<UITextComponent*>(canvas->FindComponent(name));

					if (nullptr != text)
						m_playerScoreTexts[i - 1].push_back(text);
				}
			}

			for (int i = 0; i < 12; ++i)
			{
				auto* image = static_cast<UIImageComponent*>(canvas->FindComponent(ZONE_NAMES[i]));
				if (nullptr != image)
				{
					image->SetColor(ZONE_HIDE);
					image->SetOnRelease([this, i](UIComponent* comp) { DetermineScore(i); });
					m_zones.push_back(image);
				}
			}
		}
	}

	{
		auto* canvas = UIManager::GetInstance().FindCanvas("ScoreText");

		if (canvas)
		{
			m_scoreText = static_cast<UITextComponent*>(canvas->FindComponent("ScoreText"));
		}
	}



	m_playerScores.resize(MAX_PLAYER);
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		m_playerScores[i].resize(ScoreType::END);
	}

	m_tempScores.resize(ScoreType::END);

	ResetGame();
}

void ScoreboardUI::RecordOpponentScore(int index)
{
	// Silent version of DetermineScore for the current turn player (m_nowPlayer)
	// m_tempScores must already be populated via CalculateScores before calling this
	const int pi = m_nowPlayer - 1;

	if (0 <= index && 6 > index)
	{
		if (m_playerScores[pi][index] != -1)
			return; // already recorded

		m_playerScores[pi][index] = m_tempScores[index];

		if (m_playerScoreTexts[pi][index])
		{
			m_playerScoreTexts[pi][index]->SetColor(float4(0.f, 0.f, 0.f, 1.f));
			m_playerScoreTexts[pi][index]->SetNumber(m_playerScores[pi][index]);
		}

		int prevScore = m_playerScores[pi][ScoreType::SCORE];
		m_playerScores[pi][ScoreType::SCORE] += m_playerScores[pi][index];

		if (m_playerScoreTexts[pi][ScoreType::SCORE])
			m_playerScoreTexts[pi][ScoreType::SCORE]->SetNumber(m_playerScores[pi][ScoreType::SCORE]);

		if (BONUS_TARGET > prevScore)
		{
			if (BONUS_TARGET <= m_playerScores[pi][ScoreType::SCORE])
			{
				m_playerScores[pi][ScoreType::BONUS] = BONUS_SCORE;
				m_playerScoreTexts[pi][ScoreType::BONUS]->SetNumber(BONUS_SCORE);
				m_playerScores[pi][ScoreType::TOTAL] += BONUS_SCORE;
			}
			else
			{
				m_playerScoreTexts[pi][ScoreType::BONUS]->SetText(
					std::to_wstring(m_playerScores[pi][ScoreType::SCORE]) + L"/" + std::to_wstring(Score::BONUS_TARGET));
			}
		}

		m_playerScores[pi][ScoreType::TOTAL] += m_tempScores[index];
	}
	else if (6 <= index && 12 > index)
	{
		if (m_playerScores[pi][index + 2] != -1)
			return;

		m_playerScores[pi][index + 2] = m_tempScores[index + 2];

		if (m_playerScoreTexts[pi][index + 2])
		{
			m_playerScoreTexts[pi][index + 2]->SetColor(float4(0.f, 0.f, 0.f, 1.f));
			m_playerScoreTexts[pi][index + 2]->SetNumber(m_playerScores[pi][index + 2]);
		}

		m_playerScores[pi][ScoreType::TOTAL] += m_tempScores[index + 2];
	}

	if (m_playerScoreTexts[pi][ScoreType::TOTAL])
		m_playerScoreTexts[pi][ScoreType::TOTAL]->SetNumber(m_playerScores[pi][ScoreType::TOTAL]);
}

void ScoreboardUI::DetermineScore(int index)
{
	if (!m_isCanSelect)
		return;

	if (0 <= index && 6 > index)
	{
		m_playerScores[m_nowPlayer - 1][index] = m_tempScores[index];

		if (m_playerScoreTexts[m_nowPlayer - 1][index])
		{
			m_playerScoreTexts[m_nowPlayer - 1][index]->SetColor(float4(0.f, 0.f, 0.f, 1.f));
			m_playerScoreTexts[m_nowPlayer - 1][index]->SetNumber(m_playerScores[m_nowPlayer - 1][index]);
		}

		if (m_zones[index])
		{
			m_zones[index]->SetColor(ZONE_HIDE);
			m_zones[index]->SetInteractive(false);
		}

		int prevScore = m_playerScores[m_nowPlayer - 1][ScoreType::SCORE];
		m_playerScores[m_nowPlayer - 1][ScoreType::SCORE] += m_playerScores[m_nowPlayer - 1][index];

		if (m_playerScoreTexts[m_nowPlayer - 1][ScoreType::SCORE])
		{
			m_playerScoreTexts[m_nowPlayer - 1][ScoreType::SCORE]->SetNumber(m_playerScores[m_nowPlayer - 1][ScoreType::SCORE]);
		}

		if (BONUS_TARGET > prevScore)
		{
			if (BONUS_TARGET <= m_playerScores[m_nowPlayer - 1][ScoreType::SCORE])
			{
				m_playerScores[m_nowPlayer - 1][ScoreType::BONUS] = BONUS_SCORE;
				m_playerScoreTexts[m_nowPlayer - 1][ScoreType::BONUS]->SetNumber(BONUS_SCORE);

				m_playerScores[m_nowPlayer - 1][ScoreType::TOTAL] += BONUS_SCORE;
			}
			else
			{
				m_playerScoreTexts[m_nowPlayer - 1][ScoreType::BONUS]->SetText(std::to_wstring(m_playerScores[m_nowPlayer - 1][ScoreType::SCORE]) + L"/" + std::to_wstring(Score::BONUS_TARGET));
			}
		}
		m_playerScores[m_nowPlayer - 1][ScoreType::TOTAL] += m_tempScores[index];

	}
	else if (6 <= index && 12 > index)
	{
		m_playerScores[m_nowPlayer - 1][index + 2] = m_tempScores[index + 2];

		if (m_playerScoreTexts[m_nowPlayer - 1][index + 2])
		{
			m_playerScoreTexts[m_nowPlayer - 1][index + 2]->SetColor(float4(0.f, 0.f, 0.f, 1.f));
			m_playerScoreTexts[m_nowPlayer - 1][index + 2]->SetNumber(m_playerScores[m_nowPlayer - 1][index + 2]);
		}

		if (m_zones[index])
		{
			m_zones[index]->SetColor(ZONE_HIDE);
			m_zones[index]->SetInteractive(false);
		}

		m_playerScores[m_nowPlayer - 1][ScoreType::TOTAL] += m_tempScores[index + 2];
	}

	m_playerScoreTexts[m_nowPlayer - 1][ScoreType::TOTAL]->SetNumber(m_playerScores[m_nowPlayer - 1][ScoreType::TOTAL]);

	m_selectedCategory = index;
	m_isSelectScore = true;
}
