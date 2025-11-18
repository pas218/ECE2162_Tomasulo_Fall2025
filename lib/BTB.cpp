#include "BTB.hpp"
#include <iostream>
/*
struct BTB_Entry
{
	int instrAddress;
	int predictedAddress;
	bool isTaken;
}
*/

BTB::BTB()
{
	table = new BTB_Entry[length];

	// Initialize entry.
	for (int i = 0; i < length; i++)
	{
		table[i].instrAddress = -1;
		table[i].predictedAddress = -1;
		table[i].isTaken = true; // Assume first is taken.
	}
}

BTB::~BTB()
{
	delete[] table;
}

int BTB::getLength()
{
	return length;
}

int BTB::freeSpot()
{
	int returnVal = -1;
	for (int i = 0; i < length; i++)
	{
		if (table[i].instrAddress == -1)
		{
			returnVal = i;
			break;
		} 
	}
	return returnVal;
}

bool BTB::clearEntry(int address)
{
	bool returnVal = 0;
	for (int i = 0; i < length; i++)
	{
		if (table[i].instrAddress == address)
		{
			table[i].instrAddress = -1;
			table[i].predictedAddress = -1;
			table[i].isTaken = true; // Assume first is taken.
			returnVal = 1;
			break;
		} 
	}
	return returnVal;
}


bool BTB::changeFullEntry(int instrAddress, int predictedAddress, bool isTaken, bool kickout)
{
	bool returnVal = 0;
	// std::cout << "Changing entry with address " << instrAddress << std::endl;
	// std::cout << "Length (shoud be 8): " << length << std::endl;
	for (int i = 0; i < length; i++)
	{
		if (table[i].instrAddress == instrAddress)
		{
			// InstrAddress stays the same.
			table[i].predictedAddress = predictedAddress;
			table[i].isTaken ^= isTaken;
			returnVal = 1;
			break;
		} 
	}
	
	// If buffer doesn't have an existing entry and has an open spot, put the entry into the new spot.
	int spot = freeSpot();
	if ((returnVal == 0) && (spot != -1))
	{
		// std::cout << "Creating a new BTB entry.";
		table[spot].instrAddress     = instrAddress;
		table[spot].predictedAddress = predictedAddress;
		table[spot].isTaken         ^= isTaken; // Assume first is taken.
		returnVal = 1;
	}

	// If the buffer is full and the user decides to kickout, then kickout the first option.
	if ((returnVal == 0) && (kickout == 1))
	{
		// std::cout << "Creating a new BTB entry.";
		table[0].instrAddress     = instrAddress;
		table[0].predictedAddress = predictedAddress;
		table[0].isTaken         ^= isTaken; // Assume first is taken.
		returnVal = 1;
	}

	return returnVal;
}

bool BTB::changeIsTaken(int instrAddress, bool isTaken)
{
	bool returnVal = 0;
	for (int i = 0; i < length; i++)
	{
		if (table[i].instrAddress == instrAddress)
		{
			// Keep instrAddress and predictedAddress the same.
			table[i].isTaken ^= isTaken; // Assume first is taken.
			returnVal = 1;
			break;
		} 
	}
	return returnVal;
}

BTB_Entry* BTB::getTableEntry(int address)
{
	BTB_Entry *returnVal = nullptr;
	for (int i = 0; i < length; i++)
	{
		if (table[i].instrAddress == address)
		{
			returnVal = &table[i];
			break;
		}
	}
	return returnVal;
}

BTB_Entry* BTB::getTableEntryByIndex(int index)
{
	BTB_Entry *returnVal = nullptr;
	for (int i = 0; i < length; i++)
	{
		if (i == index)
		{
			returnVal = &table[i];
			break;
		}
	}
	return returnVal;
}