//CLIENT SIDE
#include <iostream>
#include <Net.hpp>

enum class CustomMsgType : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	BroadCast,
	ServerMessage
};

class MyClient : public net::ClientInterface<CustomMsgType>
{
public:
	void PingServer()
	{
		net::Message<CustomMsgType> message;
		message.header.id = CustomMsgType::ServerPing;

		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		message << now;
		Send(message);
	}

	void BroadCast()
	{
		net::Message<CustomMsgType> message;
		message.header.id = CustomMsgType::BroadCast;
		Send(message);
	}
};

int main()
{
	MyClient c;
	c.Connect("127.0.0.1", 60000);

	bool key[3] = { false, false, false };
	bool oldKey[3] = { false, false, false };

	auto isQuit = false;
	while (!isQuit)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[0] && !oldKey[0])
			c.PingServer();

		if (key[1] && !oldKey[1])
			c.BroadCast();

		if (key[2] && !oldKey[2])
			isQuit = true;
		for (auto i = 0; i < 3; i++) oldKey[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.IncomingQueue().Empty())
			{
				auto messageIn = c.IncomingQueue().PopFront().message;

				switch (messageIn.header.id)
				{
				case CustomMsgType::ServerMessage:
				{
					uint32_t clientId;
					messageIn >> clientId;
					std::cout << "Broadcast from [" << clientId << "]" << "\n";
				}
				break;
				case CustomMsgType::ServerPing:
				{
					auto now = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					messageIn >> timeThen;
					std::cout << "Ping: " << std::chrono::duration_cast<std::chrono::microseconds>(now - timeThen).count() << "us." << "\n";
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server has shutdown\n";
			isQuit = true;
		}
	}

	return 0;
}