/*
	nyansd_server.cpp - NyanSD server reference implementation.
	
	2020/04/27, Maya Posch
*/


#include "../src/nyansd.h"

#include <iostream>


int main() {
	// Send query via NyanSD.
	std::vector<NYSD_query> queries;
	std::vector<NYSD_service> responses;
	
	NYSD_query query;
	query.protocol = NYSD_PROTOCOL_ALL;
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
