all: test_print processes

test_print: test_print.cpp
	g++ -std=c++11 test_print.cpp -o test_print
	
processes: main.cpp process.h process.cpp process_observer.h process_watchdog.h process_watchdog.cpp process_supervisor.h process_supervisor.cpp process_launcher.h process_launcher.cpp utils.h utils.cpp
	g++ -std=c++11 main.cpp process.cpp process_watchdog.cpp process_supervisor.cpp process_launcher.cpp utils.cpp -lpthread -o processes
	