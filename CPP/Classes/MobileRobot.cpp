#include "MobileRobot.h"
#include "AStarAlgorithm.h"
#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>
#include <iostream>
#include <sstream>

uint16_t 			MobileRobot::m_Identity = -1;
std::string 		MobileRobot::RobotInStationTopic = "";
bool 				MobileRobot::stationHasFinished = false;
TaskQueue 			MobileRobot::m_TaskQueue;
MQTTCommunication 	MobileRobot::m_mqttComm;
FactoryMap*			MobileRobot::m_FactoryMap = NULL;
bool 				MobileRobot::m_TaskAnswerArrived = false;
bool				MobileRobot::m_TaskSuccessfullyTaken = false;
bool 				MobileRobot::m_PathAnswerFromServer = false;

MobileRobot::MobileRobot(const std::string mqttHostname, const int mqttPort, const std::string serialDevice, const int serialBaud) 
	: m_SerialDevice(serialDevice)
	, m_SerialBaud(serialBaud)
	, m_StateMachineState(0)
	, m_ActualPositionStationID(10004)
	, m_RobotDrivesToStart(false)
	, stationToggle(false)
	, stateMachineState(0)
	, lastBatteryInformationsSend(0)
	, m_mqttHostname(mqttHostname)
	, m_mqttPort(mqttPort)
{
	
}

bool MobileRobot::Run()
{
	if(!InitializeSerialConnection())
	{
		return false;
	}
	
	if(m_mqttComm.Connect(m_mqttHostname, m_mqttPort))
	{
		m_mqttComm.Subscribe("SIP40_Factory/MOR_General/TaskQueue", UpdateMORTaskQueue);
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/MOR/Identity", InitMOR);
		m_mqttComm.Subscribe("SIP40_Factory/Factory/BookPath", Callback_BookPathInFactory);
		m_mqttComm.Subscribe("SIP40_Factory/Factory/FreePath", Callback_FreePathInFactory);
		
		m_mqttComm.Publish("SIP40_Factory/Anmeldung/MOR", "1");
		
		while(1)
		{				
			switch (stateMachineState)
			{
				case 0:
				{
					if (!m_TaskQueue.IsEmpty()) 
					{
						m_ActualTask = m_TaskQueue.GetHighestPriorityTask();
						TakeTask(m_ActualTask.GetTaskID());
						stateMachineState++;
					}
					break;
				}
				case 1:
				{
					if(m_TaskAnswerArrived)
					{
						if(m_TaskSuccessfullyTaken)
						{
							std::cout << "MOR_" + std::to_string(m_Identity) + " takes the task with id: " + std::to_string(m_ActualTask.GetTaskID()) + " - ";
							m_TaskQueue.PrintWayFromTaskWithID(m_ActualTask.GetTaskID());
							stateMachineState++;
							
							// Has to be in both conditions, because of the receive thread --> Better: Try with Mutex
							m_TaskAnswerArrived = false;
							m_TaskSuccessfullyTaken = false;
						}
						else
						{
							std::cout << "Task doesn't exist anymore! Maybe a other MOR took it already!" << std::endl;
							std::cout << "Wait to took another Task!" << std::endl;
							stateMachineState--;
							
							// Has to be in both conditions, because of the receive thread  --> Better: Try with Mutex
							m_TaskAnswerArrived = false;
							m_TaskSuccessfullyTaken = false;
						}
					}
					
					break;
				}
				case 2:
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
				case 3:
				{
					DriveAlongPath(path);
					stateMachineState++;
					break;
				}
				case 4:
				{
					RobotInStationTopic = "SIP40_Factory/" + m_FactoryMap->GiveStationNameFromID(m_ActualTask.GetStartStationID()) + "/RobotInStation";
					std::cout << RobotInStationTopic << std::endl;
					
					// Publish the message to the topic
					if (m_mqttComm.Publish(RobotInStationTopic, "1"))
					{
						fprintf (stderr, "Can't publish to Mosquitto server\n");
						exit (-1);
					}
					
					// Subscribe on topic
					m_mqttComm.Subscribe(RobotInStationTopic, RobotInStation);
					
					std::cout << "Robot in Station!" << std::endl;
					
					stateMachineState++;
					break;
				}
				case 5:
				{
					if(stationHasFinished)
					{
						stationHasFinished = false;
						std::cout << "Robot ready for destination drive!" << std::endl;
						stateMachineState++;
					}
					break;
				}
				case 6:
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
				case 7:
				{
					DriveAlongPath(path);
					
					RobotInStationTopic = "SIP40_Factory/" + m_FactoryMap->GiveStationNameFromID(m_ActualTask.GetDestStationID()) + "/RobotInStation";
					std::cout << RobotInStationTopic << std::endl;
						
					// Publish the message to the topic
					if (m_mqttComm.Publish(RobotInStationTopic, "1"))
					{
						fprintf (stderr, "Can't publish to Mosquitto server\n");
						exit (-1);
					}
					
					// Subscribe on topic
					m_mqttComm.Subscribe(RobotInStationTopic, RobotInStation);
					
					std::cout << "Robot in Station!" << std::endl;
					
					stateMachineState++;
					
					break;
				}
				case 8:
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
	
	m_mqttComm.Subscribe("SIP40_Factory/MOR_" + std::to_string(m_Identity) + "/PathAnswerFromServer", Callback_PathAnswerFromServer);
	m_mqttComm.Subscribe("SIP40_Factory/MOR_" + std::to_string(m_Identity) + "/TakeTaskAnswer", Callback_TakeTask);	
	m_mqttComm.Unsubscribe("SIP40_Factory/Anmeldung/MOR/Identity");
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

void MobileRobot::Callback_PathAnswerFromServer(std::string topic, std::string value)
{
	if(value == "RequestAccepted")
	{
		m_PathAnswerFromServer = true;
	}
}

void MobileRobot::Callback_BookPathInFactory(std::string topic, std::string path)
{
	std::cout << "Empfangen: " << path << std::endl;
	
	std::string singleField;
	std::istringstream issPath(path);
	while (getline(issPath, singleField, '-'))
	{
		std::string xPos;
		std::string yPos;
		std::istringstream issSingleField(singleField);
		getline(issSingleField, xPos, ':');
		getline(issSingleField, yPos, ':');
			
		m_FactoryMap->BookField(std::stoi(xPos), std::stoi(yPos));
	}
}

void MobileRobot::Callback_FreePathInFactory(std::string topic, std::string path)
{
	std::string singleField;
	std::istringstream issPath(path);
	while (getline(issPath, singleField, '-'))
	{
		std::string xPos;
		std::string yPos;
		std::istringstream issSingleField(singleField);
		getline(issSingleField, xPos, ':');
		getline(issSingleField, yPos, ':');
			
		m_FactoryMap->FreeField(std::stoi(xPos), std::stoi(yPos));
	}
}

void MobileRobot::Callback_TakeTask(std::string topic, std::string answer)
{
	if(answer.compare("TaskSuccessfullyTaken") == 0)
	{
		m_TaskAnswerArrived = true;
		m_TaskSuccessfullyTaken = true;
	}
	else if(answer.compare("TaskNotTaken") == 0)
	{
		m_TaskAnswerArrived = true;
		m_TaskSuccessfullyTaken = false;
	}
}
    
void MobileRobot::TakeTask(uint64_t taskID)
{
	if (m_mqttComm.Publish(std::string("SIP40_Factory/MOR_" + std::to_string(m_Identity) + "/TakeTask"), std::to_string(taskID)))
	{
		fprintf (stderr, "Can't publish to Mosquitto server\n");
		exit (-1);
	}
}

bool MobileRobot::DriveAlongPath(const std::vector<std::pair<int, int>> path) const
{
	BookPath(path);
	
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
	
	FreePath(path);
}

void MobileRobot::BookPath(const std::vector<std::pair<int, int>> path) const
{
	std::string pathString;
	
	std::cout << "Pfad blockieren: ";
	
	for(int index = 0; index < (path.size() - 1); index++)
	{
		pathString.append(std::to_string(path[index].first));
		pathString.append(":");
		pathString.append(std::to_string(path[index].second));
		
		if(index < (path.size() - 2))
		{
			pathString.append("-");
		}
	}
	
	std::cout << pathString << std::endl;
	
	m_mqttComm.Publish("SIP40_Factory/MOR_" + std::to_string(m_Identity) + "/BookPathRequest", pathString);
	
	// wait till path is booked on server
	while(!m_PathAnswerFromServer);
	m_PathAnswerFromServer = false;
}

void MobileRobot::FreePath(const std::vector<std::pair<int, int>> path) const
{
	std::string pathString;
	
	std::cout << "Pfad freigeben: ";
	
	for(int index = 1; index < path.size(); index++)
	{
		pathString.append(std::to_string(path[index].first));
		pathString.append(":");
		pathString.append(std::to_string(path[index].second));
		
		if(index < (path.size() - 1))
		{
			pathString.append("-");
		}
	}
	
	std::cout << pathString << std::endl;
	
	m_mqttComm.Publish("SIP40_Factory/Factory/FreePath", pathString);
}