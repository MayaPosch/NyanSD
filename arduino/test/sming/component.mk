ifdef MQTT_URL
	USER_CFLAGS += -DMQTT_URL=\"$(MQTT_URL)\" 
endif

# Use LwIP version 2.
ENABLE_CUSTOM_LWIP=2
