#include <iostream>
#include <vector>

template <typename T>
RAT<T>::RAT()
{
	// Arbitrarily decide 32 locations.
    this->numLocations = 32;
    locationsPtr = new RAT_type[32];
	for (int i = 0; i < 32; i++)
	{
		if (std::is_same<T, int>::value)
		{
			locationsPtr[i].locationType = 'R';
		}
		else
		{
			locationsPtr[i].locationType = 'F';
		}
		locationsPtr[i].locationNumber = i;
	}
}

template <typename T>
RAT<T>::RAT(int numLocations)
{
	this->numLocations = numLocations;
    locationsPtr = new RAT_type[numLocations];
	for (int i = 0; i < numLocations; i++)
	{
		if (std::is_same<T, int>::value)
		{
			locationsPtr[i].locationType = 'R';
		}
		else
		{
			locationsPtr[i].locationType = 'F';
		}
		locationsPtr[i].locationNumber = i;
	}
}

template <typename T>
RAT<T>::~RAT()
{
	delete[] locationsPtr;
}

template <typename T>
std::vector<int> RAT<T>::getARFLocations(int robLocation)
{
	std::vector<int> returnVal;
	for (int i = 0; i < numLocations; i++)
	{
		if ((locationsPtr[i].locationType == 'B') && (locationsPtr[i].locationNumber == robLocation))
		{
			returnVal.push_back(i);
			resetLocation(i);
		}
	}
	return returnVal;
}

template<typename T>
int RAT<T>::getSize()
{
	return numLocations;
}

template <typename T>
bool  RAT<T>::changeValue(int locationNumber, bool isARF, int robNumber)
{	
	bool returnVal = 0;
	if (locationNumber < numLocations)
	{
		returnVal = 1;
		if (isARF)
		{
			if (std::is_same<T, int>::value)
			{
				locationsPtr[locationNumber].locationType = 'R';
			}
			else
			{
				locationsPtr[locationNumber].locationType = 'F';
			}
			locationsPtr[locationNumber].locationNumber = locationNumber;
		}
		else
		{
			locationsPtr[locationNumber].locationType = 'B';
			locationsPtr[locationNumber].locationNumber = robNumber;
		}
	}
	
	return returnVal;
}

template <typename T>
bool RAT<T>::resetLocation(int locationNumber)
{
	if (std::is_same<T, int>::value)
	{
		locationsPtr[locationNumber].locationType = 'R';
	}
	else
	{
		locationsPtr[locationNumber].locationType = 'F';
	}
	locationsPtr[locationNumber].locationNumber = locationNumber;
	return true;
}

template <typename T>
RAT_type* RAT<T>::getValue(int locationNumber)
{	
	RAT_type *returnVal = locationsPtr;
	if (locationNumber < numLocations)
	{
		returnVal = &locationsPtr[locationNumber];
	}
	return returnVal;
}