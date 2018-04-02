#include "../Classes/TaskQueue.h"
#include "../Classes/MQTTCommunication.h"
#include "../Classes/IniReader.h"
#include <iostream>
#include <sstream>

/*
 * Prototypes of the callback functions for the broker
 */
void Callback_StationRegistration(std::string topic, std::string stationID);
void Callback_MORRegistration(std::string topic, std::string morID);
void Callback_StationTaskInProgress(std::string topic, std::string value);
void Callback_NextTaskForMor(std::string topic, std::string task);
void Callback_TakeTaskFromMOR(std::string topic, std::string taskID);

TaskQueue 				m_TaskQueue;
MQTTCommunication 		m_mqttComm;
std::vector<uint8_t>	m_MORList;
	
/*
 * Entry point of the program
 */
int main (int argc, char **argv)
{	
	
	// Read the Ini-File
	INIReader reader("../../SIP40.ini");
	if (reader.ParseError() < 0) {
		std::cout << "Can't load 'SIP40.ini'\n";
		return 1;
	}
	std::string mqtt_Hostname 	= reader.Get("Broker", "MQTT_Hostname", "UNKNOWN");
	int 		mqtt_Port 		= reader.GetInteger("Broker", "MQTT_Port", -1);
	
	if(m_mqttComm.Connect(mqtt_Hostname, mqtt_Port))
	{
		//m_mqttComm.Subscribe("SIP40_Factory/MOR_General/GetInitTaskListFromRobot")
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/Station", Callback_StationRegistration);
		m_mqttComm.Subscribe("SIP40_Factory/Anmeldung/MOR", Callback_MORRegistration);
		
		while(1)
		{
			// endless loop to receive all incomming datas
		}
	}
	
	return 0;
}

/**
 * Callback function, if a station registers
 * 
 * @detail		Send the station a task and subscribe to important topics
 * @param [in]	topic 		- Topic of the received message
 * @param [in]	stationID 	- Payload of the received message
 * @return 		Nothing
 */
void Callback_StationRegistration(std::string topic, std::string stationID)
{
	m_mqttComm.Publish("SIP40_Factory/" + stationID + "/TaskForStation", "1");
	m_mqttComm.Subscribe("SIP40_Factory/" + stationID + "/NextTaskForMOR", Callback_NextTaskForMor);
	m_mqttComm.Subscribe("SIP40_Factory/" + stationID + "/TaskInProgress", Callback_StationTaskInProgress);
    std::cout << "Sent on: SIP40_Factory/" << stationID << "/TaskForStation" << std::endl;
}

/**
 * Callback function, if a mobile robot registers
 * 
 * @detail		Create and save a unique identity for the new robot
 * 				Send the mor this identity and the whole tasklist
				Subscribe to important topics from this mobile robot
 * @param [in]	topic 		- Topic of the received message
 * @param [in]	morID 		- Payload of the received message
 * @return 		Nothing
 */
void Callback_MORRegistration(std::string topic, std::string morID)
{
	morID = std::to_string(m_MORList.size());
	m_MORList.push_back(stoi(morID));
	
	m_mqttComm.Publish("SIP40_Factory/Anmeldung/MOR/Identity", morID);
    std::cout << "Gave MOR the identity: " << morID << std::endl;
    
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
    m_mqttComm.Subscribe("SIP40_Factory/MOR_" + morID + "/TakeTask", Callback_TakeTaskFromMOR);
    std::cout << "Sent on: SIP40_Factory/MOR_General/TaskQueue" << std::endl;
}

/**
 * Callback function, if a station changes his task progress
 * 
 * @detail		Parse the station name out of the topic
 * 				If the station finished a task, send it a new one
 * @param [in]	topic 		- Topic of the received message
 * @param [in]	value 		- Payload of the received message (0 - task finished)
 * @return 		Nothing
 */
void Callback_StationTaskInProgress(std::string topic, std::string value)
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

/**
 * Callback function, if a station has a new task for the mobile robot
 * 
 * @detail		Insert this new task in the global task queue
 * 				Send the new current task queue to all mobile robots
 * @param [in]	topic 		- Topic of the received message
 * @param [in]	value 		- Payload of the received message (new Task)
 * @return 		Nothing
 */
void Callback_NextTaskForMor(std::string topic, std::string task)
{
	m_TaskQueue.InsertNewTaskInQueueFromMQTTString(task);
	m_mqttComm.Publish("SIP40_Factory/MOR_General/TaskQueue", m_TaskQueue.GetMQTTStringOfTaskQueue());
}

/**
 * Callback function, if a mobile robot chooses a task to do
 * 
 * @detail		Extract the mobile robot identification out of the topic
 * 				Check if the requested task exists or if another mobile robot took it already
 * 				Response to the right mobile robot the state of the requested tast
 * 				Delete the task if it is available and the mobile robot took it
 * 				Send all mobile robots the new updated task queue
 * @param [in]	topic 		- Topic of the received message
 * @param [in]	value 		- Payload of the received message (TaskID of the requested task)
 * @return 		Nothing
 */
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