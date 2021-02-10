#ifndef NETCLIENT_HPP
#define NETCLIENT_HPP

#include "NetCommon.hpp"
#include "NetConnection.hpp"

namespace net
{
	template<typename T>
	class ClientInterface
	{
	public:
		ClientInterface() : m_socket(m_context)
		{

		}

		virtual ~ClientInterface()
		{
			Disconnect();
		}

	public:
		bool Connect(const std::string& host, const uint16_t port)
		{
			try
			{
				asio::ip::tcp::resolver resolver(m_context);
				auto endpoints = resolver.resolve(host, std::to_string(port));

				m_connection = std::make_unique<Connection<T>>(Connection<T>::ConnectionOwner::Client, m_context, asio::ip::tcp::socket(m_context), m_queueMessageIn);
				m_connection->ConnectToServer(endpoints);
				thrContext = std::thread([this]() { m_context.run(); });
				std::cout << "CLIENT CONNECTED" << std::endl;
			}
			catch (std::exception& e)
			{
				std::cerr << "CLIENT EXCEPTION: " << e.what() << "\n";
				return false;
			}
			return true;
		}

		void Disconnect()
		{
			if (IsConnected())
			{
				m_connection->Disconnect();
			}

			m_context.stop();
			if (thrContext.joinable())
				thrContext.join();

			m_connection.release();
		}

		bool IsConnected()
		{
			return m_connection ? m_connection->IsConnected() : false;
		}

	public:
		ThreadSafeQueue<OwnedMessage<T>>& IncomingQueue()
		{
			return m_queueMessageIn;
		}

		void Send(const net::Message<T> message)
		{
			if (IsConnected())
				m_connection->Send(message);
		}

	protected:
		std::thread thrContext;
		asio::io_context m_context;
		asio::ip::tcp::socket m_socket;
		std::unique_ptr<Connection<T>> m_connection;

	private:
		ThreadSafeQueue<OwnedMessage<T>> m_queueMessageIn;
	};
}

#endif