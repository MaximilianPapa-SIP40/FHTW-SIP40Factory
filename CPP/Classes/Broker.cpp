#include "Broker.h"
#include <iostream>
#include <sstream>

MQTTCommunication 		Broker::m_mqttComm;
TaskQueue 				Broker::m_TaskQueue;
std::vector<uint8_t>	Broker::m_MORList;

Broker::Broker(const std::string mqttHostname, const int mqttPort)
{
	
}

bool Broker::Run()
{
	if(m_mqttComm.Connect("192.168.1.25", 1883))
	{
		//m_mqttComm.Subscribe("SIP40_Factory/MOR_General/GetInitTaskListFromRobot")
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/Station", InitStation);
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/MOR", InitMOR);
		
		while(1)
		{
			
		}
	}
}

bool Broker::InitStation(std::string topic, std::string stationID)
{
	m_mqttComm.Publish("SIP40_Factory/" + stationID + "/TaskForStation", "1");
	m_mqttComm.Subscribe("SIP40_Factory/" + stationID + "/NextTaskForMOR", AddTaskInMORQueue);
	m_mqttComm.Subscribe("SIP40_Factory/" + stationID + "/TaskInProgress", SendNewTaskToStation);
    std::cout << "Sent on: SIP40_Factory/" << stationID << "/TaskInProgress" << std::endl;
}

bool Broker::InitMOR(std::string topic, std::string morID)
{
	morID = m_MORList.size();
	m_MORList[stoi(morID)] = stoi(morID);
	
	m_mqttComm.Publish("SIP40_Factory/Anmeldung/MOR/Identity", morID);
    std::cout << "Gave MOR the identity: " << morID << std::endl;
    
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
    m_mqttComm.Subscribe("SIP40_Factory/MOR_" + morID + "/TakeTask", RemoveTaskFromMORQueue);
    std::cout << "Sent on: SIP40_Factory/MOR_General/TaskQueue" << std::endl;
}

bool Broker::SendNewTaskToStation(std::string topic, std::string value)
{
	std::string stationName;
	std::istringstream issTaskQueue(topic);
	getline(issTaskQueue, stationName, ';');
	getline(issTaskQueue, stationName, ';');
			
	if(value == "0")
	{
		m_mqttComm.Publish("SIP40_Factory/" + stationName +"/TaskForStation", "1");
		std::cout << "Send to: " << stationName << std::endl;
	}
}

bool Broker::AddTaskInMORQueue(std::string topic, std::string task)
{
	m_TaskQueue.InsertNewTaskInQueueFromMQTTString(task);
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
}

bool Broker::RemoveTaskFromMORQueue(std::string topic, std::string taskID)
{
	std::cout << "Delete Task: " << std::endl;
	m_TaskQueue.PrintTaskWithID(stoi(taskID));
	m_TaskQueue.DeleteTaskWithID(stoi(taskID));
	m_TaskQueue.PrintWholeTaskQueue();
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
}