#pragma once

#include <condition_variable>
#include <list>
#include <mutex>
#include <string>

class ProcessObserver;

/*!
 \brief Class, responsible for launching of commands from list as child processes.
 */

class ProcessLauncher final
{
	ProcessLauncher(const ProcessLauncher&) = delete;
	const ProcessLauncher& operator=(const ProcessLauncher&) = delete;
	ProcessLauncher(ProcessLauncher&&) = delete;
	const ProcessLauncher& operator=(ProcessLauncher&&) = delete;
	
public:
	ProcessLauncher();
	~ProcessLauncher();
	
	/*!
	 * Add command to launch to list, thread safe.
	 \param[in] command Command with arguments, which will be used to launch child process.
	 */
	void add_command(const std::string& command) noexcept;
	/*!
	 * Main loop of program, which will be parent process.
	 */
	void run() noexcept;
	/*!
	 * Method to interrupt main loop, for example from signal handler.
	 */
	void stop() noexcept;
	/*!
	 * Add instance of class, which implements ProcessObserver interface,
	 * i.e. interested in receiving of notifications about launching of new process.
	 */
	void add_process_observer(ProcessObserver* process_observer);
	
private:
	/*!
	 * Get next command to execute from list, thread safe.
	 */
	std::string fetch_command();
		
private:
	bool _running;
	std::list<std::string> _commands;
	std::mutex _commands_mutex;
	std::condition_variable _awake_cv;
	std::mutex _awake_cv_mutex;
	std::list<ProcessObserver*> _observers;
};