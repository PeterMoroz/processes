#include <cassert>

#include <chrono>
#include <iostream>

#include "process_watchdog.h"


ProcessWatchdog::ProcessWatchdog(const ProcessRestriction& process_restriction,
								unsigned watch_delay /*= DEFAULT_WATCH_DELAY*/)
	: _process_restriction(process_restriction)
	, _watch_delay(watch_delay)
	, _running(false)
	, _processes_list()
	, _proc_list_mutex()
	, _handler_thread()
	, _handler_cv_mutex()
	, _handler_cv()
{
}

ProcessWatchdog::~ProcessWatchdog()
{
	if (_running)
		stop();	
}

void ProcessWatchdog::ProcessWatchdog::run()
{
	assert(!_running);
	
	_handler_thread = std::thread(&ProcessWatchdog::processes_handler, this);
}

void ProcessWatchdog::ProcessWatchdog::stop() noexcept
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

void ProcessWatchdog::add_process(const Process& process)
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

void ProcessWatchdog::processes_handler() noexcept
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
						it->update_status();
						if (violates_restrictions(*it))
						{
							std::cout << "watchdog is going to kill process " 
								<< it->pid() << "\t" << it->cmd_line() << std::endl;
							it->interrupt();
							std::cout << " process " << it->pid() << "\t" << it->cmd_line()
								<< "\tinterrupted\texit status " << it->exit_status() << std::endl;
							it = _processes_list.erase(it);
							continue;
						}
						++it;
					} catch (const std::exception& ex) {
						std::cerr << "Exception when testing violation of restrictions: " << ex.what() << std::endl;
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

bool ProcessWatchdog::violates_restrictions(const Process& process) const
{
	const std::string& vm_size_str = process.get_status_field("VmSize");
	unsigned vm_size_val = std::stoi(vm_size_str);
	
	std::cout << "PID: " << process.pid() << "\tVmSize: " << vm_size_val << std::endl;
	if (vm_size_val > _process_restriction._vm_size_max)
	{
		std::cout << "PID: " << process.pid() << ", actual value of VmSize = " << vm_size_val
			<< ", allowed value of VmSize = " << _process_restriction._vm_size_max << std::endl;
		return true;
	}
	return false;
}
