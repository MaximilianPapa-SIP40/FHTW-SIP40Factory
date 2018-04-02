#include "../Classes/TaskQueue.h"
#include "../Classes/MQTTCommunication.h"
#include "../Classes/IniReader.h"
#include <iostream>
#include <sstream>

void InitStation(std::string topic, std::string stationID);
void InitMOR(std::string topic, std::string morID);
void SendNewTaskToStation(std::string topic, std::string value);
void AddTaskInMORQueue(std::string topic, std::string task);
void Callback_TakeTaskFromMOR(std::string topic, std::string taskID);

TaskQueue 				m_TaskQueue;
MQTTCommunication 		m_mqttComm;
std::vector<uint8_t>	m_MORList;
	
/*
 * Start here
 */
int main (int argc, char **argv)
{	
	INIReader reader("../../SIP40.ini");

    if (reader.ParseError() < 0) {
        std::cout << "Can't load 'SIP40.ini'\n";
        return 1;
    }
	
	std::string mqtt_Hostname = reader.Get("Broker", "MQTT_Hostname", "UNKNOWN");
	int mqtt_Port = reader.GetInteger("Broker", "MQTT_Port", -1);
	
	if(m_mqttComm.Connect(mqtt_Hostname, mqtt_Port))
	{
		//m_mqttComm.Subscribe("SIP40_Factory/MOR_General/GetInitTaskListFromRobot")
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/Station", InitStation);
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/MOR", InitMOR);
		
		while(1)
		{
			// endless loop to receive all incomming datas
		}
	}
	
	return 0;
}

void InitStation(std::string topic, std::string stationID)
{
	m_mqttComm.Publish("SIP40_Factory/" + stationID + "/TaskForStation", "1");
	m_mqttComm.Subscribe("SIP40_Factory/" + stationID + "/NextTaskForMOR", AddTaskInMORQueue);
	m_mqttComm.Subscribe("SIP40_Factory/" + stationID + "/TaskInProgress", SendNewTaskToStation);
    std::cout << "Sent on: SIP40_Factory/" << stationID << "/TaskForStation" << std::endl;
}

void InitMOR(std::string topic, std::string morID)
{
	morID = std::to_string(m_MORList.size());
	m_MORList.push_back(stoi(morID));
	
	m_mqttComm.Publish("SIP40_Factory/Anmeldung/MOR/Identity", morID);
    std::cout << "Gave MOR the identity: " << morID << std::endl;
    
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
    m_mqttComm.Subscribe("SIP40_Factory/MOR_" + morID + "/TakeTask", Callback_TakeTaskFromMOR);
    std::cout << "Sent on: SIP40_Factory/MOR_General/TaskQueue" << std::endl;
}

void SendNewTaskToStation(std::string topic, std::string value)
{
	std::string stationName;
	std::istringstream issTaskQueue(topic);
	getline(issTaskQueue, stationName, '/');
	getline(issTaskQueue, stationName, '/');
			
	if(value == "0")
	{
		m_mqttComm.Publish("SIP40_Factory/" + stationName +"/TaskForStation", "1");
		std::cout << "Send to: " << stationName << std::endl;
	}
}

void AddTaskInMORQueue(std::string topic, std::string task)
{
	m_TaskQueue.InsertNewTaskInQueueFromMQTTString(task);
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
}

void Callback_TakeTaskFromMOR(std::string topic, std::string taskID)
{
	std::string morName;
	std::istringstream issTopic(topic);
	getline(issTopic, morName, '/');
	getline(issTopic, morName, '/');
		
	if(m_TaskQueue.DoesTaskExist(stoi(taskID)))
	{
		std::cout << "Delete Task: " << std::endl;
		m_TaskQueue.PrintTaskWithID(stoi(taskID));
		m_TaskQueue.DeleteTaskWithID(stoi(taskID));
		
		m_mqttComm.Publish("SIP40_Factory/" + morName +"/TakeTaskAnswer", "TaskSuccessfullyTaken");
	}
	else
	{
		std::cout << "Task doesn't exist anymore! Maybe a other MOR took it already!" << std::endl;
		
		m_mqttComm.Publish("SIP40_Factory/" + morName +"/TakeTaskAnswer", "TaskNotTaken");
	}
	
	m_TaskQueue.PrintWholeTaskQueue();
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
}