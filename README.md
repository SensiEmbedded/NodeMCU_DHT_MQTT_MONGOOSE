# NodeMCU_DHT_MQTT_MONGOOSE

Application in C using NodeMCU with Mongoose OS, MQTT, DHT.

### Common

The app measure hunidity and temperature from DHC11 sensor and send data to MQTT broker. 
If there is significant change of measred humidity and the last sent value to the broker the app publish new values to the broker.
The value of "significant change" can be set and after that save to non volatile memomory throught mqtt protocol.


### MQTT

	1. The device connect to broker.hivemq.com:1883
	2. Subscribe to topic "askoIn" - topic for commands and setting params to device.
	2.1. Commands send to device:
	2.1.1. {command:"refresh"} - this will force publish measured data
	2.1.2. {command:"reboot"} - this will reboot device
	2.1.3. {command:"SetHumHysteresis",value:"5"} - this command will set the hysteresis for humidity to resend the measured result. 
		If the new value is different from the current value the new value of hysteresis is saved to the system config file mos.yml
	3. Measured data is publish to topic "askoOut"
	3.1. Data will be publish in the following cases:
	3.1.1. Receive command "refresh" see 2.1.1
	3.1.2. The difference between current measered humidity and the last sent is bigger than the hysteresis see 2.1.3.
	3.1.3. The button flash on NodeMCU is pressed
	4. The logic is codded in a_mqtt.c and a_mqtt.h files
	
### DHT

	1. The logic is codded in a_dht.c and a_dht.h files
	2. Discretization period is 1 sec. That means every sec the new data is read from the DHT11.
	3. If data is significantly changed that its publish to mqqt broker.
	
### RPC
		
	1. There is one RPC written for reading the measured data in a_dht.c 
	2. "Temp.Read" is the rpc name  
	

	









 
