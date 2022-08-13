/*
	nyansd_client.cpp - Implementation file for the NyanSD service discovery client library.
	
	Notes:
			- Arduino-specific version of the full-fat version.
			
	2020/04/23, Maya Posch
	2022/08/09, Maya Posch - Adapted for Arduino.
*/


// Uncomment or define DEBUG to enable debug output.
//#define DEBUG 1


#include "nyansd_client.h"

#include <cstring>

//#include <UdpConnection.h>

//#include <iostream>
//#include <map>

#include <cstdlib>

#include <lwip/pbuf.h>
#include <lwip/ip.h>
#include <lwip/udp.h>

/* #include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/NetworkInterface.h>
#include <Poco/Net/DNS.h>
#include <Poco/Net/NetException.h>
#include <Poco/Exception.h> */


// Static variables.
/* std::vector<NYSD_service> NyanSD::services;
std::mutex NyanSD::servicesMutex;
std::atomic<bool> NyanSD::running{false};
std::thread NyanSD::handler;
ByteBauble NyanSD::bb; */

ListNode* NyanSD_client::response = 0;
ListNode* NyanSD_client::last = 0;
int NyanSD_client::count = 0;
bool NyanSD_client::received = false;


struct ResponseStruct {
	char* data;
	uint32_t length;
};


// --- UDP RECEIVE CALLBACK --
// void arg					- User argument - udp_recv `arg` parameter 
// struct udp_pcb* upcb 	- Receiving Protocol Control Block
// struct pbuf* p			- Pointer to Datagram
// const ip_addr_t* addr 	- Address of sender 
// uint16_t port			- Sender port
void NyanSD_client::udp_receive_callback(void* arg, struct udp_pcb* upcb,  struct pbuf* p, 
													ip_addr_t* addr, uint16_t port) { 
							
	// Process datagram here (non-blocking code).
	// Copy data to global buffer, set flag.
	ListNode* node = new ListNode;
	node->data = (char*) malloc(p->len);
	memcpy(node->data, p->payload, p->len);
	node->length = p->len;
	node->next = 0;
	if (response == 0) {
		// First response.
		response = node;
		last = node;
	}
	else {
		// Attach at end.
		last->next = node;
		last = node;
	}
	
	count++;
	received = true;
	
	// Free received pbuf before return.
	pbuf_free(p); 
}


// --- SEND QUERY ---
bool NyanSD_client::sendQuery(uint16_t port, NYSD_query* queries, uint8_t qnum,
										ServiceNode* responses, uint32_t &resnum) {
	if (qnum > 255) {
		//std::cerr << "No more than 255 queries can be send simultaneously." << std::endl;
		return false;
	}
	else if (qnum < 1) {
		//std::cerr << "At least one query must be sent. No query found." << std::endl;
		return false;
	}
				
	// Compose the NYSD message.
	//BBEndianness he = bb.getHostEndian();
	String msg = String("NYANSD");
	//char msg[] = { 'N', 'Y', 'A', 'N', 'S', 'D' };
	uint16_t len = 0;
	uint8_t type = (uint8_t) NYSD_MESSAGE_TYPE_BROADCAST;
	
	String body;
	body += String((char) qnum);
	NYSD_query* qptr = queries;
	for (int i = 0; i < qnum; ++i) {
		body += String("Q");
		uint8_t prot = (uint8_t) qptr->protocol;
		uint8_t qlen = (uint8_t) qptr->length;
		body += (char) prot;
		body += (char) qlen;
		if (qlen > 0) {
			body += qptr->filter;
		}
		
		if (i != (qnum - 1)) {
			++qptr;
		}
	}
	
	len = body.length() + 1;	// Add one byte for the message type.
	//len = bb.toGlobal(len, he);
	msg += String((char) *((char*) &len));
	msg += String((char) *(((char*) &len) + 1));
	msg += (char) type;
	msg += body;

		
	err_t wr_err = ERR_OK;
	int length = msg.length();
	struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	udp_pcb* udpsocket = udp_new();
	//ip_set_option(udpsocket, SOF_BROADCAST);
	udpsocket->so_options |= SOF_BROADCAST;
	
	wr_err = udp_bind(udpsocket, IP_ADDR_ANY, 0);
	wr_err = udp_connect(udpsocket, IP_ADDR_ANY, 10);
	if (wr_err != ERR_OK) { return false; }
	
	memcpy(p->payload, msg.c_str(), length);
	p->len = length;
	p->tot_len = length;
	wr_err = udp_sendto(udpsocket, p, IP_ADDR_BROADCAST, 10);
	if (wr_err != ERR_OK) {
		//wr_err = udp_sendto(udpsocket,p, IP_ADDR_BROADCAST, 10);
		return false;
	}
	
	pbuf_free(p);
	
	// Set up the callback and start receiving.
	response = 0;
	count = 0;
	udp_bind(udpsocket, IP_ADDR_ANY, 0);
	udp_recv(udpsocket, udp_receive_callback, 0);
		
		
	// Wait for the receive callback to be called. Wait for a maximum of 200 ms.
	delay(200);
		
	if (!received) {
		// No responses. Return false.
		return false;
	}
				
	// Close socket as we're done with this interface.
	//udpsocket.close();
	udp_recv(udpsocket, nullptr, nullptr);
	udp_remove(udpsocket);
	
	// Copy parsed responses into the 'responses' list.
	responses = 0;
	for (int i = 0; i < count; ++i) {
		int n = response->length;
		if (n < 8) {
			// Nothing to do. Try next response, if any.
			continue;
		}
		
		// The received data can contain more than one response. Start parsing from the beginning until
		// we are done.
		char* buffer = response->data;
		ServiceNode* pres = responses;
		resnum = 0;
		//int index = 0;
		//while (index < n) {
			int index = 0;
			//std::string signature = std::string(buffer, 6);
			char signature[6];
			memcpy((char*) &signature, buffer, 6);
			index += 6;
			//if (signature != "NYANSD") {
			if (strncmp((char*) &signature, "NYANSD", 6) == 0) {
				//std::cerr << "Signature of message incorrect: " << signature << std::endl;
				return false;
			}
			
			//len = *((uint16_t*) &buffer[index]);
			memcpy(&len, (buffer + index), 2);
			//len = bb.toHost(len, BB_LE);
			index += 2;
			
			//if (len > buffers[i].length - (index)) {
			if (len > (response->length - index)) {
				//std::cerr << "Insufficient data in buffer to finish parsing message: " << len << "/" 
					//		<< (buffers[i].length - (index + 6)) << std::endl;
				return false;
			}
			
#ifdef DEBUG
			//std::cout << "Found message with length: " << len << std::endl;
#endif
			
			type = *((uint8_t*) (buffer + index++));
			
#ifdef DEBUG
			//std::cout << "Message type: " << (uint16_t) type << std::endl;
#endif
			
			if (type != NYSD_MESSAGE_TYPE_RESPONSE) {
				//std::cerr << "Not a response message type. Skipping..." << std::endl;
				continue;
			}
			
			// Number of services in this response.
			uint8_t rnum = *((uint8_t*) buffer + index++);
#ifdef DEBUG
			//std::cout << "Response count: " << (uint16_t) rnum << std::endl;
#endif
			
			// Service sections.
			for (int i = 0; i < rnum; ++i) {
				if (buffer[index] != 'S') {
					//std::cerr << "Invalid service section signature. Aborting parsing." << std::endl;
					return false;
				}
				
				index++;
				uint32_t ipv4;
				memcpy(&ipv4, (buffer + index), 4);
				index += 4;
				
				uint8_t ipv6len = *((uint8_t*) (buffer + index));
				index++;
				
#ifdef DEBUG
				//std::cout << "IPv6 string with length: " << (uint16_t) ipv6len << std::endl;
#endif
				
				char ipv6[ipv6len];
				memcpy(&ipv6, buffer + index, ipv6len);
				index += ipv6len;
				
				uint16_t hostlen;
				memcpy(&hostlen, (buffer + index), 2);
				index += 2;
				
				char hostname[hostlen];
				memcpy(&hostname, buffer + index, hostlen);
				index += hostlen;
				
				uint16_t port;
				memcpy(&port, (buffer + index), 2);
				index += 2;
				
				uint8_t prot = *((uint8_t*) (buffer + index));
				index++;
				
				uint16_t snlen;
				memcpy(&snlen, (buffer + index), 2);
				index += 2;
				
				char svname[snlen];
				memcpy(&svname, buffer + index, snlen);
				index += snlen;
				
#ifdef DEBUG
				//std::cout << "Adding service with name: " << svname << std::endl;
#endif
				
				ServiceNode* sn = new ServiceNode;
				sn->next = 0;
				NYSD_service sv;
				sv.ipv4 = ipv4;
				sv.ipv6 = ipv6;
				sv.port = port;
				sv.hostname = hostname;
				sv.service = svname;
				if (prot == NYSD_PROTOCOL_ALL) {
					sv.protocol = NYSD_PROTOCOL_ALL;
				}
				else if (prot == NYSD_PROTOCOL_TCP) {
					sv.protocol = NYSD_PROTOCOL_TCP;
				}
				else if (prot == NYSD_PROTOCOL_UDP) {
					sv.protocol = NYSD_PROTOCOL_UDP;
				}
				
				sn->service = sv;
				if (responses == 0) {
					responses = sn;
					pres = responses;
				}
				else {
					pres->next = sn;
					pres = sn;
				}
				
				resnum++;
			}
			
#ifdef DEBUG
			//std::cout << "Buffer: " << index << "/" << n << std::endl;
#endif
		//}
		
		// Delete previously allocated data.
		free(buffer);
		
		// Prepare for next loop: move to next list node, delete old node.
		ListNode* old = response;
		response = response->next;
		delete old;
	}
	
	return true;
}


// -- IPv4
String NyanSD_client::ipv4_uintToString(uint32_t ipv4) {
	String out;
	for (int i = 0; i < 4; ++i) {
		//out += std::to_string(*(((uint8_t*) &ipv4) + i));
		out.concat('0' + *((uint8_t*) &ipv4) + 1);
		if (i < 3) { out += "."; }
	}
	
	return out;
}
