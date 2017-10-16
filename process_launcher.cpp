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
			try {
				Process process;
				std::cout << "PID " << ::getpid() << " run command '" << cmd << "' in background " << std::endl;
				process.create(cmd, true);
				std::cout << "PID " << ::getpid() << " notify listeners about new children, pid " << process.pid() << std::endl;
				
				std::list<ProcessObserver*>::iterator it = _observers.begin();
				while (it != _observers.end())
				{
					(*it)->add_process(process);
					++it;
				}
				
			} catch (const std::exception& ex) {
				std::cerr << "Exception: " << ex.what() << std::endl;
			}
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
