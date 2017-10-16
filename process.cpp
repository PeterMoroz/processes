#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>

#include <fstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>


#include "process.h"


Process::Process(pid_t pid, const std::string& cmd_line)
	: _pid(pid)
	, _cmd_line(cmd_line)
	, _status()
{
	
}

Process::~Process()
{
	// don't release any resource here
}

void Process::interrupt() noexcept
{
	try {
		signal(SIGINT);
		/* To avoid zombies, aprent process have to call waitpid()
		if (alive())
			wait(); 
		*/
	} catch (const std::exception& ex) {
		std::cerr << "An error when interrupt process, PID = " << _pid << ".\nDetails: " << ex.what() << std::endl;
	}
}

void Process::wait()
{
	if (_pid <= 0)
		throw std::logic_error("Invalid PID.");
	
	pid_t pid = ::waitpid(_pid, &_exit_status, 0);
	if (pid == -1)
	{
		char err_msg_buffer[256] = { '\0' };
		const char* err_msg = ::strerror_r(errno, err_msg_buffer, sizeof(err_msg_buffer));
		throw std::runtime_error(std::string("waitpid() failed, details: ").append(err_msg));
	}
}

bool Process::alive() const
{
	if (_pid <= 0)
		throw std::logic_error("Invalid PID.");
		
	if (::kill(_pid, 0) != 0)
	{
		if (errno == ESRCH)
			return false;
		if (errno == EPERM)
		{
			std::stringstream ss;
			ss << "kill() failed, process " << ::getpid() 
				<< " has no permissions to send signal to process " << _pid;
			throw std::logic_error(ss.str());
		}
	}
	return true;
}

void Process::update_status()
{
	if (_pid <= 0)
		throw std::logic_error("Invalid PID.");
		
	std::stringstream ss;
	ss << "/proc/" << _pid << "/status";
	const std::string path(ss.str());
	
	std::ifstream status_file(path.c_str());
	if (!status_file)
		throw std::runtime_error(std::string("Can't open file: ").append(path));
		
	std::string line;
	while (!status_file.eof())
	{
		std::getline(status_file, line);
		if (line.empty())
			continue;
		std::size_t p = line.find_first_of(':');
		if (p == std::string::npos)
			continue;
		const std::string key(line.substr(0, p));
		_status[key] = line.substr(p + 1);
	}
}

const std::string& Process::get_status_field(const std::string& name) const
{
	Status::const_iterator it = _status.find(name);
	if (it == _status.end())
		throw std::logic_error("no field with such name.");
	return it->second;
}


void Process::signal(int signum) const
{
	if (_pid <= 0)
		throw std::logic_error("Invalid PID.");
	
	if (::kill(_pid, signum) == -1)
	{
		char err_msg_buffer[256] = { '\0' };
		const char* err_msg = ::strerror_r(errno, err_msg_buffer, sizeof(err_msg_buffer));
		throw std::runtime_error(std::string("kill() failed, details: ").append(err_msg));
	}
}
