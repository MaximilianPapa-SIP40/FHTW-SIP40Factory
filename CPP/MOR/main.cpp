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

#include "../Classes/MobileRobot.h"
#include "../Classes/FactoryMap.h"

/*
 * Start here
 */
int main (int argc, char **argv)
{	
	FactoryMap factoryMap(5, 5);
	
	/* Description of the Grid-
	1--> The cell is not blocked
	0--> The cell is blocked   
	@ToDo: Automatisch erstellen lassen
	*/
	bool factoryMap_FreeWays[5][5] =
	{
		{ 1, 0, 1, 0, 1 },
		{ 1, 0, 1, 0, 1 },
		{ 1, 1, 1, 1, 1 },
		{ 1, 0, 1, 0, 0 },
		{ 1, 0, 1, 0, 0 }
	};
	
	int factoryMap_IDs[5][5] =
	{
		{ 10001, 99999, 10000, 99999, 10002 },
		{ 31010, 99999, 31010, 99999, 31010 },
		{ 21110, 30101, 21111, 30101, 21100 },
		{ 31010, 99999, 31010, 99999, 99999 },
		{ 10003, 99999, 10004, 99999, 99999 }
	};
	
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
	
	// MOR Run
	// @todo: make a config file with all important infos for all devices like IP and so on
	MobileRobot mor("192.168.1.96", 1883, factoryMap, "/dev/ttyACM0", 9600);
	mor.Run();
	
	return 0;
}