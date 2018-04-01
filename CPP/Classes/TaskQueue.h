#pragma once

#include <vector>
#include <string>
#include <stdint.h>

class Task
{
public:
	int GetTaskID() const
	{
		return m_TaskID;
	}
	
	int GetTaskPriority() const
	{
		return m_TaskPriority;
	}
	
	int GetStartStationID() const
	{
		return m_StartStationID;
	}
	
	int GetDestStationID() const
	{
		return m_DestStationID;
	}
	
private:
	int 	m_TaskID 			= 0;
	int 	m_TaskPriority		= 0;
	int 	m_StartStationID	= 0;
	int 	m_DestStationID		= 0;
	friend 	class TaskQueue;
};

class TaskQueue
{
public: 	
	TaskQueue() : m_RunningTaskID(0) {}

	bool SetTaskQueueFromMQTTString(const std::string taskString);
	bool InsertNewTaskInQueueFromMQTTString(const std::string taskString);
	bool UpdatePriorityFromTaskWithID(const int taskID);
	bool DeleteTaskWithID(const int taskID);
	
	bool DoesTaskExist(const int taskID) const;
	bool IsEmpty() const;
	
	const std::vector<Task>* GetTaskQueue() const;
	const Task GetHighestPriorityTask() const;
	const Task GetTaskWithID(const int taskID) const;
	int GetIndexofTask(const int taskID) const;
	std::string GetMQTTStringOfTaskQueue() const;
	
	void PrintWholeTaskQueue() const;
	void PrintTaskWithID(const int taskID) const;
	void PrintWayFromTaskWithID(const int taskID) const;
	
private:
	std::vector<Task> m_TaskQueue;
	uint64_t m_RunningTaskID;
};