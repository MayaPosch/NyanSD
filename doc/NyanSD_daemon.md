# NyanSD Daemon #

The NyanSD (Nyanko Service Discovery) daemon is a service designed to run on any OS to provide service discovery functionality. New service records can be added in the form of single files, with the location depending on the platform the daemon runs on.

## Running ##

The NyanSD daemon is designed to be run as a system service. Service configuration files are provided for:

* Systemd
* OpenRC

These services are installed using the Makefile in the root of the NyanSD project:

`make install-daemon-systemd`

Or for OpenRC:

`make install-daemon-openrc`

## Adding services ##

A service file consists out of a simple file in INI format. It can contain many services. E.g.:

<pre>
[http]
name=httpd
port=80
protocol=tcp
host=http-server
ipv4=192.168.0.42
ipv6=2001:0db8:0000:0000:0000:8a2e:0370:7334

[ssl]
name=ssld
port=22
protocol=tcp

[ssl]
name=ssld
port=22
protocol=udp
</pre>

The above example lists all of the possible properties. Of these, only the service name, the port number and the protocol (TCP, UDP or both) are required. The other properties are obtained from the host which the daemon runs on.

*Note:* IPv6 address have be written out fully, without dropping any zeros or sections.

The usual purpose for adding the optional properties is to specify services which do not run on the local machine, but elsewhere in the network.

**Installing**

Service files are placed in the following location, depending on the OS:

* `/etc/nyansd/services` - Linux, BSD, MacOS and similar.
* `%PROGRAMDATA%\NyanSD\services` - Windows.

## Building ##

From the root of the NyanSD project:

`make daemon`

To install:

`make install-daemon`

