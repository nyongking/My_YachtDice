#pragma once

class GameRoom;

class GameSession : public PacketSession
{
public:
	void OnConnected() override;
	void OnDisconncted() override;
	void OnRecvPacket(BYTE* pBuffer, int32 len) override;
	void OnSend(int32 len) override;

	void SetPlayerId(uint32 id) { m_playerId = id; }
	uint32 GetPlayerId() const { return m_playerId; }

	void SetPlayerName(const std::string& name) { m_playerName = name; }
	const std::string& GetPlayerName() const { return m_playerName; }

	void SetGameRoom(std::shared_ptr<GameRoom> room) { m_gameRoom = room; }
	std::shared_ptr<GameRoom> GetGameRoom() { return m_gameRoom.lock(); }

private:
	void HandleJoin(BYTE* buffer, int32 len);
	void HandleThrowDice(BYTE* buffer, int32 len);
	void HandleSelectScore(BYTE* buffer, int32 len);
	void HandleLeave(BYTE* buffer, int32 len);
	void HandleInjectOrientation(BYTE* buffer, int32 len);
	void HandleClearInject(BYTE* buffer, int32 len);
	void HandleCupShake(BYTE* buffer, int32 len);
	void HandleCupFlip(BYTE* buffer, int32 len);
	void HandleDiceSelect(BYTE* buffer, int32 len);
	void HandleDiceReturn(BYTE* buffer, int32 len);
	void HandleDiceToCup(BYTE* buffer, int32 len);

private:
	uint32 m_playerId = 0;
	std::string m_playerName;
	std::weak_ptr<GameRoom> m_gameRoom;
};
