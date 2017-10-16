#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>

#include "process.h"
#include "process_observer.h"

class ProcessLauncher; 

class ProcessSupervisor final : public ProcessObserver
{
	ProcessSupervisor(const ProcessSupervisor&) = delete;
	const ProcessSupervisor& operator=(const ProcessSupervisor&) = delete;
	ProcessSupervisor(ProcessSupervisor&&) = delete;
	const ProcessSupervisor& operator=(ProcessSupervisor&&) = delete;
	
public:
	static constexpr unsigned DEFAULT_WATCH_DELAY = 100; // milliseconds
	
public:
	explicit ProcessSupervisor(ProcessLauncher& process_launcher,
							unsigned watch_delay = DEFAULT_WATCH_DELAY);
	~ProcessSupervisor();
	
	void run();
	void stop() noexcept;
	
	// implementation of ProcessObserver-interface
	void add_process(const Process& process) final;
	
private:
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
