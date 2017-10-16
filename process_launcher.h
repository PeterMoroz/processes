#pragma once


#include <list>
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
	
	void add_process_observer(ProcessObserver* process_observer);
		
private:
	bool _running;
	std::list<std::string> _commands;
	std::list<ProcessObserver*> _observers;
};