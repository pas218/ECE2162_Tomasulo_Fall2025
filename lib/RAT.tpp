#include <iostream>

template <typename T>
RAT<T>::RAT()
{
	// Arbitrarily decide 32 locations.
    numLocations = 32;
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
	numLocations = numLocations;
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
RAT_type RAT<T>::getValue(int locationNumber)
{	
	RAT_type returnVal = locationsPtr[0];
	if (locationNumber < numLocations)
	{
		returnVal = locationsPtr[locationNumber];
	}
	return returnVal;
}