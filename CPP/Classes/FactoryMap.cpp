#include "FactoryMap.h"
#include <iostream>

using namespace std;

FactoryMap::FactoryMap(const int rows, const int columns) : m_Rows(rows), m_Columns(columns)
{
	m_Map = new FactoryMapField*[rows];
	
	for(int row = 0; row < rows; row++)
	{
		m_Map[row] = new FactoryMapField[columns];
	}
}

FactoryMap::~FactoryMap()
{
	//Free each sub-array
    for(int row = 0; row < m_Rows; row++) 
	{
        delete[] m_Map[row];   
    }
    //Free the array of pointers
    delete[] m_Map;
}

void FactoryMap::SetInMap(const FactoryMapField field, const int row, const int column)
{
	m_Map[row][column] = field;
}

void FactoryMap::PrintMap() const 
{
	for(int row = 0; row < m_Rows; row++) 
	{
		for(int column = 0; column < m_Columns; column++) 
		{
			cout << m_Map[row][column].fieldIsFree << " ";
		}
		cout << endl;
	}
	
	cout << "----------------------------------" << endl;
	
	for(int row = 0; row < m_Rows; row++) 
	{
		for(int column = 0; column < m_Columns; column++) 
		{
			cout << m_Map[row][column].fieldID << " ";
		}
		cout << endl;
	}
}

string FactoryMap::GiveStationNameFromID(const int stationID) const
{
	return "Station_" + to_string(stationID - 10000);
}

pair<int, int> FactoryMap::FindIDField(const int fieldID) const 
{
	for(int row = 0; row < m_Rows; row++) 
	{
		for(int column = 0; column < m_Columns; column++) 
		{
			if(fieldID == m_Map[row][column].fieldID)
			{
				return pair<int, int>(row, column);
			}
		}
	}
	
	return pair<int, int>(-1, -1);
}

/*int FactoryMap::GetIDFromXYPosition(const int xPos, const int yPos) const
{
	return m_Map[xPos][yPos].fieldID;
}*/

stack<string> FactoryMap::CreateCommandsForRobot(vector<pair<int, int>> path) const 
{
	stack<string> tmp_commandPathToRobot;
	pair<int, int> prevCoordinates = path.back();
	
	while (!path.empty())
	{
		pair<int, int> coordinates = path.back();
		path.pop_back();
		
		if((m_Map[coordinates.first][coordinates.second].fieldID - 10000) < 10000)
		{
			// Station on this position
			if(!path.empty())
			{
				// don't turn at the destination
				tmp_commandPathToRobot.push("T");
				// drive straight after the turn
				tmp_commandPathToRobot.push("F");
			}
		}
		else if((m_Map[coordinates.first][coordinates.second].fieldID - 20000) < 10000)
		{
			// Crossing on this position -> Check in which direction the robot should go next
			pair<int, int> nextCoordinates = path.back();
			pair<int, int> moveOne = make_pair((coordinates.first - prevCoordinates.first), (coordinates.second - prevCoordinates.second));
			pair<int, int> moveTwo = make_pair((nextCoordinates.first - coordinates.first), (nextCoordinates.second - coordinates.second));
			pair<int, int> moveAll = make_pair((moveOne.first + moveTwo.first), (moveOne.second + moveTwo.second));
			
			if(moveOne.first == 0)
			{
				if((moveAll.first + moveAll.second) == 0)
				{
					// Drive left
					tmp_commandPathToRobot.push("L");
				}
				else
				{
					if((moveAll.first == 1) || (moveAll.first == -1))
					{
						// Drive right
						tmp_commandPathToRobot.push("R");
					}
					else
					{
						// Drive straight
						tmp_commandPathToRobot.push("F");
					}
				}
			}
			else
			{
				if((moveAll.first + moveAll.second) == 0)
				{
					// Drive right
					tmp_commandPathToRobot.push("R");
				}
				else
				{
					if((moveAll.first == 1) || (moveAll.first == -1))
					{
						// Drive left
						tmp_commandPathToRobot.push("L");
					}
					else
					{
						// Drive straight
						tmp_commandPathToRobot.push("F");
					}
				}
			}
		}
		else if((m_Map[coordinates.first][coordinates.second].fieldID - 30000) < 10000)
		{
			// Road on this position
			// Do nothing -> Let the robot drive till next crossing or station
			// Save the previous coordinates, if the next field is a crossing
			prevCoordinates = coordinates;
		}
		else if((m_Map[coordinates.first][coordinates.second].fieldID - 99999) < 0)
		{
			cout << "Error: Path planning gone wrong!";
		}
		else
		{
			cout << "Error: Path planning gone wrong!";
		}
	}
	
	// switch the direction of the stack
	stack<string> commandPathToRobot;
	while(!tmp_commandPathToRobot.empty())
	{
		commandPathToRobot.push(tmp_commandPathToRobot.top());
		tmp_commandPathToRobot.pop();
	}
	
	return commandPathToRobot;
}

bool FactoryMap::GetFieldFreeInformationOnPosition(const int row, const int column)
{
	return m_Map[row][column].fieldIsFree;
}

bool FactoryMap::BookField(const int xPos, const int yPos) const
{
	m_Map[xPos][yPos].fieldIsBooked = true;
}

bool FactoryMap::FreeField(const int xPos, const int yPos) const
{
	m_Map[xPos][yPos].fieldIsBooked = false;
}

bool FactoryMap::IsFieldBooked(const int xPos, const int yPos) const
{
	return m_Map[xPos][yPos].fieldIsBooked;
}

std::string FactoryMap::GetAllBookedFieldsAsString() const
{
	std::string bookedFields = "";
	
	for(int row = 0; row < m_Rows; row++) 
	{
		for(int column = 0; column < m_Columns; column++) 
		{
			if(m_Map[row][column].fieldIsBooked == true)
			{
				bookedFields.append(to_string(row));
				bookedFields.append(":");
				bookedFields.append(to_string(column));
				bookedFields.append("-");
			}
		}
	}
	
	// Delete the last "-" of the bookedFields
	bookedFields = bookedFields.substr(0, bookedFields.size()-1);
	
	if(bookedFields.empty())
	{
		return "NoFieldsBooked";
	}
	
	return bookedFields;
}