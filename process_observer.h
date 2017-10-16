#pragma once

class Process;

class ProcessObserver
{
public:
	virtual ~ProcessObserver() = default;
	
	virtual void add_process(const Process& process) = 0;
};
