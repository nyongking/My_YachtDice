#include "ServerPch.h"
#include "GameSession.h"
#include "GameRoom.h"

void GameSession::OnConnected()
{
	std::cout << "[GameSession] Client connected." << std::endl;
}

void GameSession::OnDisconncted()
{
	std::cout << "[GameSession] Client disconnected." << std::endl;

	if (auto room = m_gameRoom.lock())
	{
		room->Leave(std::static_pointer_cast<GameSession>(shared_from_this()));
	}
}

void GameSession::OnRecvPacket(BYTE* pBuffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(pBuffer);

	switch (static_cast<PacketId>(header->id))
	{
	case PacketId::C_JOIN:
		HandleJoin(pBuffer, len);
		break;
	case PacketId::C_THROW_DICE:
		HandleThrowDice(pBuffer, len);
		break;
	case PacketId::C_SELECT_SCORE:
		HandleSelectScore(pBuffer, len);
		break;
	case PacketId::C_LEAVE:
		HandleLeave(pBuffer, len);
		break;
	case PacketId::C_INJECT_ORIENTATION:
		HandleInjectOrientation(pBuffer, len);
		break;
	case PacketId::C_CLEAR_INJECT:
		HandleClearInject(pBuffer, len);
		break;
	case PacketId::C_CUP_SHAKE:
		HandleCupShake(pBuffer, len);
		break;
	case PacketId::C_CUP_FLIP:
		HandleCupFlip(pBuffer, len);
		break;
	case PacketId::C_DICE_SELECT:
		HandleDiceSelect(pBuffer, len);
		break;
	case PacketId::C_DICE_RETURN:
		HandleDiceReturn(pBuffer, len);
		break;
	case PacketId::C_DICE_TO_CUP:
		HandleDiceToCup(pBuffer, len);
		break;
	default:
		std::cout << "[GameSession] Unknown packet id: " << header->id << std::endl;
		break;
	}
}

void GameSession::OnSend(int32 len)
{
}

void GameSession::HandleJoin(BYTE* buffer, int32 len)
{
	Protocol::C_JOIN pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	m_playerName = pkt.player_name();
	std::cout << "[GameSession] Join request from: " << m_playerName << std::endl;

	if (GGameRoom)
	{
		GGameRoom->Enter(std::static_pointer_cast<GameSession>(shared_from_this()));
	}
}

void GameSession::HandleThrowDice(BYTE* buffer, int32 len)
{
	Protocol::C_THROW_DICE pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	std::cout << "[GameSession] Player " << m_playerId << " throw dice. Held:";
	for (int i = 0; i < pkt.held_indices_size(); ++i)
		std::cout << " " << pkt.held_indices(i);
	std::cout << std::endl;

	if (auto room = m_gameRoom.lock())
	{
		std::vector<int> heldIndices(pkt.held_indices().begin(), pkt.held_indices().end());
		room->HandleThrowRequest(m_playerId, heldIndices);
	}
}

void GameSession::HandleSelectScore(BYTE* buffer, int32 len)
{
	Protocol::C_SELECT_SCORE pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	std::cout << "[GameSession] Player " << m_playerId << " selected category: " << pkt.category() << std::endl;

	if (auto room = m_gameRoom.lock())
	{
		room->HandleSelectScore(m_playerId, pkt.category());
	}
}

void GameSession::HandleInjectOrientation(BYTE* buffer, int32 len)
{
	Protocol::C_INJECT_ORIENTATION pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	if (auto room = m_gameRoom.lock())
		room->HandleInjectRequest(m_playerId, pkt.dice_index(), pkt.target_face());
}

void GameSession::HandleClearInject(BYTE* buffer, int32 len)
{
	Protocol::C_CLEAR_INJECT pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	if (auto room = m_gameRoom.lock())
		room->HandleClearInject(m_playerId, pkt.dice_index());
}

void GameSession::HandleCupShake(BYTE* buffer, int32 len)
{
	if (auto room = m_gameRoom.lock())
		room->HandleCupShake(m_playerId);
}

void GameSession::HandleCupFlip(BYTE* buffer, int32 len)
{
	if (auto room = m_gameRoom.lock())
		room->HandleCupFlip(m_playerId);
}

void GameSession::HandleDiceSelect(BYTE* buffer, int32 len)
{
	Protocol::C_DICE_SELECT pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	if (auto room = m_gameRoom.lock())
		room->HandleDiceSelect(m_playerId, pkt.dice_index(), pkt.slot_index());
}

void GameSession::HandleDiceReturn(BYTE* buffer, int32 len)
{
	Protocol::C_DICE_RETURN pkt;
	if (!PacketHelper::ParsePacket(buffer, len, pkt))
		return;

	if (auto room = m_gameRoom.lock())
		room->HandleDiceReturn(m_playerId, pkt.dice_index());
}

void GameSession::HandleDiceToCup(BYTE* buffer, int32 len)
{
	if (auto room = m_gameRoom.lock())
		room->HandleDiceToCup(m_playerId);
}

void GameSession::HandleLeave(BYTE* buffer, int32 len)
{
	std::cout << "[GameSession] Player " << m_playerId << " requested leave." << std::endl;

	if (auto room = m_gameRoom.lock())
	{
		room->Leave(std::static_pointer_cast<GameSession>(shared_from_this()));
	}

	Disconnect(L"Player requested leave");
}
