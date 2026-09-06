#include "ServerPch.h"
#include "GameSession.h"
#include "GameRoom.h"
#include "ThreadManager.h"
#include "SocketUtils.h"
#include <chrono>

int main()
{
	InitCore();
	InitServerCore();
	SocketUtils::Init();

	GGameRoom = std::make_shared<GameRoom>();

	RefServerService service = std::make_shared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		std::make_shared<IocpCore>(),
		[]() -> RefSession { return std::make_shared<GameSession>(); },
		10
	);

	ASSERT_TRIG_CRASH(service->Start());

	std::cout << "[Server] Listening on port 7777..." << std::endl;

	// Game tick thread (physics simulation)
	GThreadManager->Launch([]()
	{
		auto prev = std::chrono::high_resolution_clock::now();
		while (true)
		{
			auto now = std::chrono::high_resolution_clock::now();
			float dt = std::chrono::duration<float>(now - prev).count();
			prev = now;

			if (GGameRoom)
				GGameRoom->Tick(dt);

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	});

	// IOCP worker threads
	const int32 threadCount = std::thread::hardware_concurrency();
	for (int32 i = 0; i < threadCount; i++)
	{
		GThreadManager->Launch([&service]()
		{
			while (true)
			{
				service->GetIocpCore()->Dispatch();
			}
		});
	}

	GThreadManager->Join();

	GGameRoom = nullptr;
	google::protobuf::ShutdownProtobufLibrary();

	SocketUtils::Clear();
	ReleaseServerCore();
	ReleaseCore();

	return 0;
}
