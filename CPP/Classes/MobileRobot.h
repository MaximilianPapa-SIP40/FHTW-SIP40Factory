#pragma once

#include "FactoryMap.h"
#include "TaskQueue.h"
#include "MQTTCommunication.h"

class MobileRobot
{
public:
	MobileRobot(const std::string mqttHostname, const int mqttPort, const std::string serialDevice, const int serialBaud);
	
	bool Run();
	
	static FactoryMap* 		m_FactoryMap;
	
private:
	Task 								m_ActualTask;
	bool 								stationToggle;
	uint8_t 							stateMachineState;
	uint64_t 							lastBatteryInformationsSend;
	std::vector<std::pair<int, int>>	path;
	uint8_t 							m_StateMachineState;
	int 								m_ActualPositionStationID;
	bool 								m_RobotDrivesToStart;
	std::string 						m_mqttHostname;
	int 								m_mqttPort;
	
	int 			m_SerialFileDescriptor;
	std::string 	m_SerialDevice;
	int 			m_SerialBaud;
	
	static uint16_t 			m_Identity;
	static std::string 			RobotInStationTopic;
	static bool 				stationHasFinished;
	static TaskQueue 			m_TaskQueue;
	static MQTTCommunication 	m_mqttComm;
	static bool 				m_TaskAnswerArrived;
	static bool 				m_TaskSuccessfullyTaken;
	static bool 				m_PathAnswerFromServer;
	
	bool InitializeSerialConnection();
	bool InitializeMQTTConnection();
	
	bool DriveAlongPath(const std::vector<std::pair<int, int>> path) const;
		
	void TakeTask(uint64_t taskID);
	void BookPath(const std::vector<std::pair<int, int>> path) const;
	void FreePath(const std::vector<std::pair<int, int>> path) const;

	/*
	 * MQTT-Callback Methods
	 */
	static void UpdateMORTaskQueue(std::string topic, std::string taskQueue);
	static void InitMOR(std::string topic, std::string identity);
	static void RobotInStation(std::string topic, std::string value);
	static void Callback_PathAnswerFromServer(std::string topic, std::string value);
	static void Callback_BookPathInFactory(std::string topic, std::string path);
	static void Callback_FreePathInFactory(std::string topic, std::string path);
	static void Callback_TakeTask(std::string topic, std::string answer);
};