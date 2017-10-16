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
#include <thread>

#include "utils.h"
#include "process.h"

Process::Process()
	: _pid(-1)
	, _cmd_line()
	, _status()
{
	
}

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

void Process::attach(pid_t pid)
{
	if (pid <= 0)
		throw std::logic_error("Invalid PID.");
		
	_pid = pid;
	_cmd_line.clear();
	_status.clear();
}

void Process::create(const std::string& cmd_line, bool run_in_background/*=false*/)
{
	if (cmd_line.empty())
		throw std::logic_error("Invalid command line.");
		
	pid_t pid = ::fork();
	if (pid < 0)
	{
		char err_msg_buffer[256] = { '\0' };
		const char* err_msg = ::strerror_r(errno, err_msg_buffer, sizeof(err_msg_buffer));
		throw std::runtime_error(std::string("fork() failed, details: ").append(err_msg));
	}
	
	if (pid == 0)
	{
		std::cout << " start of spawning child process, command: " << cmd_line << std::endl;
		std::vector<std::string> arguments = utils::split(cmd_line, ' ');
		static constexpr std::size_t MAX_ARGS = 64;
		char** args = new char*[arguments.size() + 1];	// Memory leak! Move variable to global scope and register cleanup function by means atexit()
		//std::cout << " Arguments list: " << std::endl;
		std::size_t idx = 0;
		for (; idx < arguments.size() && idx < MAX_ARGS; idx++)
		{
			//std::cout << arguments[idx] << std::endl;
			args[idx] = const_cast<char*>(arguments[idx].c_str());
		}
		args[idx] = NULL;
		
		if (::execvp(args[0], args) == -1)
		{
			char err_msg_buffer[256] = { '\0' };
			const char* err_msg = ::strerror_r(errno, err_msg_buffer, sizeof(err_msg_buffer));
			throw std::runtime_error(std::string("execvp() failed, details: ").append(err_msg));			
		}
	}
	else
	{
		_pid = pid;
		_cmd_line = cmd_line;
		_status.clear();
		
		if (run_in_background)
		{
			try {
				std::thread background_wait = std::thread([this]()
					{
						pid_t pid = ::waitpid(_pid, &_exit_status, 0);
						std::cout << " process " << _pid << " finished with status " << _exit_status << " and PID " << pid << std::endl;					
					});
				background_wait.detach();
			} catch (const std::exception& ex) {
				std::cerr << "Exception: " << ex.what() << std::endl;
			}
		} // if (run_in_background)	
	}
}

void Process::interrupt() noexcept
{
	try {
		signal(SIGINT);
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
