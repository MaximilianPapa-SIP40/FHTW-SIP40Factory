/*
 * AStarAlgorithm.cpp: Definiert den Einstiegspunkt für die Konsolenanwendung.
 * Code from https://www.geeksforgeeks.org/a-search-algorithm/
 */

#pragma once

#include "FactoryMap.h"

#include <utility>
#include <stack>

#define ROW 3
#define COL 5

// Creating a shortcut for int, int pair type
typedef std::pair<int, int> Pair;

// Creating a shortcut for pair<int, pair<int, int>> type
typedef std::pair<double, std::pair<int, int>> pPair;

// A structure to hold the neccesary parameters
struct cell
{
	// Row and Column index of its parent
	// Note that 0 <= i <= m_Rows-1 & 0 <= j <= m_Columns-1
	int parent_i, parent_j;
	// f = g + h
	double f, g, h;
};

class AStarAlgorithm
{
public: 
	AStarAlgorithm(FactoryMap* map);
	
	// A Function to find the shortest path between
	// a given source cell to a destination cell according
	// to A* Search Algorithm
	std::vector<std::pair<int, int>> AStarSearch(Pair src, Pair dest);
	
private:
	// A Utility Function to check whether given cell (row, col)
	// is a valid cell or not.
	bool IsValid(int row, int col);
	
	// A Utility Function to check whether the given cell is
	// blocked or not
	bool IsUnBlocked(int row, int col);
	
	// A Utility Function to check whether destination cell has
	// been reached or not
	bool IsDestination(int row, int col, Pair dest);
	
	// A Utility Function to calculate the 'h' heuristics.
	double CalculateHValue(int row, int col, Pair dest);
	
	// A Utility Function to trace the path from the source
	// to destination
	std::vector<std::pair<int, int>> TracePath(cell cellDetails[][COL], Pair dest);
	
	FactoryMap* m_FactoryMap;
};




