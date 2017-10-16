#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <cstring>

#include <chrono>
#include <exception>
#include <iostream>
#include <thread>
#include <vector>

#include "process.h"
#include "process_observer.h"
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
	, _commands_mutex()
	, _awake_cv()
	, _awake_cv_mutex()
	, _observers()
{
}

ProcessLauncher::~ProcessLauncher()
{
	
}

void ProcessLauncher::add_command(const std::string& command) noexcept
{
	try {
		
		{
			std::lock_guard<std::mutex> lg(_commands_mutex);
			_commands.emplace_back(command);
		}
		
		{
			std::lock_guard<std::mutex> lg(_awake_cv_mutex);
			_awake_cv.notify_one();
		}
		
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

		{
			std::unique_lock<std::mutex> ul(_awake_cv_mutex);
			_awake_cv.wait(ul, [this](){ return (!_running || !_commands.empty()); });
		}
		
		if (!_running)
			break;
			
		cmd = fetch_command();
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
				
				Process process(pid, cmd);
				std::list<ProcessObserver*>::iterator it = _observers.begin();
				while (it != _observers.end())
				{
					(*it)->add_process(process);
					++it;
				}
				
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
	
	if (!cmd.empty())
	{
		std::cout << " launcher is going to spawn child process, command: " << cmd << std::endl;
		std::vector<std::string> arguments = split(cmd, ' ');
		static constexpr std::size_t MAX_ARGS = 64;
		char** args = new char*[arguments.size() + 1];	// Memory leak! Move variable to global scope and register cleanup function by means atexit()
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
	
}

void ProcessLauncher::stop() noexcept
{
	try {
		if (_running)
		{
			_running = false;
			{
				std::lock_guard<std::mutex> lg(_awake_cv_mutex);
				_awake_cv.notify_one();
			}
		}
	} catch (const std::exception& ex) {
		std::cerr << "Exception when stop: " << ex.what() << std::endl;
	}
}

void ProcessLauncher::add_process_observer(ProcessObserver* process_observer)
{
	assert(process_observer != NULL);
	_observers.push_back(process_observer);
}


std::string ProcessLauncher::fetch_command()
{
	std::string cmd;
	std::lock_guard<std::mutex> lg(_commands_mutex);
	if (!_commands.empty())
	{
		cmd = _commands.front();
		_commands.pop_front();
	}
	return cmd;
}
