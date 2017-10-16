#pragma once

#include <sys/types.h>

#include <map>
#include <string>

/*!\brief
 * Lightweight class-wrapper. It provides interface to syscalls, 
 * related to process management, don't control lifetime
 * of process, so more than one instance of given class
 * may refers to the same process.
 */
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

	/*!
	 * Send SIGINT to process, dont't wait.
	 */
	void interrupt() noexcept;
	/*!
	 * Wait untill process will finish and get exit status.
	 */
	void wait();
	/*!
	 * Check if process is alive by sending to it signal 0.
	 * \return true, if PID is used, false otherwise.
	 */
	bool alive() const;
	
	pid_t pid() const noexcept { return _pid; }
	const std::string& cmd_line() const noexcept
	{
		return _cmd_line;
	}
	int exit_status() const { return _exit_status; }
	
	/*!
	 * Read status information from the file /proc/<PID>/status,
	 * parse lines to pairs key:value and store them in container with type Status.
	 */
	void update_status();
	/*!
	 * Get value of field with given name from container of Status type.
	 * If no field with such field, throw exception.
	 */
	const std::string& get_status_field(const std::string& name) const;
	
private:
	/*
	 * Send signal with given number to process.
	 */
	void signal(int signum) const;

private:
	pid_t _pid;						///< PID value
	const std::string _cmd_line;	///< command, which was used to spawn process
	int _exit_status;				///< exit status of process
	Status _status;					///< status information, obtained from /proc/<PID>/status
};
