/*
	nyansd_server.cpp - NyanSD server reference implementation.
	
	2020/04/27, Maya Posch
*/


#include "../src/nyansd.h"

#include <csignal>
#include <mutex>
#include <condition_variable>
#include <iostream>


// Globals.
std::mutex gMutex;
std::condition_variable gCv;


void signal_handler(int signal) {
	gCv.notify_all();
}


int main() {
	// Add the service records.
	NYSD_service sv;
	sv.port = 11310;
	sv.protocol = NYSD_PROTOCOL_TCP;
	sv.service = "HelloService";
	
	NyanSD::addService(sv);
	
	std::cout << "Added services." << std::endl;
	
	// Set signal handler to allow ending the server with 
	std::signal(SIGINT, signal_handler);
	
	// Start the server.
	std::cout << "Starting server..." << std::endl;
	NyanSD::startListener(11310);
	
	// Wait for signal.
	std::cout << "Running... terminate with SIGINT (Ctrl+c)." << std::endl;
	std::unique_lock<std::mutex> lk(gMutex);
	gCv.wait(lk);
	
	// Stop the server.
	std::cout << "Stopping server..." << std::endl;
	NyanSD::stopListener();
	
	std::cout << "All done." << std::endl;
	
	return 0;
}
