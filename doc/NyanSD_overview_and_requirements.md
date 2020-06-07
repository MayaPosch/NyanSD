# NyanSD #

**Overview and requirements**
---

## 0. The project ##

NyanSD ('Nyanko Service Discovery') is intended to be a simple service discovery protocol, in the vein of mDNS and DNS-SD. Its main differences are a focus on minimalism and simplicity, making it easy to implement and port, with more flexibility in whether the SD service is implemented on a system level or application level.

## 1. Feature set ##

The basic feature set of NyanSD is as follows:

1. A server component, capable of receiving NyanSD discovery packets and responding to them.
2. A client component, capable of sending out NyanSD discovery packets and receiving responses.


## 2. NyanSD protocol ##

The  core of the NyanSD protocol (NYSD protocol, or NYSDP) is a simple UDP-based broadcast. Any NYSD servers that receive the broadcast packet can inspect it and respond if necessary. This means that two message types exist, the broadcast message and the response message.

**2.1. Header**

The header of an NYSDP message always looks like the following:

<pre>
uint8(6)	NYANSD
uint16		Message length in bytes following this length indicator.
uint8		Message type
</pre>


*2.1.2. Message type*

The following values exist for the message type property:

<pre>
NYSD_MESSAGE_TYPE_BROADCAST	= 0x01
NYSD_MESSAGE_TYPE_RESPONSE 	= 0x02
</pre>

**2.2. Broadcast**

A broadcast message has the following structure:

<pre>
&lt;header&gt;
uint8	  Queries contained in this message. Value '0' if wildcard query.
&lt;query&gt;	(optional) One or more query entries (see section). 
</pre>

*2.2.1. Query*

The query section is used to create a specific query for a service. It specifies the service that should respond to the broadcast message, using both the requested protocol and a filter string for the service name.

<pre>
uint8		Q
uint8		Protocol this query pertains to.
uint8		Length of query string.
uint8(*)	Query string.
</pre>

*2.2.2. Query protocol*

The query protocol property can have the following values:

<pre>
NYSD_PROTOCOL_ALL = 0x0
NYSD_PROTOCOL_TCP = 0x1
NYSD_PROTOCOL_UDP = 0x2
</pre>

*2.2.3. Query string*

The NYSD query string can be empty (length of 0), in which case it indicates that all services which run on the specificied query protocol should response. Otherwise, the service name should be compared against the query string for a (partial) match. Sub-string matching is allowed.

**2.3. Response**

A Response message has the following structure:

<pre>
&lt;header&gt;
uint8		  Services contained in this message. Must be '1' or more.
&lt;service&gt;		One or more query entries (see section). 
</pre>

*2.3.1. Service*

A service section has the following structure:

<pre>
uint8		S
uint8(4)	IPv4 address of service host.
uint8(39)	IPv6 address of service host.
uint16		Length of hostname.
uint8(*)	Hostname.
uint16		Port of the service.
uint8		Protocol used by the service.
uint16		Length of service name.
uint8(*)	Service name.
</pre>

How many service records will be returned by a single NyanSD server depends both on how many services are registered with it, and how many match the query or queries in the broadcast message.

## 3. Reference implementation ##

The NyanSD reference implementation of NYSD is based around the following technologies:

* C++
* LibPOCO (networking support)

