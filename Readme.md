# NyanSD #

NyanSD (Nyanko Service Discovery) is a simple yet versatile protocol aimed at providing everything from small network-based applications to entire operating systems with the ability to perform network-based service discovery. While similar to DNS-SD and mDNS, it distinguishes itself by being exceedingly simple to implement 

## Reference implementation ##

This repository contains the C++-based reference implementation of the protocol. It requires a C++11 capable compiler and the LibPoco dependency to be installed.

Using the provided Makefile in the root of the project the entire project (library, client and server) can be built by executing:

`make` 

The library (`libnyansd.so.1` & `libnyansd.a`) can be installed using:

`sudo make install`

At this point the client and server are still rudimentary, but serve to demonstrate the use of the NyanSD reference API. E.g. the client:

<pre>
int main() {
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
	
	return 0;
}
</pre>

The above client composes a simple query to get all services available on the network, using port 11310. This means that all NyanSD listening clients on the network ('servers') that are listening on that port will receive the broadcast message.

The server:

<pre>
int main() {
	// Add the service records.
	NYSD_service sv;
	sv.port = 11310;
	sv.protocol = NYSD_PROTOCOL_TCP;
	sv.service = "HelloService";
	
	NyanSD::addService(sv);
	
	// Start the server.
	NyanSD::startListener(11310);
	
	// Wait for the server to be terminated here.
	
	// Stop the server.
	NyanSD::stopListener();
	
	return 0;
}
</pre>

The server can contain a large number of service records, none of which have to be running on that particular host, but can be elsewhere on the network too. See the protocol notes in the next section for further details.


## Protocol ##

The protocol is described in the project's requirements document: [NyanSD overview and requirements](doc/NyanSD_overview_and_requirements.md "NyanSD overview and requirements").

It's a simple binary protocol, little endian (LE)-based. The reference implementation uses UDP broadcast to find services on the network. 

## Disclaimer ##

This project is provided with no guarantees as to its functioning or fitness for a particular purpose. 