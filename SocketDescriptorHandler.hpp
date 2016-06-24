#ifndef SOCKETDESCRIPTORHANDLER_HPP
#define SOCKETDESCRIPTORHANDLER_HPP

#include <cerrno>
#include <unistd.h>

typedef int SocketDescriptor;

class SocketDescriptorHandler{
	const SocketDescriptor descriptor;
	bool valid;

public:
	explicit SocketDescriptorHandler(const SocketDescriptor descriptor): descriptor(descriptor), valid(true){}

	SocketDescriptorHandler(SocketDescriptorHandler &&handler): descriptor(handler.descriptor), valid(handler.valid){
		handler.valid = false;
	}

	~SocketDescriptorHandler(){
		if(this->valid){
			const int res = close(this->descriptor);
			if(res == -1){
				perror("close error");
			}
		}
	}

	operator SocketDescriptor() const{
		return this->descriptor;
	}

};

#endif // SOCKETDESCRIPTORHANDLER_HPP

