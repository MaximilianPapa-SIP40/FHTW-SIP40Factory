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

#include "../Classes/Broker.h"

/*
 * Start here
 */
int main (int argc, char **argv)
{	
	// Broker Run
	Broker broker("192.168.1.96", 1883);
	broker.Run();
	
	return 0;
}