/*
	nyansd_browse.cpp - NyanSD client browser implementation.
	
	2020/04/27, Maya Posch
*/


#include <nyansd.h>

#include <iostream>

#include "sarge.h"


int main() {
	// Parse command line arguments.
	Sarge sarge;
	
	sarge.setUsage("nyansd-browse <options>");
	sarge.setDescription("Browser client for the Nyanko Service Discovery (NyanSD) protocol.");
	sarge.setArgument("h", "help", "Show this help message.", false);
	sarge.setArgument("a", "all", "Show all found services (default).", false);
	
	if (sarge.exists("help")) {
		sarge.printHelp();
	}
	
	NYSD_query query;
	if (sarge.exists("all")) {
		std::cout << "Searching for all NyanSD services on the network..." << std::endl;
	}
	
	query.protocol = NYSD_PROTOCOL_ALL;
	
	std::cout << "Searching for services..." << std::endl;
	
	// Send query via NyanSD.
	std::vector<NYSD_query> queries;
	std::vector<NYSD_service> responses;
	
	queries.push_back(query);
	
	std::cout << "Sending query..." << std::endl;
	
	NyanSD::sendQuery(11310, queries, responses);
	
	// Print out responses.
	std::cout << "Received " << responses.size() << " responses." << std::endl;
	for (int i = 0; i < responses.size(); ++i) {
		std::cout << "Got service " << responses[i].service << " on host " << responses[i].hostname
					<< ", IPv4 " << NyanSD::ipv4_uintToString(responses[i].ipv4) 
					<< ", IPv6 " << responses[i].ipv6
					<< ", port " << (uint16_t) responses[i].port << std::endl;
	}
	
	std::cout << "All done." << std::endl;
	
	return 0;
}
