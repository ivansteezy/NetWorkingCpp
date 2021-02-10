#ifndef NETSERVER_H
#define NETSERVER_H

#include "NetCommon.hpp"
#include "NetThreadSafeQueue.hpp"
#include "NetMessage.hpp"
#include "NetConnection.hpp"

namespace net
{
	//base class for a custom implementation of a server
	template <typename T>
	class ServerInterface
	{
	public:
		ServerInterface(uint16_t port) :
			m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{

		}

		virtual ~ServerInterface()
		{
			Stop();
		}

		bool Start()
		{
			try
			{
				WaitForClientConnection();
				m_threadContext = std::thread([this]() { m_asioContext.run(); });
			}
			catch (std::exception e)
			{
				std::cerr << "[SERVER SIDE] Exception: " << e.what() << "\n";
				return false;
			}

			std::cout << "[SERVER SIDE] Started. \n";
			return true;
		}

		void Stop()
		{
			m_asioContext.stop();
			if (m_threadContext.joinable()) m_threadContext.join();
			std::cout << "[SERVER SIDE] Stoped. \n";
		}

		//ASYNC TASK
		void WaitForClientConnection()
		{
			m_asioAcceptor.async_accept([this](std::error_code ec, asio::ip::tcp::socket socket) {
				if (!ec)
				{
					std::cout << "[SERVER SIDE] New connection done!: " << socket.remote_endpoint() << "\n";
					auto tempConnection = std::make_shared<Connection<T>>(Connection<T>::ConnectionOwner::Server, m_asioContext, std::move(socket), m_messagesInQueue);

					//by default returns false, provide an override for some connection control
					if (OnClientConnect(tempConnection))
					{
						m_deqConnections.push_back(std::move(tempConnection));
						m_deqConnections.back()->ConnectToClient(nIdCounter++);

						std::cout << "[SERVER SIDE]" << "<" << m_deqConnections.back()->GetId() << "> connection approved!\n";
					}
					else
					{
						std::cout << "[SERVER SIDE] Connection Denied \n";
					}
				}
				else
				{
					std::cout << "[SERVER SIDE] Error on new connection: " << ec.message() << "\n";
				}

				//put server to a listen state again
				WaitForClientConnection();
				});
		}

		//Send a Message to a specific client
		void MessageClient(std::shared_ptr<Connection<T>> client, const net::Message<T>& message)
		{
			if (client && client->IsConnected())
			{
				client->Send(message);
			}
			else
			{
				//assume that the client is disconnected
				OnClientDisconnect(client);
				client.reset();
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections);
			}
		}

		void Broadcast(const net::Message<T>& message, std::shared_ptr<Connection<T>> ignoredClient = nullptr)
		{
			bool invalidClientExists = false; //for quick optimization and not iterate over all

			for (auto& client : m_deqConnections)
			{
				if (client && client->IsConnected())
				{
					if (client != ignoredClient)
					{
						client->Send(message);
					}
				}
				else
				{
					OnClientDisconnect(client);
					client.reset();
					invalidClientExists = true;
				}

			}
			if (invalidClientExists)
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
		}

		void Update(size_t maxMessage = -1)
		{
			size_t messageCount = 0;
			while (messageCount < maxMessage && !m_messagesInQueue.Empty())
			{
				auto latestMessage = m_messagesInQueue.PopFront();
				OnMessage(latestMessage.remote, latestMessage.message);
				messageCount++;
			}
		}

	protected:
		//expected to override
		virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
		{
			//for banned ip's or something
			return false;
		}

		virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client)
		{

		}

		//when a message arrives
		virtual void OnMessage(std::shared_ptr<Connection<T>> client, net::Message<T>& message)
		{

		}

	protected:
		net::ThreadSafeQueue<OwnedMessage<T>> m_messagesInQueue;
		std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;
		//first initialize the context , then the thread
		asio::io_context m_asioContext;
		std::thread m_threadContext;

		//here we "store" the sockets from the clients connected
		asio::ip::tcp::acceptor m_asioAcceptor;

		//id for the client across the system
		uint32_t nIdCounter = 10000;
	};
}

#endif
