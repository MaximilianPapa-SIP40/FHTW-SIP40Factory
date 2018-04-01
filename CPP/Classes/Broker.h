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
	
	static void InitStation(std::string topic, std::string stationID);
	static void InitMOR(std::string topic, std::string morID);
	static void SendNewTaskToStation(std::string topic, std::string value);
	static void AddTaskInMORQueue(std::string topic, std::string task);
	static void Callback_TakeTaskFromMOR(std::string topic, std::string taskID);
};