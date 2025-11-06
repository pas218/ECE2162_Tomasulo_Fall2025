// ReOrderBuf.cpp
#include "ReOrderBuf.hpp"

ReOrderBuf::ReOrderBuf(int n)
{
    size = n;
    this->n = 0;
    head = 0;
    tail = 0;
    table = new ReOrderBuf_entry[size];
	
	for (int i = 0; i < size; i++)
	{
		table[i].id       = -1;
		table[i].dst_id   = -1;
		table[i].value    = 0.0;
		table[i].cmt_flag = 0;
	}
	
}

bool ReOrderBuf::full()
{
	return n == size ? true : false;
}

int ReOrderBuf::findDependency(int depType, int regID)
{
	int returnVal = -1;
	for (int i = 0; i < size; i++)
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

	// Make sure there are no entries before the one we want to commit.
	for (int i = spotNumber-1; i >= 0; i--)
	{	
		if (table[i].id != -1)
		{
			returnVal = false;
			break;
		}
	}
	return returnVal;
}
commitReturn ReOrderBuf::commit(int spotNumber)
{
	commitReturn returnVal;

	returnVal.validCommit = true;
	returnVal.regType = table[spotNumber].id;
	returnVal.registerNum = table[spotNumber].dst_id;
	returnVal.returnValue = table[spotNumber].value;
	clearSpot(spotNumber);

	return returnVal;
}

int ReOrderBuf::freeSpot()
{
	int returnVal = -1;
	for (int i = 0; i < size; i++)
	{
		if (table[i].id == -1)
		{
			returnVal = i;
			break;
		}
	}
	return returnVal;
}

void ReOrderBuf::clearSpot(int spotNumber)
{
	table[spotNumber].id       = -1;
	table[spotNumber].dst_id   = -1;
	table[spotNumber].value    = 0.0;
	table[spotNumber].cmt_flag = 0;
}