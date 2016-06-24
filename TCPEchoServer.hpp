#ifndef TCPECHOSERVER_HPP
#define TCPECHOSERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <utility>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <tuple>

#include "SocketDescriptorHandler.hpp"

class TCPEchoServer{
	static constexpr int socketBacklogSize = 10;
	const SocketDescriptorHandler listeningSocketDescriptor;

	std::atomic<bool> stopFlag;
	std::vector<std::tuple<sockaddr_in, socklen_t, SocketDescriptorHandler>> incoming;
	std::mutex incomingAccess;

	static constexpr size_t bufferSize = 512;
	std::unique_ptr<char[]> buffer;

public:
	TCPEchoServer(const int listeningPort): listeningSocketDescriptor(socket(AF_INET, SOCK_STREAM, 0)), buffer(new char[bufferSize]){
		if(this->listeningSocketDescriptor == -1){
			throw std::runtime_error(std::string("Socket opening error: ") + strerror(errno));
		}

		sockaddr_in sin;
		std::memset(&sin, 0, sizeof(sin));

		sin.sin_family = AF_INET;
		sin.sin_port = htons(listeningPort);
		sin.sin_addr.s_addr = INADDR_ANY;

		const int bindRes = bind(this->listeningSocketDescriptor, reinterpret_cast<sockaddr *>(&sin), sizeof(sin));
		if(bindRes == -1){
			throw std::runtime_error(std::string("Bind error ") + strerror(errno));
		}

		const int listenRes = listen(this->listeningSocketDescriptor, TCPEchoServer::socketBacklogSize);
		if(listenRes == -1){
			throw std::runtime_error(std::string("Listen error ") + strerror(errno));
		}
	}

	void run(){
		sockaddr_in clientAddr;
		socklen_t sl;

		this->stopFlag = false;

		std::thread echoLoopThread(&TCPEchoServer::echoLoop, this);
		echoLoopThread.detach();

		while(true){
			std::memset(&clientAddr, 0, sizeof(clientAddr));
			std::memset(&sl, 0, sizeof(sl));

			SocketDescriptorHandler newSocketDescriptor(accept(this->listeningSocketDescriptor, reinterpret_cast<sockaddr *>(&clientAddr), &sl));
			if(newSocketDescriptor == -1){
				throw std::runtime_error(std::string("Unexpected `accept()` error: ") + strerror(errno));
			}

			std::cout << "Incoming connection" << std::endl;

			std::lock_guard<std::mutex> lg(this->incomingAccess);
			this->incoming.emplace_back(clientAddr, sl, std::move(newSocketDescriptor));
		}
	}

private:
	void echoLoop(){
		while(this->stopFlag == false){
			this->incomingAccess.lock();

			for(const std::tuple<sockaddr_in, socklen_t, SocketDescriptorHandler> &client : this->incoming){

				const ssize_t recvRes = recv(std::get<2>(client), this->buffer.get(), TCPEchoServer::bufferSize, MSG_DONTWAIT);
				if(recvRes == -1){
					switch(errno){
					case EAGAIN:
						continue;
					default:
						throw std::runtime_error(std::string("Unexpected recv error: ") + strerror(errno));
					}
				}

				if(recvRes == 0){
					continue; // the peer has performed an orderly shutdown.
				}

				std::cout << "Incoming message:" << std::endl;

				for(ssize_t pos = 0; pos < recvRes; ++pos){
					std::cout << this->buffer[pos];
				}

				std::cout << std::endl;

				const ssize_t res = send(std::get<2>(client), this->buffer.get(), recvRes, 0);
				if(res == -1){
					throw std::runtime_error(std::string("Unexpected send error: ") + strerror(errno));
				}

				this->incomingAccess.unlock();

				std::cout << "Reply was succesfully sent" << std::endl;

				std::this_thread::sleep_for(std::chrono::milliseconds(10));

				this->incomingAccess.lock();
			}

			this->incomingAccess.unlock();
		}
	}
};

#endif // TCPECHOSERVER_HPP

