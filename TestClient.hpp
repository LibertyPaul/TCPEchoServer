#ifndef TESTCLIENT_HPP
#define TESTCLIENT_HPP

#include <memory>
#include <cstring>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <random>
#include <chrono>
#include <cassert>
#include <algorithm>

#include "SocketDescriptorHandler.hpp"

class TestClient{
	const SocketDescriptorHandler serverSocketDescriptor;

	static constexpr size_t bufferSize = 512;
	std::unique_ptr<char[]> buffer;

public:
	TestClient(const std::string &ip, const int serverPort): serverSocketDescriptor(socket(AF_INET, SOCK_STREAM, 0)), buffer(new char[bufferSize]){
		if(this->serverSocketDescriptor == -1){
			throw std::runtime_error(std::string("Socket opening error: ") + strerror(errno));
		}

		sockaddr_in sock_in;
		std::memset(&sock_in, 0, sizeof(sockaddr_in));
		sock_in.sin_family = AF_INET;
		sock_in.sin_port = htons(serverPort);

		const int atonRes = inet_aton(ip.c_str(), reinterpret_cast<in_addr *>(&sock_in.sin_addr.s_addr));
		if(atonRes == 0){
			throw std::runtime_error(std::string("inet_aton error ") + strerror(errno));
		}

		const int connectRes = connect(this->serverSocketDescriptor, reinterpret_cast<sockaddr *>(&sock_in), sizeof(sock_in));
		if(connectRes == -1){
			throw std::runtime_error(std::string("Connect error ") + strerror(errno));
		}
	}

private:
	static std::string randomString(const size_t size){
		static std::default_random_engine rg(std::chrono::system_clock::now().time_since_epoch().count());
		static std::uniform_int_distribution<char> alphabetDistr('a', 'z');

		std::string result;
		result.reserve(size);

		for(size_t i = 0; i < size; ++i){
			result.push_back(alphabetDistr(rg));
		}

		return result;
	}

public:
	void run(const size_t messageCount){
		const size_t stringSize = 25;
		assert(stringSize <= TestClient::bufferSize);

		for(size_t i = 0; i < messageCount; ++i){
			const std::string currentString = TestClient::randomString(stringSize);
			std::copy(currentString.cbegin(), currentString.cend(), this->buffer.get());

			const ssize_t sendRes = send(this->serverSocketDescriptor, this->buffer.get(), stringSize, 0);
			if(sendRes == -1){
				throw std::runtime_error(std::string("send error: ") + strerror(errno));
			}

			std::cout << "Sent string:     '" << currentString << "'" << std::endl;

			const ssize_t recvRes = recv(this->serverSocketDescriptor, this->buffer.get(), TestClient::bufferSize, 0);
			if(recvRes == -1){
				throw std::runtime_error(std::string("recv error: ") + strerror(errno));
			}

			std::cout << "Recieved string: '";
			for(ssize_t pos = 0; pos < recvRes; ++pos){
				std::cout << this->buffer[pos];
			}

			std::cout << "'" << std::endl;

			const bool isEqual = std::equal(currentString.cbegin(), currentString.cend(), this->buffer.get());
			if(isEqual == false){
				throw std::runtime_error("reply is not equal");
			}

		}
	}
};

#endif // TESTCLIENT_HPP

