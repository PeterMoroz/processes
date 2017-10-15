#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

#include <chrono>
#include <exception>
#include <iostream>
#include <thread>
#include <vector>

#include "process_launcher.h"


std::vector<std::string> split(std::string s, char d)
{
	std::vector<std::string> r;
	
	std::size_t p0 = 0;
	std::size_t p1 = s.find_first_of(d);
	if (p1 == std::string::npos)
	{
		r.emplace_back(s);
		return r;
	}
		
	while (p1 != std::string::npos)
	{
		r.emplace_back(s.substr(p0, p1 - p0));
		p0 = p1 + 1;
		p1 = s.find_first_of(d, p0);
	}
	r.emplace_back(s.substr(p0));
	
	return r;
}

ProcessLauncher::ProcessLauncher()
	: _running(false)
	, _commands()
{
}

ProcessLauncher::~ProcessLauncher()
{
	
}

void ProcessLauncher::add_command(const std::string& command) noexcept
{
	try {
		_commands.emplace_back(command);
	} catch (const std::exception& ex) {
		std::cerr << "Exception: " << ex.what() << std::endl;
	}
}

void ProcessLauncher::run() noexcept
{
	std::string cmd;
	_running = true;
	while (_running)
	{
		cmd.clear();
		std::cout << "PID " << ::getpid() << ", fetch next command..." << std::endl;
		if (!_commands.empty())
		{
			cmd = _commands.front();
			_commands.pop_front();
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		
		if (!cmd.empty())
		{
			std::cout << "PID " << ::getpid() << " command '" << cmd << "'" << std::endl;
			pid_t pid = ::fork();
			if (pid < 0)
			{
				std::cerr << "fork() failed: " << ::strerror(errno) << std::endl;
			}
			else if (pid == 0)
			{
				std::cout << "PID " << ::getpid() << " PPID " << ::getppid() << " command '" << cmd << "'" << std::endl;
				break;
			}
			else
			{
				std::cout << "PID " << ::getpid() << " pid " << pid << " command '" << cmd << "'" << std::endl;
				try {
					std::thread background_wait = std::thread([=]()
						{
							int status = 0;
							pid_t child_pid = ::waitpid(pid, &status, 0);
							std::cout << " process " << pid << " finished with status " << status << " and PID " << child_pid << std::endl;					
						});
					background_wait.detach();
				} catch (const std::exception& ex) {
					std::cerr << "Exception: " << ex.what() << std::endl;
				}
			}
		}
	}
		
	std::vector<std::string> arguments = split(cmd, ' ');
	static constexpr std::size_t MAX_ARGS = 64;
	char** args = new char*[arguments.size() + 1];
	//std::cout << " Arguments list: " << std::endl;
	std::size_t idx = 0;
	for (; idx < arguments.size() && idx < MAX_ARGS; idx++)
	{
		//std::cout << arguments[idx] << std::endl;
		args[idx] = const_cast<char*>(arguments[idx].c_str());
	}
	args[idx] = NULL;
	
	if (::execvp(args[0], args) == -1)
	{
		std::cerr << "execvp() failed: " << ::strerror(errno) << std::endl;
	}
}
