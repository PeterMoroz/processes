#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

#include "process.h"
#include "process_observer.h"

class ProcessLauncher; 

/*!
 \brief Class, which worries about life of processes from observed list, and restart those ones which are not running anymore.
 */
class ProcessSupervisor final : public ProcessObserver
{
	ProcessSupervisor(const ProcessSupervisor&) = delete;
	const ProcessSupervisor& operator=(const ProcessSupervisor&) = delete;
	ProcessSupervisor(ProcessSupervisor&&) = delete;
	const ProcessSupervisor& operator=(ProcessSupervisor&&) = delete;
	
public:
	/// how long worker thread will sleep, if nothing to watch (milliseconds)
	static constexpr unsigned DEFAULT_WATCH_DELAY = 100;
	
public:
	explicit ProcessSupervisor(ProcessLauncher& process_launcher,
							unsigned watch_delay = DEFAULT_WATCH_DELAY);
	~ProcessSupervisor();
	/*!
	 Start worker thread.
	 */
	void run();
	/*!
	 Stop worker thread.
	 */
	void stop() noexcept;
	
	/// implementation of ProcessObserver-interface
	/*!
	 Add process to watching to list and awake worker thread, if it is sleeping.
	Ensure thread safe access to processes list.
	 */
	void add_process(const Process& process) final;
	
private:
	/*
	 Worker thread routine. 
	Goes through list of processes and check if each of them is alive, if no - tells to the process launcher restart it.
	 */
	void processes_handler() noexcept;
	
private:
	ProcessLauncher& _process_launcher;
	const unsigned _watch_delay;
	std::atomic<bool> _running;
	std::list<Process> _processes_list;
	std::mutex _proc_list_mutex;
	std::thread _handler_thread;
	std::mutex _handler_cv_mutex;
	std::condition_variable _handler_cv;
};
