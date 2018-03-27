#include "MobileRobot.h"
#include "AStarAlgorithm.h"
#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>
#include <iostream>

uint16_t 			MobileRobot::m_Identity = -1;
std::string 		MobileRobot::RobotInStationTopic = "";
bool 				MobileRobot::stationHasFinished = false;
TaskQueue 			MobileRobot::m_TaskQueue;
MQTTCommunication 	MobileRobot::m_mqttComm;

MobileRobot::MobileRobot(const std::string mqttHostname, const int mqttPort, FactoryMap& map, const std::string serialDevice, const int serialBaud) 
	: m_SerialDevice(serialDevice)
	, m_SerialBaud(serialBaud)
	, m_FactoryMap(&map)
	, m_StateMachineState(0)
	, m_ActualPositionStationID(10004)
	, m_RobotDrivesToStart(false)
{
	
}

bool MobileRobot::Run()
{
	if(!InitializeSerialConnection())
	{
		return false;
	}
	
	if(m_mqttComm.Connect("192.168.1.96", 1883))
	{
		m_mqttComm.Subscribe("SIP40_Factory/MOR_General/TaskQueue", UpdateMORTaskQueue);
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/MOR/Identity", InitMOR);
		
		m_mqttComm.Publish("SIP40_Factory/Anmeldung/MOR", "1");
		
		while(1)
		{				
			switch (stateMachineState)
			{
				case 0:
					if (!m_TaskQueue.IsEmpty()) 
					{
						m_ActualTask = m_TaskQueue.GetHighestPriorityTask();
						TakeTask(m_ActualTask.GetTaskID());
						stateMachineState++;
					}
					break;
				case 1:
				{
					if(m_ActualPositionStationID != m_ActualTask.GetStartStationID())
					{
						// Start Station
						Pair src = m_FactoryMap->FindIDField(m_ActualPositionStationID);
						
						// Destination Station
						Pair dest = m_FactoryMap->FindIDField(m_ActualTask.GetStartStationID());
						m_ActualPositionStationID = m_ActualTask.GetStartStationID();

						AStarAlgorithm aStar(m_FactoryMap);
						path = aStar.AStarSearch(src, dest);
						
						m_RobotDrivesToStart = true;
					}
					else
					{
						stateMachineState++;
					}
					stateMachineState++;
					break;
				}
				case 2:
				{
					std::stack<std::string> commandPathToRobot = m_FactoryMap->CreateCommandsForRobot(path);
					std::cout << "Commands: " << std::endl;
					while (!commandPathToRobot.empty())
					{
						std::string command = commandPathToRobot.top();
						commandPathToRobot.pop();
						std::cout << " --> " << command;
						
						serialPuts(m_SerialFileDescriptor, command.c_str());
						
						int dataAvailable;
						while(1)
						{ 
							dataAvailable = serialDataAvail(m_SerialFileDescriptor);
							if (dataAvailable > 0)
							{
								printf(" (Received: %i Character)\n", dataAvailable);
								
								if(serialGetchar(m_SerialFileDescriptor))
								{
									break;
								}
							}
						}
					}
					
					stateMachineState++;
					
					break;
				}
				case 3:
				{
					RobotInStationTopic = "SIP40_Factory/" + m_FactoryMap->GiveStationNameFromID(m_ActualTask.GetStartStationID()) + "/RobotInStation";
					std::cout << RobotInStationTopic << std::endl;
					
					// Subscribe on topic
					m_mqttComm.Subscribe(RobotInStationTopic, RobotInStation);
					
					// Publish the message to the topic
					if (m_mqttComm.Publish(RobotInStationTopic, "1"))
					{
						fprintf (stderr, "Can't publish to Mosquitto server\n");
						exit (-1);
					}
					
					std::cout << "Robot in Station!" << std::endl;
					
					stateMachineState++;
					break;
				}
				case 4:
				{
					if(stationHasFinished)
					{
						stationHasFinished = false;
						std::cout << "Robot ready for destination drive!" << std::endl;
						stateMachineState++;
					}
					break;
				}
				case 5:
				{
					// Start Station
					Pair src = m_FactoryMap->FindIDField(m_ActualTask.GetStartStationID());

					// Destination Station 
					Pair dest = m_FactoryMap->FindIDField(m_ActualTask.GetDestStationID());
					m_ActualPositionStationID = m_ActualTask.GetDestStationID();

					AStarAlgorithm aStar(m_FactoryMap);
					path = aStar.AStarSearch(src, dest);
					
					m_RobotDrivesToStart = false;
					
					stateMachineState++;
					break;
				}
				case 6:
				{
					std::stack<std::string> commandPathToRobot = m_FactoryMap->CreateCommandsForRobot(path);
					std::cout << "Commands: " << std::endl;
					while (!commandPathToRobot.empty())
					{
						std::string command = commandPathToRobot.top();
						commandPathToRobot.pop();
						std::cout << " --> " << command;
						
						serialPuts(m_SerialFileDescriptor, command.c_str());
						
						int dataAvailable;
						while(1)
						{ 
							dataAvailable = serialDataAvail(m_SerialFileDescriptor);
							if (dataAvailable > 0)
							{
								printf(" (Received: %i Character)\n", dataAvailable);
								
								if(serialGetchar(m_SerialFileDescriptor))
								{
									break;
								}
							}
						}
					}
					
					RobotInStationTopic = "SIP40_Factory/" + m_FactoryMap->GiveStationNameFromID(m_ActualTask.GetDestStationID()) + "/RobotInStation";
					std::cout << RobotInStationTopic << std::endl;
						
					// Subscribe on topic
					m_mqttComm.Subscribe(RobotInStationTopic, RobotInStation);
					
					// Publish the message to the topic
					if (m_mqttComm.Publish(RobotInStationTopic, "1"))
					{
						fprintf (stderr, "Can't publish to Mosquitto server\n");
						exit (-1);
					}
					
					std::cout << "Robot in Station!" << std::endl;
					
					stateMachineState++;
					
					break;
				}
				case 7:
				{
					if(stationHasFinished)
					{
						stationHasFinished = false;
						stateMachineState = 0;
						std::cout << "Robot finished Work!" << std::endl;
					}
					break;
				}
				default:
					std::cout << "Error: No allowed state reached!" << std::endl;
			}
		}
	}
}

bool MobileRobot::InitializeSerialConnection()
{
	if (wiringPiSetup () == -1)
	{
		std::cerr << "Unable to start wiringPi: " << strerror(errno) << std::endl;
		return false;
	}
	
	if ((m_SerialFileDescriptor = serialOpen(m_SerialDevice.c_str(), m_SerialBaud)) < 0)
	{
		std::cerr << "Unable to open serial device: " << strerror(errno) << std::endl;
		return false;
	}
	
	serialFlush(m_SerialFileDescriptor);
	
	return true;
}

bool MobileRobot::InitializeMQTTConnection()
{
	
}

void MobileRobot::UpdateMORTaskQueue(std::string topic, std::string taskQueue)
{
	m_TaskQueue.SetTaskQueueFromMQTTString(taskQueue);
    m_TaskQueue.PrintWholeTaskQueue();
}
    
void MobileRobot::InitMOR(std::string topic, std::string identity)
{
	m_Identity = stoi(identity);
    std::cout << "MOR received the identity: " << m_Identity << std::endl;
}

void MobileRobot::RobotInStation(std::string topic, std::string value)
{
	if(stoi(value) == 0)
	{
		stationHasFinished = true;
		
		// Unsubscribe on topic
		m_mqttComm.Unsubscribe(RobotInStationTopic.c_str());
		RobotInStationTopic.clear();
	}
}
    
void MobileRobot::TakeTask(uint64_t taskID)
{
	if (m_mqttComm.Publish(std::string("SIP40_Factory/MOR_" + std::to_string(m_Identity) + "/TakeTask"), std::to_string(taskID)))
	{
		fprintf (stderr, "Can't publish to Mosquitto server\n");
		exit (-1);
	}
	std::cout << "MOR_" + std::to_string(m_Identity) + " takes the task with id: " + std::to_string(taskID) + " - ";
	m_TaskQueue.PrintWayFromTaskWithID(m_ActualTask.GetTaskID());
	m_TaskQueue.DeleteTaskWithID(m_ActualTask.GetTaskID());
}