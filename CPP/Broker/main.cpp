#include "../Classes/Broker.h"

/*
 * Start here
 */
int main (int argc, char **argv)
{	
	// Broker Run
	Broker broker("192.168.1.25", 1883);
	broker.Run();
	
	return 0;
}