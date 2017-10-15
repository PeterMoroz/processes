#include <iostream>

#include "process_launcher.h"

int main(int argc, char* argv[])
{
	ProcessLauncher process_launcher;
	process_launcher.add_command("./test_print");
	process_launcher.add_command("./test_print message");
	process_launcher.add_command("./test_print process");
	
	process_launcher.run();
	
	return 0;
}