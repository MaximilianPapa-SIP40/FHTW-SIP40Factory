#pragma once

#include <set>
#include <stack>
#include <vector>
#include <string>

struct FactoryMapField
{
	bool 	fieldIsFree = false;
	bool 	fieldIsBooked = false;
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
	int GetIDFromXYPosition(const int xPos, const int yPos) const;
	std::stack<std::string> CreateCommandsForRobot(std::vector<std::pair<int, int>> path) const;
	bool GetFieldFreeInformationOnPosition(const int row, const int column);
	
	bool BookField(const int xPos, const int yPos) const;
	bool FreeField(const int xPos, const int yPos) const;
	bool IsFieldBooked(const int xPos, const int yPos) const;
	std::string GetAllBookedFieldsAsString() const;
	
private:
	FactoryMapField** m_Map;
	int m_Rows;
	int m_Columns;
};