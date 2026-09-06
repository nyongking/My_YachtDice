#include "ClientPch.h"
#include "ClientSession.h"
#include "NetworkEvents.h"

#include "MainThreadQueue.h"

std::weak_ptr<ClientSession> GClientSession;

void ClientSession::OnConnected()
{
	AddLog("[Network] Connected to server.");

	Protocol::C_JOIN joinPkt;
	joinPkt.set_player_name("Player");
	Send(PacketHelper::BuildPacket(PacketId::C_JOIN, joinPkt));

	AddLog("[Network] C_JOIN sent.");
}

void ClientSession::OnDisconncted()
{
	AddLog("[Network] Disconnected from server.");
	m_connected = false;
}

void ClientSession::OnRecvPacket(BYTE* pBuffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(pBuffer);

	switch (static_cast<PacketId>(header->id))
	{
	case PacketId::S_JOIN_RESULT:
		HandleJoinResult(pBuffer, len);
		break;
	case PacketId::S_ROLL_START:
		HandleRollStart(pBuffer, len);
		break;
	case PacketId::S_DICE_SNAPSHOT:
		HandleDiceSnapshot(pBuffer, len);
		break;
	case PacketId::S_ROLL_SETTLED:
		HandleRollSettled(pBuffer, len);
		break;
	case PacketId::S_PLAYER_JOINED:
		HandlePlayerJoined(pBuffer, len);
		break;
	case PacketId::S_PLAYER_LEFT:
		HandlePlayerLeft(pBuffer, len);
		break;
	case PacketId::S_TURN_CHANGE:
		HandleTurnChange(pBuffer, len);
		break;
	case PacketId::S_GAME_OVER:
		HandleGameOver(pBuffer, len);
		break;
	case PacketId::S_CUP_SHAKE:
		HandleCupShake(pBuffer, len);
		break;
	case PacketId::S_CUP_FLIP:
		HandleCupFlip(pBuffer, len);
		break;
	case PacketId::S_DICE_SELECT:
		HandleDiceSelect(pBuffer, len);
		break;
	case PacketId::S_DICE_RETURN:
		HandleDiceReturn(pBuffer, len);
		break;
	case PacketId::S_DICE_TO_CUP:
		HandleDiceToCup(pBuffer, len);
		break;
	case PacketId::S_SCORE_SELECTED:
		HandleScoreSelected(pBuffer, len);
		break;
	default:
		break;
	}
}

void ClientSession::OnSend(int32 len)
{
}

void ClientSession::HandleJoinResult(BYTE* buffer, int32 len)
{
	Protocol::S_JOIN_RESULT pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	if (pkt.success())
	{
		m_playerId = pkt.player_id();
		m_connected = true;
		AddLog("[Network] Joined. Player ID: " + std::to_string(m_playerId));

		NetEvent_JoinComplete ev;
		ev.playerCount = pkt.player_count();
		GMainQueue->PushJob([ev]() { GNetEvents.PushJoinComplete(ev); });
	}
	else
	{
		AddLog("[Network] Join rejected.");
	}
}

void ClientSession::HandlePlayerJoined(BYTE* buffer, int32 len)
{
	Protocol::S_PLAYER_JOINED pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_PlayerJoined ev;
	ev.playerId = pkt.player_id();
	GMainQueue->PushJob([ev]() { GNetEvents.PushPlayerJoined(ev); });

	AddLog("[Network] Player " + std::to_string(pkt.player_id()) + " (" + pkt.player_name() + ") joined.");
}

void ClientSession::HandlePlayerLeft(BYTE* buffer, int32 len)
{
	Protocol::S_PLAYER_LEFT pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	AddLog("[Network] Player " + std::to_string(pkt.player_id()) + " left.");
}

void ClientSession::HandleRollStart(BYTE* buffer, int32 len)
{
	Protocol::S_ROLL_START pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_RollStart ev;
	ev.rollCount = pkt.roll_count();
	GMainQueue->PushJob([ev]() { GNetEvents.PushRollStart(ev); });

	AddLog("[Network] S_ROLL_START. Rolls left: " + std::to_string(ev.rollCount));
}

void ClientSession::HandleDiceSnapshot(BYTE* buffer, int32 len)
{
	Protocol::S_DICE_SNAPSHOT pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_DiceSnapshot ev;
	for (int i = 0; i < pkt.dice_transforms_size() && i < 5; ++i)
	{
		const auto& dt = pkt.dice_transforms(i);
		ev.diceTransforms[i].pos = { dt.px(), dt.py(), dt.pz() };
		ev.diceTransforms[i].rot = { dt.qx(), dt.qy(), dt.qz(), dt.qw() };
	}

	GMainQueue->PushJob([ev]() { GNetEvents.PushSnapshot(ev); });
}

void ClientSession::HandleRollSettled(BYTE* buffer, int32 len)
{
	Protocol::S_ROLL_SETTLED pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_RollSettled ev;
	ev.rollCount = pkt.roll_count();
	for (int i = 0; i < pkt.dice_values_size() && i < 5; ++i)
		ev.diceValues[i] = pkt.dice_values(i);
	for (int i = 0; i < pkt.final_transforms_size() && i < 5; ++i)
	{
		const auto& dt = pkt.final_transforms(i);
		ev.finalTransforms[i].pos = { dt.px(), dt.py(), dt.pz() };
		ev.finalTransforms[i].rot = { dt.qx(), dt.qy(), dt.qz(), dt.qw() };
	}

	std::string faces;
	for (int i = 0; i < 5; ++i) faces += " " + std::to_string(ev.diceValues[i]);
	AddLog("[Network] S_ROLL_SETTLED." + faces);

	GMainQueue->PushJob([ev]() { GNetEvents.PushSettled(ev); });
}

void ClientSession::HandleTurnChange(BYTE* buffer, int32 len)
{
	Protocol::S_TURN_CHANGE pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_TurnChange ev;
	ev.currentPlayerId = pkt.current_player_id();
	ev.turnNumber = pkt.turn_number();
	GMainQueue->PushJob([ev]() { GNetEvents.PushTurnChange(ev); });

	AddLog("[Network] S_TURN_CHANGE. Current player: " + std::to_string(ev.currentPlayerId));
}

void ClientSession::HandleGameOver(BYTE* buffer, int32 len)
{
	Protocol::S_GAME_OVER pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_GameOver ev;
	ev.winnerId = pkt.winner_id();
	GMainQueue->PushJob([ev]() { GNetEvents.PushGameOver(ev); });

	AddLog("[Network] S_GAME_OVER.");
}

void ClientSession::HandleCupShake(BYTE* buffer, int32 len)
{
	GMainQueue->PushJob([]() { GNetEvents.PushCupShake({}); });
}

void ClientSession::HandleCupFlip(BYTE* buffer, int32 len)
{
	GMainQueue->PushJob([]() { GNetEvents.PushCupFlip({}); });
}

void ClientSession::HandleDiceSelect(BYTE* buffer, int32 len)
{
	Protocol::S_DICE_SELECT pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_DiceSelect ev;
	ev.diceIndex = pkt.dice_index();
	ev.slotIndex = pkt.slot_index();
	GMainQueue->PushJob([ev]() { GNetEvents.PushDiceSelect(ev); });
}

void ClientSession::HandleDiceReturn(BYTE* buffer, int32 len)
{
	Protocol::S_DICE_RETURN pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_DiceReturn ev;
	ev.diceIndex = pkt.dice_index();
	GMainQueue->PushJob([ev]() { GNetEvents.PushDiceReturn(ev); });
}

void ClientSession::HandleDiceToCup(BYTE* buffer, int32 len)
{
	GMainQueue->PushJob([]() { GNetEvents.PushDiceToCup({}); });
}

void ClientSession::HandleScoreSelected(BYTE* buffer, int32 len)
{
	Protocol::S_SCORE_SELECTED pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	NetEvent_ScoreSelected ev;
	ev.playerId = pkt.player_id();
	ev.category = pkt.category();
	GMainQueue->PushJob([ev]() { GNetEvents.PushScoreSelected(ev); });
}

void ClientSession::AddLog(const std::string& msg)
{
	m_logs.push_back(msg);
}
