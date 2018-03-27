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
	Task m_ActualTask;
	bool stationToggle = false;
	uint8_t stateMachineState = 0;
	uint64_t lastBatteryInformationsSend = 0;
	std::vector<std::pair<int, int>> path;
	int m_SerialFileDescriptor;
	std::string m_SerialDevice;
	int m_SerialBaud;
	
	uint8_t 			m_StateMachineState;
	int m_ActualPositionStationID;
	bool m_RobotDrivesToStart;
	
	static uint16_t 	m_Identity;
	static std::string RobotInStationTopic;
	static bool stationHasFinished;
	static TaskQueue 	m_TaskQueue;
	static MQTTCommunication 	m_mqttComm;
	
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
	static void Callback_BookPathInFactory(std::string topic, std::string path);
	static void Callback_FreePathInFactory(std::string topic, std::string path);
};