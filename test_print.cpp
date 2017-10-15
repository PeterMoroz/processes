#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char* argv[])
{
	char* message = argc < 2 ? argv[0] : argv[1];
	
	std::cout << ">>> PID: " << ::getpid() << ", PPID: " << ::getppid() << ", message: " << message << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(10));
	return 0;
}