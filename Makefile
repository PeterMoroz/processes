all: test_print processes

test_print: test_print.cpp
	g++ -std=c++11 test_print.cpp -o test_print
	
processes: main.cpp process_launcher.h process_launcher.cpp
	g++ -std=c++11 main.cpp process_launcher.cpp -lpthread -o processes
	