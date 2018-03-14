#include <stdio.h>
#include <string>
#include <iostream>
#include <unistd.h>

#include "../Classes/MQTTCommunication.h"

// In folgender Zeile muss dem Stationsnamen eine Zahl zugewiesen werden
const std::string StationName 			= "Station_1"; // kann auskommentieren, wenn mit dem RPi ein dynamischer Stationsname erstellt wird
const std::string Lager2Station 		= "3-10000>10001";
const std::string Station2Lager 		= "3-10001>10000";
const std::string Station2Verpackung 	= "3-10001>10003";

// MQTT parameter
const std::string SIP40_Anmeldung 		= "SIP40_Factory/Anmeldung/Station";
//const std::string SIP40_Status 		= "SIP40_Factory/" + StationName + "/Status";
const std::string SIP40_TaskForStation 	= "SIP40_Factory/" + StationName + "/TaskForStation";
const std::string SIP40_NextTaskForMOR 	= "SIP40_Factory/" + StationName + "/NextTaskForMOR";
const std::string SIP40_RobotInStation 	= "SIP40_Factory/" + StationName + "/RobotInStation";
const std::string SIP40_TaskInProgress 	= "SIP40_Factory/" + StationName + "/TaskInProgress";

//int Status = 0;
unsigned int state = 0;
unsigned int TaskForStation = 0;
bool TaskInProgress = false;
bool m_RobotInStation = false;

void CallBack_TaskForStation(std::string topic, std::string task);
void CallBack_RobotInStation(std::string topic, std::string task);
bool Work();
void PrintInfos();

int main(int argc, char **argv)
{
	MQTTCommunication mqttComm("192.168.1.25", 1883);
	
	if(mqttComm.Connect())
	{
		// Station subscribe to TaskForStation
		if (mqttComm.Subscribe(SIP40_TaskForStation, CallBack_TaskForStation))
		{
			std::cout << "Subscribed to = " << SIP40_TaskForStation << std::endl;
		}
		// Station subscribe to RobotInStation
		if (mqttComm.Subscribe(SIP40_RobotInStation, CallBack_RobotInStation))
		{
			std::cout << "Subscribed to = " << SIP40_RobotInStation << std::endl;
		}
		// Send the actual station name of current station
		if (mqttComm.Publish(SIP40_Anmeldung, StationName))
		{
			std::cout << "Anmeldung = " << StationName << std::endl;
		}
		
		while(1)
		{
			switch(state)
			  {
				case 0:
				  // Station is free and waits for new Tasks
				  if(TaskInProgress)
				  {
					state++;
				  }
				  break;
				  
				case 1:
				  {
					// Station wants to get materials from the warehouse
					  //CallRobotToDeliverRawMaterials();
					  
					  // send command to MOR: Lager2Station and go to next state
					  bool retVal = mqttComm.Publish(SIP40_NextTaskForMOR, Lager2Station);
					  
					  if (retVal)
					  {
						std::cout << "Sending to " << SIP40_NextTaskForMOR << ": " << Lager2Station << std::endl;
						std::cout << "Waiting for robot..." << std::endl;
					  }
					  state++;
				  }
				  break;

				case 2:
				   // Station waits for the robot
				  if(m_RobotInStation)
				  {
					PrintInfos();
					std::cout << "Robot arrivied!" << std::endl;

					usleep(5000); // unload robot
        
					// send command: Robot can left the station
					if (mqttComm.Publish(SIP40_RobotInStation, "0"))
					{
					  m_RobotInStation = false;
					  std::cout << "Sending to " << SIP40_RobotInStation << ": 0" << std::endl;
					}
					  
					state++;
				  }
				  break;

				case 3:
				  // Station works --> Robot can fullfill another tasks or go to charger
				  if(Work())
				  {
					state++;
				  }
				  break;
				
				case 4:
				  // Station wants to transfer the manufactured material to the package station
				  // send command to MOR: Station2Verpackung and go to next state
				  if (mqttComm.Publish(SIP40_NextTaskForMOR, (Station2Verpackung)))
				  {
					std::cout << "Sending to " << SIP40_NextTaskForMOR << ": " << Station2Verpackung << std::endl;
					std::cout << "Waiting for robot..." << std::endl;
					
				  }
				  state++;
				  break;

				case 5:
				  // Station waits for the robot
				  if(m_RobotInStation)
				  {
					// Send broker that the task has finished
					if (mqttComm.Publish(SIP40_TaskInProgress, "0"))
					{
					  std::cout << "Sending to " << SIP40_TaskInProgress << ": 0" << std::endl;
					}
					
					std::cout << "--- Task finished ---" << std::endl;
					TaskInProgress = false;
					state = 0;
				  }
				  break;
				
				default:
				  break;
			  }
		}
	}
	else 
	{
		std::cerr << "Could not connect to the broker" << std::endl;
	}
	
	return 0;
}

void CallBack_TaskForStation(std::string topic, std::string value)
{
	if(value == "1")
	{
		std::cout << "Station hat eine Aufgabe zugeteilt gekommen!" << std::endl;
		TaskInProgress = true;
	}
}

void CallBack_RobotInStation(std::string topic, std::string value)
{
	if(value == "1")
	{
		m_RobotInStation = true;
	}
}

bool Work()
{
  PrintInfos();
  std::cout << "Working..." << std::endl;
  usleep(10000); // working
        
  PrintInfos();
  std::cout << "Finished work!" << std::endl;

  return true;
}

void PrintInfos()
{
  std::cout << "Task for Station = " << TaskForStation << std::endl;
  std::cout << "Robot in Station = " << m_RobotInStation << std::endl;
  std::cout << "Task in Progress = " << TaskInProgress << std::endl;
}