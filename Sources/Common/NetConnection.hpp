#ifndef NETCONNECTION_HPP
#define NETCONNECTION_HPP

#include "NetCommon.hpp"
#include "NetThreadSafeQueue.hpp"
#include "NetMessage.hpp"

namespace net
{
	template<typename T>
	class Connection : public std::enable_shared_from_this<Connection<T>>
	{
	public:
		enum class ConnectionOwner
		{
			Server,
			Client
		};

	public:
		Connection(ConnectionOwner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, net::ThreadSafeQueue<OwnedMessage<T>>& queueIn) :
			m_asioContext(asioContext),
			m_socket(std::move(socket)),
			m_queueMessageIn(queueIn)
		{
			m_ownerType = parent;
		}

		virtual ~Connection() {}

	public:
		void ConnectToServer(asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (m_ownerType == ConnectionOwner::Client)
			{
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							ReadHeader();
						}
					});
			}
		}


		void Disconnect()
		{
			if (IsConnected())
			{
				asio::post(m_asioContext, [this]() { m_socket.close(); });
			}
		}

		bool IsConnected() const
		{
			return m_socket.is_open();
		}

		void ConnectToClient(uint32_t uid = 0)
		{
			if (m_ownerType == ConnectionOwner::Server)
			{
				if (m_socket.is_open())
				{
					id = uid;
					ReadHeader();
				}
			}
		}

		uint32_t GetId() const
		{
			return id;
		}

	public:
		void Send(const Message<T>& message)
		{
			asio::post(m_asioContext, [this, message]()
				{
					bool isWritingMessage = !m_queueMessageOut.Empty();
					m_queueMessageOut.PushBack(message);
					if (!isWritingMessage)
					{
						WriteHeader();
					}
				});
		};

	private:
		void ReadHeader()
		{
			asio::async_read(m_socket, asio::buffer(&m_msgTemporary.header, sizeof(net::MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_msgTemporary.header.size > 0)
						{
							m_msgTemporary.body.resize(m_msgTemporary.header.size);
							ReadBody();
						}
						else
						{
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						std::cout << "[" << id << "] Read header fail!.\n";
						m_socket.close();
					}
				}
			);
		}

		void ReadBody()
		{
			asio::async_read(m_socket, asio::buffer(m_msgTemporary.body.data(), m_msgTemporary.body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						AddToIncomingMessageQueue();
					}
					else
					{
						std::cout << "[" << id << "] Read body fail!.\n";
						m_socket.close();
					}
				});
		}

		void WriteHeader()
		{
			asio::async_write(m_socket, asio::buffer(&m_queueMessageOut.Front().header, sizeof(net::MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_queueMessageOut.Front().header.size > 0)
						{
							WriteBody();
						}
						else
						{
							m_queueMessageOut.PopFront();
							if (!m_queueMessageOut.Empty())
							{
								WriteHeader();
							}
						}
					}
					else
					{
						std::cout << "[" << id << "] write header fail!.\n";
						m_socket.close();
					}
				}
			);
		}

		void WriteBody()
		{
			asio::async_write(m_socket, asio::buffer(m_queueMessageOut.Front().body.data(), m_queueMessageOut.Front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						m_queueMessageOut.PopFront();
						if (!m_queueMessageOut.Empty())
						{
							WriteHeader();
						}
					}
					else
					{
						std::cout << "[" << id << "] write body fail!.\n";
						m_socket.close();
					}
				}
			);
		}

		void AddToIncomingMessageQueue()
		{
			if (m_ownerType == ConnectionOwner::Server)
			{
				m_queueMessageIn.PushBack({ this->shared_from_this(), m_msgTemporary });
			}
			else
			{
				m_queueMessageIn.PushBack({ nullptr, m_msgTemporary }); // asume that the client only have 1 connection (with server)
			}

			ReadHeader();
		}

	protected:
		asio::ip::tcp::socket m_socket;
		asio::io_context& m_asioContext;
		ThreadSafeQueue<Message<T>> m_queueMessageOut;
		ThreadSafeQueue<OwnedMessage<T>>& m_queueMessageIn;
		net::Message<T> m_msgTemporary;
		ConnectionOwner m_ownerType = ConnectionOwner::Server;
		uint32_t id = 0;
	};
}

#endif