#ifndef NETMESSAGE_HPP
#define NETMESSAGE_HPP
#include "NetCommon.hpp"

namespace net
{
	template <typename T>
	struct MessageHeader
	{
		T id{};
		uint32_t size = 0;
	};

	template<typename T>
	struct Message
	{
		MessageHeader<T> header{};
		std::vector<uint8_t> body;

		size_t Size() const
		{
			return body.size();
		}

		friend std::ostream& operator << (std::ostream& os, const Message<T>& message)
		{
			os << "ID: " << static_cast<int>(message.header.id) << "Size" << message.header.size;
			return os;
		}

		// To store POD's on de Message struct
		template<typename Datatype>
		friend Message<T>& operator << (Message<T>& message, const Datatype& data)
		{
			static_assert(std::is_standard_layout<Datatype>::value, "Data is not a POD-like object");
			size_t i = message.body.size();
			message.body.resize(message.body.size() + sizeof(Datatype));
			std::memcpy(message.body.data() + i, &data, sizeof(Datatype));
			message.header.size = message.Size();
			return message;
		}

		template<typename Datatype>
		friend Message<T>& operator >> (Message<T>& message, Datatype& data)
		{
			static_assert(std::is_standard_layout<Datatype>::value, "Data is not a POD-like object");
			size_t i = message.body.size() - sizeof(Datatype);
			std::memcpy(&data, message.body.data() + i, sizeof(Datatype));
			message.body.resize(i);
			message.header.size = message.Size();
			return message;
		}
	};

	template<typename T> class Connection;

	template<typename T>
	class OwnedMessage
	{
	public:
		std::shared_ptr<Connection<T>> remote = nullptr;
		Message<T> message;

	public:
		friend std::ostream& operator<<(std::ostream& os, const OwnedMessage<T>& message)
		{
			os << message.message;
			return os;
		}
	};

}

#endif
