#include <SmingCore.h>
#include <Network/Mqtt/MqttBuffer.h>

#include "nyansd_client.h"


// If you want, you can define WiFi settings globally in Eclipse Environment Variables
/* #ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif */

// For testing purposes, try a few different URL formats
#define MQTT_URL1 "mqtt://test.mosquitto.org:1883"
#define MQTT_URL2 "mqtts://test.mosquitto.org:8883" // (Need ENABLE_SSL)
#define MQTT_URL3 "mqtt://frank:fiddle@192.168.100.107:1883"

#ifdef ENABLE_SSL
#include <ssl/private_key.h>
#include <ssl/cert.h>
#define MQTT_URL MQTT_URL2
#else
#define MQTT_URL MQTT_URL1
#endif


// Forward declarations
void startMqttClient();

MqttClient mqtt;
Timer procTimer;
String mqtt_url;

// Check for MQTT Disconnection
void checkMQTTDisconnect(TcpClient& client, bool flag)
{
	if(flag == true) {
		Serial.println(_F("MQTT Broker Disconnected!!"));
	} else {
		Serial.println(_F("MQTT Broker Unreachable!!"));
	}

	// Restart connection attempt after few seconds
	procTimer.initializeMs(2 * 1000, startMqttClient).start(); // every 2 seconds
}

int onMessageDelivered(MqttClient& client, mqtt_message_t* message)
{
	Serial.printf(_F("Message with id %d and QoS %d was delivered successfully.\n"), message->puback.message_id,
				  message->puback.qos);
	return 0;
}

// Publish our message
void publishMessage()
{
	if(mqtt.getConnectionState() != eTCS_Connected) {
		startMqttClient(); // Auto reconnect
	}

	Serial.print(_F("Let's publish message now. Memory free="));
	Serial.println(system_get_free_heap_size());
	mqtt.publish(F("main/frameworks/sming"), F("Hello friends, from Internet of things :)"));

	mqtt.publish(F("important/frameworks/sming"), F("Request Return Delivery"),
				 MqttClient::getFlags(MQTT_QOS_AT_LEAST_ONCE));
}

// Callback for messages, arrived from MQTT server
int onMessageReceived(MqttClient& client, mqtt_message_t* message)
{
	Serial.print("Received: ");
	Serial.print(MqttBuffer(message->publish.topic_name));
	Serial.print(":\r\n\t"); // Pretify alignment for printing
	Serial.println(MqttBuffer(message->publish.content));
	return 0;
}

// Run MQTT client
void startMqttClient()
{
	procTimer.stop();

	// 1. [Setup]
	if(!mqtt.setWill(F("last/will"), F("The connection from this device is lost:("),
					 MqttClient::getFlags(MQTT_QOS_AT_LEAST_ONCE, MQTT_RETAIN_TRUE))) {
		debugf("Unable to set the last will and testament. Most probably there is not enough memory on the device.");
	}

	mqtt.setEventHandler(MQTT_TYPE_PUBACK, onMessageDelivered);

	mqtt.setConnectedHandler([](MqttClient& client, mqtt_message_t* message) {
		Serial.print(_F("Connected to "));
		Serial.println(client.getRemoteIp());

		// Start publishing message now
		publishMessage();
		
		// Schedule a timer to send messages every 10 seconds
		procTimer.initializeMs(10 * 1000, publishMessage).start();
		return 0;
	});

	mqtt.setCompleteDelegate(checkMQTTDisconnect);
	mqtt.setMessageHandler(onMessageReceived);

#ifdef ENABLE_SSL
	mqtt.setSslInitHandler([](Ssl::Session& session) {
		session.options.verifyLater = true;
		session.keyCert.assign(default_private_key, sizeof(default_private_key), default_certificate,
							   sizeof(default_certificate), nullptr);
	});
#endif

	// 2. [Connect]
	//Url url(MQTT_URL);
	Url url(mqtt_url);
	Serial.print(_F("Connecting to "));
	Serial.println(url);
	mqtt.connect(url, F("esp8266"));
	mqtt.subscribe(F("main/status/#"));
}

void onConnected(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.println(_F("WIFI connected. Starting MQTT client..."));
	
	// TODO: Use NyanSD to fetch the MQTT broker IP & port.
	uint16_t port = 11310;
	NYSD_query query;
	query.protocol = NYSD_PROTOCOL_ALL;
	char filter[] = "mqtt";
	query.filter = (char*) &filter;
	query.length = 4;
	uint8_t qnum = 1;
	ServiceNode* responses = 0;
	uint32_t resnum = 0;
	uint32_t res = NyanSD_client::sendQuery(port, &query, qnum, responses, resnum);
	if (res != 0) {
		// Handle error.
		Serial.println(_F("Failed to query for an MQTT server: ") + String(res));
	}
	
	// Process received responses, we should have just a single MQTT server. Pick the first one
	// regardless.
	if (resnum < 1) {
		// No MQTT server found. Abort connecting.
		Serial.println(_F("Failed to find an MQTT server..."));
		while(1) { };
	}
	
	String ipv4 = NyanSD_client::ipv4_uintToString(responses->service.ipv4);
	
	// Print IP.
	Serial.println(_F("Found MQTT server at: ") + ipv4);
	
	// Build MQTT URL.
	// Format: mqtt://<ipv4>:<port>
	mqtt_url = "mqtt://";
	mqtt_url += ipv4;
	mqtt_url += ":";
	mqtt_url.concat(responses->service.port);
	
	Serial.println(_F("MQTT URL: ") + mqtt_url);
	
	// Clean up NyanSD query resources.
	while (responses->next != 0) {
		ServiceNode* oldn = responses;
		responses = responses->next;
		delete oldn;
	}
	
	delete responses;

	// Run MQTT client
	startMqttClient();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationGotIP(onConnected);
}
