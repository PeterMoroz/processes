#include <cassert>

#include <chrono>
#include <iostream>

#include "process_launcher.h"
#include "process_supervisor.h"


ProcessSupervisor::ProcessSupervisor(ProcessLauncher& process_launcher,
						unsigned watch_delay/* = DEFAULT_WATCH_DELAY*/)
	: _process_launcher(process_launcher)
	, _watch_delay(watch_delay)
	, _running(false)
	, _processes_list()
	, _proc_list_mutex()
	, _handler_thread()
	, _handler_cv_mutex()
	, _handler_cv()
{
}

ProcessSupervisor::~ProcessSupervisor()
{
	if (_running)
		stop();
}

void ProcessSupervisor::run()
{
	assert(!_running);
	
	_handler_thread = std::thread(&ProcessSupervisor::processes_handler, this);
}

void ProcessSupervisor::stop() noexcept
{
	try {
		if (_running)
		{
			_running = false;
			{
				std::lock_guard<std::mutex> lg(_handler_cv_mutex);
				_handler_cv.notify_one();
			}
			_handler_thread.join();
		}
	} catch (const std::exception& ex) {
		std::cerr << "Exception when handler thread joined: " << ex.what() << std::endl;
	}	
}

void ProcessSupervisor::add_process(const Process& process)
{
	{
		std::lock_guard<std::mutex> lg(_proc_list_mutex);
		_processes_list.emplace_back(process);
	}
	
	{
		std::lock_guard<std::mutex> lg(_handler_cv_mutex);
		_handler_cv.notify_one();
	}
}

void ProcessSupervisor::processes_handler() noexcept
{
	try {
		_running = true;
		while (_running)
		{
			
			{
				std::unique_lock<std::mutex> ul(_handler_cv_mutex);
				_handler_cv.wait(ul, [this](){ return (!_running || !_processes_list.empty()); });
			}
			
			if (!_running)
				break;
			
			{
				std::lock_guard<std::mutex> lg(_proc_list_mutex);
				std::list<Process>::iterator it = _processes_list.begin();
				while (it != _processes_list.end())
				{
					try {
						//std::cout << "check whether process " << it->pid() << " is alive." << std::endl;
						if (!it->alive())
						{
							std::cout << " process " << it->pid() << " isn't alive anymore, restart\t" << it->cmd_line() << std::endl;
							_process_launcher.add_command(it->cmd_line());
							it = _processes_list.erase(it);
							continue;
						}
						++it;
					} catch (const std::exception& ex) {
						std::cerr << "Exception when testing status of process " << it->pid() << "\nDetails: " << ex.what() << std::endl;
						it = _processes_list.erase(it);
					}
				}
			}
			
			std::this_thread::sleep_for(std::chrono::milliseconds(_watch_delay));
		}
	} catch (const std::exception& ex) {
		std::cerr << "Exception in thread routine: " << ex.what() << std::endl;
		_running = false;
	}
}
