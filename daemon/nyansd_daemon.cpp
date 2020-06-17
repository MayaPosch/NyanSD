/*
	nyansd_daemon.cpp - NyanSD server daemon implementation.
	
	2020/04/27, Maya Posch
*/


#include <nyansd.h>

#include <csignal>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <string>
#include <filesystem>	// C++17
#include <cstdlib>

namespace fs = std::filesystem;

#include "INIReader.h"


// Globals.
std::mutex gMutex;
std::condition_variable gCv;


void signal_handler(int signal) {
	gCv.notify_all();
}


int main() {
	// Add the service records.
	// Get the list of files in the 'services' folder, parsing all 'service' files'
#ifdef _WIN32
	char* pdpath = getenv("ProgramData");
	if (pdpath == nullptr) {
		std::cerr << "Could not find 'ProgramData' environment variable." << std::endl;
		return 1;
	}
	
	std::string fullpath(pdpath);
	fullpath += "/NyanSD/services";
	fs::path path(fullpath);
#else
	fs::path path = "/etc/nyansd/services/"
#endif

	if (!fs::exists(path)) {
		std::cerr << "Services path " << path << " does not exist." << std::endl;
		return 1;
	}
	
	for (fs::directory_entry const &entry : fs::directory_iterator(path)) {
		if (entry.is_regular_file() && entry.path().extension() == ".service") {
			INIReader serviceFile(entry.path().string());	// Read in service file.
			if (serviceFile.ParseError() != 0) {
				std::cerr << "Failed to parse file '" << path << "'." << std::endl;
				return false;
			}
			
			std::set<std::string> sections = serviceFile.Sections();
				
			std::set<std::string>::const_iterator it;
			for (it = sections.cbegin(); it != sections.cend(); ++it) {
				NYSD_service sv;
				sv.service = serviceFile.Get(*it, "name", "");
				if (sv.service.empty()) {
					std::cerr << "Service name was empty. Skipping..." << std::endl;
					continue;
				}
				
				sv.port = serviceFile.GetInteger(*it, "port", 0);
				
				std::string protocol = serviceFile.Get(*it, "protocol", "");
				if (protocol == "tcp") {
					sv.protocol = NYSD_PROTOCOL_TCP;
				}
				else if (protocol == "udp") {
					sv.protocol = NYSD_PROTOCOL_UDP;
				}
				else if (protocol == "all") {
					sv.protocol = NYSD_PROTOCOL_ALL;
				}
				else {
					// Default is to ignore this service.
					std::cerr << "Invalid protocol for service " << sv.service << ". Ignoring..." << std::endl;
					continue;
				}
				
				sv.hostname = serviceFile.Get(*it, "host", "");
				std::string ipv4 = serviceFile.Get(*it, "ipv4", "");
				if (ipv4.empty()) { sv.ipv4 = 0; }
				else { sv.ipv4 = NyanSD::ipv4_stringToUint(ipv4); }
				
				sv.ipv6 = serviceFile.Get(*it, "ipv6", "");
				
				std::cout << "Adding service: " << sv.service << std::endl;
				
				if (!NyanSD::addService(sv)) { 
					std::cerr << "Failed to add service. Aborting..." << std::endl;
					return 1;
				}
			}
		}
	}
	
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
