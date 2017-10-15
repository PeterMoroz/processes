#pragma once


#include <list>
#include <string>


class ProcessLauncher
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
	
private:
	bool _running;
	std::list<std::string> _commands;
};