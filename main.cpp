#include <string>
#include <iostream>

#include "TCPEchoServer.hpp"
#include "TestClient.hpp"

void man(const std::string &progName){
	std::cout << "Usage:" << std::endl;
	std::cout << progName << " -s|-c port" << std::endl;
}

int main(int argc, char **argv){
	if(argc != 3){
		man(argv[0]);
		return -1;
	}

	const std::string mode(argv[1]);
	const int port = std::stoi(argv[2]);

	if(mode == "-s"){
		TCPEchoServer echoServer(port);
		echoServer.run();
	}
	else if(mode == "-c"){
		TestClient client("127.0.0.1", port);
		client.run(1000);
	}
	else{
		man(argv[0]);
		return -1;
	}
}

