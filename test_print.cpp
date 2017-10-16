#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
	const char* message = argc < 2 ? argv[0] : argv[1];	
	std::cout << ">>> PID: " << ::getpid() << ", PPID: " << ::getppid() << ", message: " << message << std::endl;
	
	const char* buffsize = argc < 3 ? "65536" : argv[2];
	
	unsigned char* buffer = NULL;
	try {
		std::size_t buffer_size = std::atoi(buffsize);
		buffer_size *= 1024;
		if (buffer_size)
		{
			buffer = new unsigned char[buffer_size];
			std::cout << ">>> PID: " << ::getpid() << ", allocated " << buffer_size << " bytes." << std::endl;
		}
	} catch (const std::exception& ex) {
		std::cerr << ">>> PID: " << ::getpid() << " caught exception " << ex.what() << std::endl;
	}
	
	std::this_thread::sleep_for(std::chrono::seconds(90));
	delete [] buffer;
	
	return 0;
}