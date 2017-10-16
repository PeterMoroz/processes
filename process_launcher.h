#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include <string>

class ProcessObserver;

class ProcessLauncher final
{
	ProcessLauncher(const ProcessLauncher&) = delete;
	const ProcessLauncher& operator=(const ProcessLauncher&) = delete;
	ProcessLauncher(ProcessLauncher&&) = delete;
	const ProcessLauncher& operator=(ProcessLauncher&&) = delete;
	
public:
	ProcessLauncher();
	~ProcessLauncher();
	
	void add_command(const std::string& command) noexcept;
	void run() noexcept;
	
	void stop() noexcept;
	
	void add_process_observer(ProcessObserver* process_observer);
	
private:
	std::string fetch_command();
		
private:
	bool _running;
	std::list<std::string> _commands;
	std::mutex _commands_mutex;
	std::condition_variable _awake_cv;
	std::mutex _awake_cv_mutex;
	std::list<ProcessObserver*> _observers;
};