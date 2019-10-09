#ifndef __A_MQTT_H
#define __A_MQTT_H

extern void MQTTInstallEventHandler();
extern bool MqttPublishMeasuredDataIfNeeded(char * mess);


#endif