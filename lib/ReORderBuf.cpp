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
		table[i].dst_id   = 0;
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