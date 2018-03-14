#pragma once

#include <set>
#include <stack>
#include <string>

struct FactoryMapField
{
	bool 	fieldIsFree = false;
	int 	fieldID = 0;
};

class FactoryMap
{
public: 
	FactoryMap(const int rows, const int columns);
	~FactoryMap();
	
	void SetInMap(const FactoryMapField field, const int row, const int column);
	
	void PrintMap() const;
	
	std::string GiveStationNameFromID(const int stationID) const;
	
	std::pair<int, int> FindIDField(const int fieldID) const;
	
	std::stack<std::string> CreateCommandsForRobot(std::stack<std::pair<int, int>> path) const;
	
	bool GetFieldFreeInformationOnPosition(const int row, const int column);
	
private:
	FactoryMapField** m_Map;
	int m_Rows;
	int m_Columns;
};