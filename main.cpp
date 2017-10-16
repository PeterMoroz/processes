#include <iostream>

#include "process_launcher.h"
#include "process_watchdog.h"

int main(int argc, char* argv[])
{
	ProcessLauncher process_launcher;
	process_launcher.add_command("./test_print");
	process_launcher.add_command("./test_print message 4096");
	process_launcher.add_command("./test_print process 8192");
	
	ProcessWatchdog process_watchdog(ProcessRestriction(32768));
	
	process_launcher.add_process_observer(&process_watchdog);
	
	process_watchdog.run();
	process_launcher.run();
	
	return 0;
}