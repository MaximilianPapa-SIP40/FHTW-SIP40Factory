#include "MQTTCommunication.h"
#include <mosquitto.h>
#include <iostream>

std::vector<std::pair<std::string, void (*)(std::string topic, std::string message)>> MQTTCommunication::m_CallbackFunctions;
bool MQTTCommunication::m_ConnectionState = false;

MQTTCommunication::MQTTCommunication(const std::string mqttHostname, const int mqttPort)
	: m_mqttHostname(mqttHostname)
	, m_mqttPort(mqttPort)
{
	mosquitto_lib_init();
	m_Communication = mosquitto_new(NULL, true, 0);
}

MQTTCommunication::~MQTTCommunication()
{
	if(m_Communication)
	{
		mosquitto_disconnect(m_Communication);
		mosquitto_loop_stop(m_Communication, 1);
		mosquitto_destroy(m_Communication);
	}
	
	mosquitto_lib_cleanup();
}

bool MQTTCommunication::Connect(const std::string hostname, const int port) 
{
	if(!hostname.empty())
	{
		m_mqttHostname = hostname; 
	}
	if(port != 0)
	{
		m_mqttPort = port;
	}
	
	if(m_Communication)
	{
		std::cout << "Connecting to: " << m_mqttHostname << ":" << m_mqttPort << std::endl;
		
		mosquitto_connect_callback_set(m_Communication, OnConnect_Callback);
		mosquitto_message_callback_set(m_Communication, OnMessage_Callback);

		mosquitto_connect(m_Communication, m_mqttHostname.c_str(), m_mqttPort, 60);
		mosquitto_loop_start(m_Communication);
		
		while(!IsConnected())
		{
			
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

void MQTTCommunication::AddTopicMessageCallback(const std::string topic, void (*callBackFunction)(std::string topic, std::string message)) 
{
	m_CallbackFunctions.push_back(std::pair<std::string, void (*)(std::string topic, std::string message)>(topic, callBackFunction));
}

void MQTTCommunication::DeleteTopicMessageCallback(const std::string topic) 
{
	for (std::vector<std::pair<std::string, void (*)(std::string topic, std::string message)>>::iterator it = m_CallbackFunctions.begin() ; it != m_CallbackFunctions.end();)
    {
		if(it->first == topic)
		{
			it = m_CallbackFunctions.erase(it);
		}
		else
		{
			it++;
		}
	}
}

bool MQTTCommunication::Publish(const std::string topic, const std::string message)
{
	return mosquitto_publish(m_Communication, NULL, topic.c_str(), message.length(), message.c_str(), 0, false);
}

bool MQTTCommunication::Subscribe(const std::string topic, void (*callBackFunction)(std::string topic, std::string message))
{
	AddTopicMessageCallback(topic, callBackFunction);
	
	return mosquitto_subscribe(m_Communication, NULL, topic.c_str(), 0);
}

bool MQTTCommunication::Unsubscribe(const std::string topic)
{
	DeleteTopicMessageCallback(topic);
	return mosquitto_unsubscribe(m_Communication, NULL, topic.c_str());
}

bool MQTTCommunication::IsConnected() const
{
	return m_ConnectionState;
}

/*
 * OnConnect_Callback
 * Called when a new connection is started
 */
void MQTTCommunication::OnConnect_Callback(struct mosquitto *mosq, void *obj, int resultCode)
{
	std::cout << "Connected with result code: " << resultCode << std::endl;
	
	m_ConnectionState = true;
}

/*
 * Message_Callback
 * Called whenever a new message arrives
 */
void MQTTCommunication::OnMessage_Callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	// Note: nothing in the Mosquitto docs or examples suggests that we
	//  must free this message structure after processing it,
	//  (nor that we can assume that it is null-terminated.)
	std::cout << "Got message:" << (char *)message->payload;
	std::cout << " on Topic: " << (char *)message->topic << std::endl;
	
	for (int index = 0; index < m_CallbackFunctions.size(); index++)
    {
		if(m_CallbackFunctions[index].first == std::string((char *)message->topic))
		{
			m_CallbackFunctions[index].second((char *)message->topic, (char *)message->payload);
		}
	}
}