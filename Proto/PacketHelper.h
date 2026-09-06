#pragma once

#include "Session.h"
#include "SendBuffer.h"

struct PacketHeader;

class PacketHelper
{
public:
	template<typename T>
	static bool ParsePacket(BYTE* buffer, int32 len, T& pkt)
	{
		return pkt.ParseFromArray(
			buffer + sizeof(PacketHeader),
			len - sizeof(PacketHeader));
	}

	template<typename T>
	static RefSendBuffer BuildPacket(PacketId id, const T& pkt)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = sizeof(PacketHeader) + dataSize;

		RefSendBuffer sendBuffer = GSendBufferManager->Open(packetSize);

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = static_cast<uint16>(id);

		pkt.SerializeToArray(&header[1], dataSize);
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};
