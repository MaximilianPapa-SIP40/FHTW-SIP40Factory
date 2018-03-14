#pragma once

#include <string>
#include <vector>

class MQTTCommunication
{
public: 
	MQTTCommunication(const std::string mqttHostname = "", const int mqttPort = 0);
	~MQTTCommunication();
	
	bool Connect(const std::string hostname = "", const int port = 0);
	bool Publish(const std::string topic, const std::string message);
	bool Subscribe(const std::string topic, void (*callBackFunction)(std::string topic, std::string message));
	bool Unsubscribe(const std::string topic);
	
	bool IsConnected() const;
	
private:
	std::string m_mqttHostname;
	int m_mqttPort;
	struct mosquitto* m_Communication;
	static bool m_ConnectionState;
	static std::vector<std::pair<std::string, void (*)(std::string topic, std::string message)>> m_CallbackFunctions;
	
	void AddTopicMessageCallback(const std::string topic, void (*callBackFunction)(std::string topic, std::string message));
	void DeleteTopicMessageCallback(const std::string topic);
	
	static void OnConnect_Callback(struct mosquitto *mosq, void *obj, int resultCode);
	static void OnMessage_Callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
};