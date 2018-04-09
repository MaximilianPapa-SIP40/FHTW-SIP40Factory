#include "../Classes/TaskQueue.h"
#include "../Classes/MQTTCommunication.h"
#include "../Classes/FactoryMap.h"
#include "../Classes/IniReader.h"
#include <iostream>
#include <sstream>
#include <map>

/*
 * Prototypes of the callback functions for the broker
 */
void Callback_StationRegistration(std::string topic, std::string stationID);
void Callback_MORRegistration(std::string topic, std::string morID);
void Callback_StationTaskInProgress(std::string topic, std::string value);
void Callback_NextTaskForMor(std::string topic, std::string task);
void Callback_TakeTaskFromMOR(std::string topic, std::string taskID);
void Callback_BookPathRequest(std::string topic, std::string path);
void Callback_FreePathInFactory(std::string topic, std::string path);

bool TryToBookTempBookPathRequest();
bool BookPath(std::string path);

TaskQueue 							m_TaskQueue;
MQTTCommunication 					m_mqttComm;
FactoryMap							m_FactoryMap(5, 5);
std::vector<uint8_t>				m_MORList;
std::map<std::string, std::string>	m_TempBookPathRequests;
	
/*
 * Entry point of the program
 */
int main (int argc, char **argv)
{		
	/* Description of the Grid-
	1--> The cell is not blocked
	0--> The cell is blocked   
	@ToDo: Automatisch erstellen lassen
	*/
	bool factoryMap_FreeWays[5][5] =
	{
		{ 1, 0, 1, 0, 1 },
		{ 1, 0, 1, 0, 1 },
		{ 1, 1, 1, 1, 1 },
		{ 1, 0, 1, 0, 0 },
		{ 1, 0, 1, 0, 0 }
	};
	
	int factoryMap_IDs[5][5] =
	{
		{ 10001, 99999, 10000, 99999, 10002 },
		{ 31010, 99999, 31010, 99999, 31010 },
		{ 21110, 30101, 21111, 30101, 21100 },
		{ 31010, 99999, 31010, 99999, 99999 },
		{ 10003, 99999, 10004, 99999, 99999 }
	};
	
	for(int row = 0; row < 5; row++) 
	{
		for(int column = 0; column < 5; column++) 
		{
			FactoryMapField field;
			field.fieldIsFree = factoryMap_FreeWays[row][column];
			field.fieldID = factoryMap_IDs[row][column];
			m_FactoryMap.SetInMap(field, row, column);
		}
	}
	
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
		m_mqttComm.Subscribe("SIP40_Factory/Factory/FreePath", Callback_FreePathInFactory);
		
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
	std::cout << "Sent on: SIP40_Factory/MOR_General/TaskQueue" << std::endl;
	
	if(m_FactoryMap.GetAllBookedFieldsAsString() != "NoFieldsBooked")
	{
		m_mqttComm.Publish("SIP40_Factory/Factory/BookPath", m_FactoryMap.GetAllBookedFieldsAsString());
		std::cout << "Sent on: SIP40_Factory/Factory/BookPath" << std::endl;
	}
	
    m_mqttComm.Subscribe("SIP40_Factory/MOR_" + morID + "/TakeTask", Callback_TakeTaskFromMOR);
	m_mqttComm.Subscribe("SIP40_Factory/MOR_" + morID + "/BookPathRequest", Callback_BookPathRequest);
    
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

void Callback_BookPathRequest(std::string topic, std::string path)
{
	bool pathIsBookable = true;
	std::string morName;
	std::istringstream issTopic(topic);
	getline(issTopic, morName, '/');
	getline(issTopic, morName, '/');
	
	std::string singleField;
	std::istringstream issPath(path);
	while (getline(issPath, singleField, '-'))
	{
		std::string xPos;
		std::string yPos;
		std::istringstream issSingleField(singleField);
		getline(issSingleField, xPos, ':');
		getline(issSingleField, yPos, ':');
		
		if(m_FactoryMap.IsFieldBooked(std::stoi(xPos), std::stoi(yPos)))
		{
			std::cout << "Pfad war nicht frei, es wird gewartet bis er frei wird!" << std::endl;
			std::cout << xPos << ":" << yPos << std::endl;
			m_TempBookPathRequests.insert(std::make_pair(morName, path));
			pathIsBookable = false;
			break;
		}
	}
	
	if(pathIsBookable)
	{
		// Falls alle Felder leer sind, dann pfad buchen und antwort schicken
		m_mqttComm.Publish("SIP40_Factory/" + morName +"/PathAnswerFromServer", "RequestAccepted");
		std::cout << "Pfad ist frei und wird dem Roboter gemeldet!" << std::endl;
		BookPath(path);
	}
}

void Callback_FreePathInFactory(std::string topic, std::string path)
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
			
		m_FactoryMap.FreeField(std::stoi(xPos), std::stoi(yPos));
	}
	
	std::cout << "Path released!" << std::endl;
	
	TryToBookTempBookPathRequest();
}

bool TryToBookTempBookPathRequest()
{
	// Create a map iterator and point to beginning of map
	std::map<std::string, std::string>::iterator it = m_TempBookPathRequests.begin();
	
	// Iterate over the m_TempBookPathRequests-map using Iterator till end.
	while (it != m_TempBookPathRequests.end())
	{
		// Accessing morID from element pointed by it.
		std::string morID = it->first;
 
		// Accessing path from element pointed by it.
		std::string path = it->second;
		
		bool pathIsBookable = true;
		std::string singleField;
		std::istringstream issPath(path);
		while (getline(issPath, singleField, '-'))
		{
			std::string xPos;
			std::string yPos;
			std::istringstream issSingleField(singleField);
			getline(issSingleField, xPos, ':');
			getline(issSingleField, yPos, ':');
			
			if(m_FactoryMap.IsFieldBooked(std::stoi(xPos), std::stoi(yPos)))
			{
				pathIsBookable = false;
				break;
			}
		}
		
		if(pathIsBookable)
		{
			std::cout << "Path is now free --> Answer the right mor" << std::endl;
			m_mqttComm.Publish("SIP40_Factory/" + morID +"/PathAnswerFromServer", "RequestAccepted");
			BookPath(path);
			m_TempBookPathRequests.erase(it);
			break;
		}
 
		// Increment the Iterator to point to next entry
		it++;
	}
}

bool BookPath(std::string path)
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
			
		m_FactoryMap.BookField(std::stoi(xPos), std::stoi(yPos));
	}
	
	m_mqttComm.Publish("SIP40_Factory/Factory/BookPathInFactory", path);
	std::cout << "Path booked: " << path << std::endl;
}