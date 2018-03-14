#pragma once

#include "FactoryMap.h"
#include "TaskQueue.h"
#include "MQTTCommunication.h"

class MobileRobot
{
public:
	MobileRobot(const std::string mqttHostname, const int mqttPort, FactoryMap& map, const std::string serialDevice, const int serialBaud);
	
	bool Run();
	
private:
	Task m_ActualTask;
	bool stationToggle = false;
	uint8_t stateMachineState = 0;
	uint64_t lastBatteryInformationsSend = 0;
	std::stack<std::pair<int, int>> path;
	int m_SerialFileDescriptor;
	std::string m_SerialDevice;
	int m_SerialBaud;
	FactoryMap* 		m_FactoryMap;
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
		
	void TakeTask(uint64_t taskID);

	/*
	 * MQTT-Callback Methods
	 */
	static void UpdateMORTaskQueue(std::string topic, std::string taskQueue);
	static void InitMOR(std::string topic, std::string identity);
	static void RobotInStation(std::string topic, std::string value);
};