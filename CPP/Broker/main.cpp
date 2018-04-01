#include "../Classes/Broker.h"
#include "../Classes/IniReader.h"
#include <iostream>

/*
 * Start here
 */
int main (int argc, char **argv)
{	
	INIReader reader("../../SIP40.ini");

    if (reader.ParseError() < 0) {
        std::cout << "Can't load 'SIP40.ini'\n";
        return 1;
    }
	
	std::string mqtt_Hostname = reader.Get("Broker", "MQTT_Hostname", "UNKNOWN");
	int mqtt_Port = reader.GetInteger("Broker", "MQTT_Port", -1);
	
	// Broker Run
	Broker broker(mqtt_Hostname, mqtt_Port);
	broker.Run();
	
	return 0;
}