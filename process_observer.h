#pragma once

class Process;

/*!
 \brief Declaration of interface of classes, which are interested in receiving of notification, about new process is created.
 */
class ProcessObserver
{
public:
	virtual ~ProcessObserver() = default;
	
	virtual void add_process(const Process& process) = 0;
};
