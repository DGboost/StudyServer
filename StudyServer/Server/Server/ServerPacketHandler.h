#pragma once

enum
{
	S_TEST = 1,
	S_EnterGame = 2,

	S_MyPlayer = 4,
	S_AddObject = 5,
	S_RemoveObject = 6,

	C_Move = 10,
	S_Move = 11,

	C_Attack = 20,
	S_Attack = 21,
	S_Die = 22,
};

struct BuffData
{
	uint64 buffId;
	float remainTime;
};

class ServerPacketHandler
{
public:
	static void HandlePacket(GameSessionRef session, BYTE* buffer, int32 len);
	// �ޱ�
	static void Handle_C_Move(GameSessionRef session, BYTE* buffer, int32 len);
	static void Handle_C_Attack(GameSessionRef session, BYTE* buffer, int32 len);
	
	// ������
	static SendBufferRef Make_S_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> buffs);
	static SendBufferRef Make_S_EnterGame();
	static SendBufferRef Make_S_MyPlayer(const Protocol::ObjectInfo& info);
	static SendBufferRef Make_S_AddObject(const Protocol::S_AddObject& pkt);
	static SendBufferRef Make_S_RemoveObject(const Protocol::S_RemoveObject& pkt);
	static SendBufferRef Make_S_Move(const Protocol::ObjectInfo& info);
	static SendBufferRef Make_S_Attack(const Protocol::ObjectInfo& attackerInfo, uint64 targetId, uint32 damage);
	static SendBufferRef Make_S_Die(uint64 objectId, uint64 killerId);


	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

		SendBufferRef sendBuffer = make_shared<SendBuffer>(packetSize);
		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;
		assert(pkt.SerializeToArray(&header[1], dataSize));
		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};