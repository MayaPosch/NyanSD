/*
	nyansd_server.cpp - NyanSD server reference implementation.
	
	2020/04/27, Maya Posch
*/


#include "../src/nyansd.h"

#include <iostream>


bool isDuplicate(std::vector<NYSD_service> &uniques, NYSD_service &rm) {
	for (uint32_t j = 0; j < uniques.size(); ++j) {
		if (uniques[j].ipv4 == rm.ipv4 &&
			uniques[j].ipv6 == rm.ipv6 &&
			uniques[j].hostname == rm.hostname &&
			uniques[j].port == rm.port &&
			uniques[j].service == rm.service &&
			uniques[j].protocol == rm.protocol) {
			return true;
		}
	}
	
	return false;
}


int main() {
	// Send query via NyanSD.
	std::vector<NYSD_query> queries;
	std::vector<NYSD_service> responses;
	
	NYSD_query query;
	query.protocol = NYSD_PROTOCOL_ALL;
	queries.push_back(query);
	
	std::cout << "Sending query..." << std::endl;
	NyanSD::sendQuery(11310, queries, responses);
	
	std::cout << "Received " << responses.size() << " responses." << std::endl;
	
	// Filter out duplicates.
	std::vector<NYSD_service> uniques;
	for (int i = 0; i < responses.size(); ++i) {
		if (isDuplicate(uniques, responses[i])) {
			std::cout << "Skipping duplicate entry for '" << responses[i].service << "'" << std::endl;
			continue;
		}
		
		uniques.push_back(responses[i]);
	}
	
	// Print out responses.
	for (uint32_t i = 0; i < uniques.size(); ++i) {
		std::cout << "Got service " << uniques[i].service << " on host " << uniques[i].hostname
					<< ", IPv4 " << NyanSD::ipv4_uintToString(uniques[i].ipv4) 
					<< ", IPv6 " << uniques[i].ipv6
					<< ", port " << (uint16_t) uniques[i].port;
		if (uniques[i].protocol == NYSD_PROTOCOL_TCP) {
			std::cout << " (TCP)" << std::endl;
		}
		else if (uniques[i].protocol == NYSD_PROTOCOL_UDP) {
			std::cout << " (UDP)" << std::endl;
		}
		else {
			std::cout << " (ALL)" << std::endl;
		}
	}
	
	std::cout << "All done." << std::endl;
	
	return 0;
}
