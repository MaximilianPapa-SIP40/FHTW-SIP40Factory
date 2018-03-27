#include "TaskQueue.h"
#include <iostream>
#include <sstream>

using namespace std;

bool TaskQueue::SetTaskQueueFromMQTTString(const string taskString)
{
	std::cout << "MOR received TaskString: " << taskString << std::endl;
	
	m_TaskQueue.clear();
	
	if (taskString.compare("empty") != 0)
	{
		string singleTask;
		istringstream issTaskQueue(taskString);
		while (getline(issTaskQueue, singleTask, ';'))
		{
			if(singleTask.compare("empty") != 0)
			{
				Task tempTask;
				string tempTaskID, tempTaskPriority, tempStartStationID, tempDestStationID;
				
				istringstream issSingleTask(singleTask);
				
				getline(issSingleTask, tempTaskID, '-');
				tempTask.m_TaskID = stoi(tempTaskID);
				
				getline(issSingleTask, tempTaskPriority, '-');
				tempTask.m_TaskPriority = stoi(tempTaskPriority);
				
				string pathFromStartToDestination;
				getline(issSingleTask, pathFromStartToDestination, '-');
				
				istringstream issPath(pathFromStartToDestination);
				
				getline(issPath, tempStartStationID, '>');
				tempTask.m_StartStationID = stoi(tempStartStationID);
				
				getline(issPath, tempDestStationID, '>');
				tempTask.m_DestStationID = stoi(tempDestStationID);
				
				m_TaskQueue.push_back(tempTask);
			}
		}
	}
	
	return true;
}

bool TaskQueue::InsertNewTaskInQueueFromMQTTString(string taskString)
{
	taskString = to_string(m_RunningTaskID) + "-" + taskString;
	m_RunningTaskID++;
	
	std::cout << taskString << std::endl;
	
	Task tempTask;
	string tempTaskID, tempTaskPriority, tempStartStationID, tempDestStationID;
	
	istringstream issSingleTask(taskString);
	
	getline(issSingleTask, tempTaskID, '-');
	tempTask.m_TaskID = stoi(tempTaskID);
	
	getline(issSingleTask, tempTaskPriority, '-');
	tempTask.m_TaskPriority = stoi(tempTaskPriority);
	
	string pathFromStartToDestination;
	getline(issSingleTask, pathFromStartToDestination, '-');
	
	istringstream issPath(pathFromStartToDestination);
	
	getline(issPath, tempStartStationID, '>');
	tempTask.m_StartStationID = stoi(tempStartStationID);
	
	getline(issPath, tempDestStationID, '>');
	tempTask.m_DestStationID = stoi(tempDestStationID);
	
	m_TaskQueue.push_back(tempTask);
	
	std::cout << "Added Task: " << std::endl;
	PrintTaskWithID(tempTask.m_TaskID);
	
	return true;
}
	
bool TaskQueue::UpdatePriorityFromTaskWithID(const int taskID)
{
	
}

bool TaskQueue::DeleteTaskWithID(const int taskID)
{
	int index = GetIndexofTask(taskID);
	
	if(index == -1)
	{
		return false;
	}
	
	m_TaskQueue.erase(m_TaskQueue.begin() + index);
	return true;
}

bool TaskQueue::IsEmpty() const
{
	return m_TaskQueue.empty();
}
	
const vector<Task>* TaskQueue::GetTaskQueue() const
{
	return &m_TaskQueue;
}
	
const Task TaskQueue::GetHighestPriorityTask() const
{
	if(IsEmpty())
	{
		return Task();
	}
	
	int hightestPriority = 0, highestPriorityIndex = 0;
	for(int index = (m_TaskQueue.size() - 1); index >= 0; index--)
	{
		if(m_TaskQueue[index].m_TaskPriority >= hightestPriority)
		{
			highestPriorityIndex = index;
			hightestPriority = m_TaskQueue[index].m_TaskPriority;
		}
	}
	
	return m_TaskQueue[highestPriorityIndex];
}

const Task TaskQueue::GetTaskWithID(const int taskID) const
{
	
}

int TaskQueue::GetIndexofTask(const int taskID) const
{
	for(size_t index = 0; index < m_TaskQueue.size(); index++)
	{
		if(m_TaskQueue[index].GetTaskID() == taskID)
		{
			return index;
		}
	}
	
	return -1;
}

std::string TaskQueue::GetMQTTStringOfTaskQueue() const
{
	if(m_TaskQueue.empty())
	{
		return "empty";
	}
	else
	{
		std::string mqttString;
		
		for(size_t index = 0; index < m_TaskQueue.size(); index++)
		{
			mqttString.append(to_string(m_TaskQueue[index].GetTaskID()));
			mqttString.append("-");
			mqttString.append(to_string(m_TaskQueue[index].GetTaskPriority()));
			mqttString.append("-");
			mqttString.append(to_string(m_TaskQueue[index].GetStartStationID()));
			mqttString.append(">");
			mqttString.append(to_string(m_TaskQueue[index].GetDestStationID()));
						
			if(index < (m_TaskQueue.size() - 1))
			{
				mqttString.append(";");
			}
		}
		
		return mqttString;
	}
}

void TaskQueue::PrintWholeTaskQueue() const
{
	for(size_t index = 0; index < m_TaskQueue.size(); index++)
	{
		cout 	<< m_TaskQueue[index].GetTaskID() << "-" 
				<< m_TaskQueue[index].GetTaskPriority() << "-" 
				<< m_TaskQueue[index].GetStartStationID() << ">" 
				<< m_TaskQueue[index].GetDestStationID() << endl;
	}
}

void TaskQueue::PrintTaskWithID(const int taskID) const
{
	int index = GetIndexofTask(taskID);
	
	if(index == -1)
	{
		return;
	}
	
	cout << "ID: " << m_TaskQueue[index].GetTaskID() << endl;
	cout << "Prioritaet: " << m_TaskQueue[index].GetTaskPriority() << endl;
	cout << "StartStationID: " << m_TaskQueue[index].GetStartStationID() << endl;
	cout << "DestStationID: " << m_TaskQueue[index].GetDestStationID() << endl;
}

void TaskQueue::PrintWayFromTaskWithID(const int taskID) const
{
	int index = GetIndexofTask(taskID);
	
	if(index == -1)
	{
		return;
	}
	
	cout << "( " << m_TaskQueue[index].GetStartStationID() << ">" << m_TaskQueue[index].GetDestStationID() << " )" << endl;
}