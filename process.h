#pragma once

#include <sys/types.h>

#include <map>
#include <string>

/// Lightweight class-wrapper. It provides interface to syscalls, 
/// related to process management, don't control lifetime
/// of process, so more than one instance of given class
/// may refers to the same process.

class Process final
{
	/*Process(const Process&) = delete;
	const Process& operator=(const Process&) = delete;
	Process(Process&&) = delete;
	const Process& operator=(Process&&) = delete;*/
	
	using Status = std::map<std::string, std::string>;
	
public:
	Process(pid_t pid, const std::string& cmd_line);
	~Process();
	
	void interrupt() noexcept;
	void wait();
	
	bool alive() const;
	
	pid_t pid() const noexcept { return _pid; }
	const std::string& cmd_line() const noexcept
	{
		return _cmd_line;
	}
	int exit_status() const { return _exit_status; }
	
	void update_status();
	const std::string& get_status_field(const std::string& name) const;
	
private:
	void signal(int signum) const;

private:
	pid_t _pid;
	const std::string _cmd_line;
	int _exit_status;
	Status _status;
};
