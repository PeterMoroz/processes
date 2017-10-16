#pragma once

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>


#include "process.h"
#include "process_observer.h"
#include "process_restriction.h"

/*!
 \brief Class, which is responsible for watching about resource consumption by proceses from the list.
 */

class ProcessWatchdog final : public ProcessObserver
{
	ProcessWatchdog(const ProcessWatchdog&) = delete;
	const ProcessWatchdog& operator=(const ProcessWatchdog&) = delete;
	ProcessWatchdog(ProcessWatchdog&&) = delete;
	const ProcessWatchdog& operator=(ProcessWatchdog&&) = delete;
	
public:
	/// how long worker thread will sleep, if nothing to watch (milliseconds)
	static constexpr unsigned DEFAULT_WATCH_DELAY = 100;
	
public:
	explicit ProcessWatchdog(const ProcessRestriction& process_restriction,
							unsigned watch_delay = DEFAULT_WATCH_DELAY);
	~ProcessWatchdog();
	
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
	void processes_handler() noexcept;
	bool violates_restrictions(const Process& process) const;
	
private:
	const ProcessRestriction _process_restriction;
	const unsigned _watch_delay;
	std::atomic<bool> _running;
	std::list<Process> _processes_list;
	std::mutex _proc_list_mutex;
	std::thread _handler_thread;
	std::mutex _handler_cv_mutex;
	std::condition_variable _handler_cv;
};
