/*
 * From : 	http://kevinboone.net/mosquitto-test.html
 * and : 	https://gist.github.com/evgeny-boger/8cefa502779f98efaf24
 */
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <fstream>

#include "../Classes/MobileRobot.h"
#include "../Classes/FactoryMap.h"
#include "../Classes/IniReader.h"

/*
 * Start here
 */
int main (int argc, char **argv)
{	
	FactoryMap factoryMap(11, 13);
	
	/* Description of the Grid-
	1--> The cell is not blocked
	0--> The cell is blocked   
	@ToDo: Automatisch erstellen lassen
	*/
	bool factoryMap_FreeWays[11][13] =
	{
		{ 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
		{ 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
		{ 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
		{ 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0 },
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
	};
	
	int factoryMap_IDs[11][13];
	
	std::string factoryMapLine;
	std::ifstream factoryMapFile ("../../FactoryMap.txt");
	if(factoryMapFile.is_open())
	{
		for(int row = 0; getline(factoryMapFile,factoryMapLine); row++)
		{
			std::string mapPosID;
			std::istringstream issMapLine(factoryMapLine);
			for(int column = 0; getline(issMapLine, mapPosID, '-'); column++)
			{
				factoryMap_IDs[row][column] = std::stoi(mapPosID);
			}
		}
		factoryMapFile.close();
	}
	else
	{
		std::cout << "Unable to open FactoryMap-File"; 
	}
	
	for(int row = 0; row < 5; row++) 
	{
		for(int column = 0; column < 5; column++) 
		{
			FactoryMapField field;
			field.fieldIsFree = factoryMap_FreeWays[row][column];
			field.fieldID = factoryMap_IDs[row][column];
			factoryMap.SetInMap(field, row, column);
		}
	}
	
	INIReader reader("../../SIP40.ini");

    if (reader.ParseError() < 0) {
        std::cout << "Can't load 'SIP40.ini'\n";
        return 1;
    }
	
	std::string mqtt_Hostname = reader.Get("Broker", "MQTT_Hostname", "UNKNOWN");
	int mqtt_Port = reader.GetInteger("Broker", "MQTT_Port", -1);
	
	// MOR Run
	MobileRobot::m_FactoryMap = &factoryMap;
	MobileRobot mor(mqtt_Hostname, mqtt_Port, "/dev/ttyACM0", 9600);
	mor.Run();
	
	return 0;
}