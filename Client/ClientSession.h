#pragma once
#include <string>
#include <vector>

class ClientSession : public PacketSession
{
public:
	void OnConnected() override;
	void OnDisconncted() override;
	void OnRecvPacket(BYTE* pBuffer, int32 len) override;
	void OnSend(int32 len) override;

	int32 GetPlayerId() const { return m_playerId; }
	bool IsJoined() const { return m_connected; }

	const std::vector<std::string>& GetLogs() const { return m_logs; }

private:
	void HandleJoinResult(BYTE* buffer, int32 len);
	void HandlePlayerJoined(BYTE* buffer, int32 len);
	void HandlePlayerLeft(BYTE* buffer, int32 len);
	void HandleRollStart(BYTE* buffer, int32 len);
	void HandleDiceSnapshot(BYTE* buffer, int32 len);
	void HandleRollSettled(BYTE* buffer, int32 len);
	void HandleTurnChange(BYTE* buffer, int32 len);
	void HandleGameOver(BYTE* buffer, int32 len);
	void HandleCupShake(BYTE* buffer, int32 len);
	void HandleCupFlip(BYTE* buffer, int32 len);
	void HandleDiceSelect(BYTE* buffer, int32 len);
	void HandleDiceReturn(BYTE* buffer, int32 len);
	void HandleDiceToCup(BYTE* buffer, int32 len);
	void HandleScoreSelected(BYTE* buffer, int32 len);
	void AddLog(const std::string& msg);

private:
	int32 m_playerId = 0;
	bool m_connected = false;
	std::vector<std::string> m_logs;
};

extern std::weak_ptr<ClientSession> GClientSession;
