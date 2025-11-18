// ReOrderBuf.cpp
#include "ReOrderBuf.hpp"

ReOrderBuf::ReOrderBuf(int n)
{
    size = n;
    this->n = 0;
    head = 0;
    tail = 0;
    table = new ReOrderBuf_entry[size];
	headPtr = 0;
	tailPtr = 0;
	
	for (int i = 0; i < size; i++)
	{
		table[i].id       = -1;
		table[i].dst_id   = -1;
		table[i].value    = 0.0;
		table[i].cmt_flag = 0;
		table[i].isBranch = 0;
	}
	
}

bool ReOrderBuf::full()
{
	return n == size ? true : false;
}

bool ReOrderBuf::flushAfterSpot(int spotNumber)
{
	bool returnVal = false;
	if (headPtr == tailPtr)
	{
		//std::cout << "Nothing to flush. The ROB is empty.\n";
	}
	else if (headPtr > tailPtr)
	{
		for (int i = spotNumber + 1; i < size; i++)
		{
			clearSpot(i);
		}
		returnVal = true;
		headPtr = spotNumber + 1;
	}
	else if (headPtr < tailPtr)
	{
		std::cout << "TODO: [ReOrderBuf::flushAfterSpot]: Need to look at the case where tailPtr < headPtr." << std::endl;
	}
	
	
	return returnVal;
}

int ReOrderBuf::findDependency(int depType, int regID)
{
	int returnVal = -1;
	// Bug fix: don't scan for the latest addition to the ROB.
	// So, you can write instructions like "Add R1, R1, R2" without inducing a self-dependency.
	for (int i = headPtr-2; i >= 0; i--)
	{
		if ((table[i].id == depType) && (table[i].dst_id == regID))
		{
			returnVal = i;
			break;
		}
	}
	return returnVal;
}

bool ReOrderBuf::ableToCommit(int spotNumber)
{
	// First make sure that the commit flag is set.
	bool returnVal = table[spotNumber].cmt_flag == 1 ? true : false;

	if (!((returnVal == true) && (spotNumber == tailPtr)))
	{
		returnVal = false;
	}
	
	return returnVal;
}
commitReturn ReOrderBuf::commit(int spotNumber)
{
	commitReturn returnVal;

	if (spotNumber == tailPtr)
	{
		returnVal.validCommit = true;
		returnVal.regType = table[spotNumber].id;
		returnVal.registerNum = table[spotNumber].dst_id;
		returnVal.returnValue = table[spotNumber].value;
		clearSpot(spotNumber);
		tailPtr = spotNumber+1;
	}

	return returnVal;
}

int ReOrderBuf::freeSpot()
{
	int returnVal = -1;
	if (headPtr == tailPtr)
	{
		returnVal = headPtr;
	}
	else if (headPtr > tailPtr)
	{
		returnVal = headPtr;
	}
	else if (headPtr < tailPtr)
	{
		std::cout << "TODO: [ReOrderBuf::commit]: Need to look at the case where tailPtr < headPtr." << std::endl;
	}
	return returnVal;
}

void ReOrderBuf::clearSpot(int spotNumber)
{
	table[spotNumber].id       = -1;
	table[spotNumber].dst_id   = -1;
	table[spotNumber].value    = 0.0;
	table[spotNumber].cmt_flag = 0;
	table[spotNumber].isBranch = 0;
}

void ReOrderBuf::printSpot(int spotNumber)
{
	std::cout << "Printing spot: " << spotNumber << std::endl;
	std::cout << "table[" << spotNumber << "].id = " << table[spotNumber].id << std::endl;
	std::cout << "table[" << spotNumber << "].dst_id = " << table[spotNumber].dst_id << std::endl;
	std::cout << "table[" << spotNumber << "].value = " <<  std::to_string(table[spotNumber].value) << std::endl;
	std::cout << "table[" << spotNumber << "].cmt_flag = " << table[spotNumber].value << std::endl;
}