#pragma once

#include "TaskQueue.h"
#include "MQTTCommunication.h"

class Broker
{
public:
	Broker(const std::string mqttHostname, const int mqttPort);
	
	bool Run();
	
private:
	static TaskQueue 				m_TaskQueue;
	static MQTTCommunication 		m_mqttComm;
	static std::vector<uint8_t>		m_MORList;
	
	static bool InitStation(std::string topic, std::string stationID);
	static bool InitMOR(std::string topic, std::string morID);
	static bool SendNewTaskToStation(std::string topic, std::string value);
	static bool AddTaskInMORQueue(std::string topic, std::string task);
	static bool RemoveTaskFromMORQueue(std::string topic, std::string taskID);
};