//SERVER SIDE
#include <iostream>
#include "../Common/Net.hpp"

enum class CustomMsgType : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	BroadCast,
	ServerMessage
};

class MyServer : public net::ServerInterface<CustomMsgType>
{
public:
	MyServer(uint16_t port) : net::ServerInterface<CustomMsgType>(port)
	{

	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<net::Connection<CustomMsgType>> client)
	{
		return true;
	}

	virtual void OnClientDisconnect(std::shared_ptr<net::Connection<CustomMsgType>> client)
	{

	}

	virtual void OnMessage(std::shared_ptr<net::Connection<CustomMsgType>> client, net::Message<CustomMsgType>& message)
	{
		switch (message.header.id)
		{
		case CustomMsgType::BroadCast:
		{
			std::cout << "[" << client->GetId() << "]: BROADCAST \n";
			net::Message<CustomMsgType> message;
			message.header.id = CustomMsgType::ServerMessage;
			message << client->GetId();
			Broadcast(message, client);
		}
		break;
		case CustomMsgType::ServerAccept: break;
		case CustomMsgType::ServerDeny: break;
		case CustomMsgType::ServerMessage: break;
		case CustomMsgType::ServerPing:
		{
			std::cout << "Ping from client: " << client->GetId() << "\n";
			client->Send(message);
		}
		break;
		}
	}
};

int main()
{
	MyServer server(60000);
	server.Start();
	while (1)
	{
		server.Update();
	}
	return 0;
}