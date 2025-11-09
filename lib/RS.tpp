// RS.tpp
template <typename T, typename Op>
inline bool RS<T, Op>::checkBounds(int stationNumber)
{
	bool returnVal = 0;
	if ((stationNumber >= 0) && (stationNumber < numStations))
	{
		returnVal = 1;
	}
	return returnVal;
	
}

/*
template <typename T, typename Op>
inline bool RS<T, Op>::checkRobFree(int robLocation)
{
	return 
}
*/

template <typename T, typename Op>
RS<T, Op>::RS()
{
	// Arbitrarily decide 32 locations.
    numStations = 32;
    stationsPtr = new RS_type<T, Op>[32];
	for (int i = 0; i < 32; i++)
	{
		if (std::is_same<T, int>::value)
		{
			stationsPtr[i].value0 = static_cast<int>(0);
			stationsPtr[i].value1 = static_cast<int>(0);
			stationsPtr[i].computation = static_cast<int>(0);
			
		}
		else
		{
			stationsPtr[i].value0 = static_cast<float>(0.0);
			stationsPtr[i].value1 = static_cast<float>(0.0);
			stationsPtr[i].computation = static_cast<float>(0.0);
		}
		stationsPtr[i].operation      = EMPTY;
		stationsPtr[i].robLocation    = -1;
		stationsPtr[i].robDependency0 = -1;
		stationsPtr[i].robDependency1 = -1;
		stationsPtr[i].computationDone = false;
	}
}

template <typename T, typename Op>
RS<T, Op>::RS(int numLocations)
{
	numStations = numLocations;
    stationsPtr = new RS_type<T, Op>[numLocations];
	for (int i = 0; i < numLocations; i++)
	{
		if (std::is_same<T, int>::value)
		{
			stationsPtr[i].value0 = static_cast<int>(0);
			stationsPtr[i].value1 = static_cast<int>(0);
			stationsPtr[i].computation = static_cast<int>(0);
			
		}
		else
		{
			stationsPtr[i].value0 = static_cast<float>(0.0);
			stationsPtr[i].value1 = static_cast<float>(0.0);
			stationsPtr[i].computation = static_cast<float>(0.0);
		}
		stationsPtr[i].operation      = EMPTY;
		stationsPtr[i].robLocation    = -1;
		stationsPtr[i].robDependency0 = -1;
		stationsPtr[i].robDependency1 = -1;
		stationsPtr[i].computationDone = false;
	}
}

template <typename T, typename Op>
RS<T, Op>::~RS(){
	delete[] stationsPtr;
}

template <typename T, typename Op>
int RS<T, Op>::freeSpot()
{
	int returnVal = -1;
	for (int i = 0; i < numStations; i++)
	{
		if (stationsPtr[i].robLocation == -1)
		{
			returnVal = i;
			break;
		}
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::replaceROBDependency(int robLocation, T value)
{
	bool returnVal = false;
	for (int i = 0; i < numStations; i++)
	{
		if (getValue(i)->robDependency0 == robLocation)
		{
			returnVal = true;
			changeRSVal0(i, value);
			getValue(i)->robDependency0 = -1;  // Clear the dependency
		}
		if (getValue(i)->robDependency1 == robLocation)
		{
			returnVal = true;
			changeRSVal1(i, value);
			getValue(i)->robDependency1 = -1;  // Clear the dependency
		}
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::hasUnaddressedDependencies(int robLocation)
{
	bool returnVal = false;
	int RSSpot = findRSFromROB(robLocation);
	if ((stationsPtr[RSSpot].robDependency0 != -1) 
		|| (stationsPtr[RSSpot].robDependency1 != -1))
	{
		returnVal = true;
	}
	return returnVal;
}

template <typename T, typename Op>
int RS<T, Op>::findRSFromROB(int robLocation)
{
	int returnVal = -1;
	for (int i = 0; i < numStations; i++)
	{
		if (stationsPtr[i].robLocation == robLocation)
		{
			returnVal = i;
			break;
		}
	}
	return returnVal;
}

template <typename T, typename Op>
int RS<T, Op>::getSize()
{
	return numStations;
}

template <typename T, typename Op>
bool RS<T, Op>::changeROBLocation(int stationNumber, int robNumber)
{
    bool returnVal = checkBounds(stationNumber) && (takenRobSpots.count(robNumber) == 0);
    if (returnVal)
    {
        stationsPtr[stationNumber].robLocation = robNumber;
        takenRobSpots.insert(robNumber);  // Insert ROB number, not station number
    }
    return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::changeOperation(int stationNumber, Op operation)
{
	bool returnVal = checkBounds(stationNumber);
	if (returnVal)
	{
		stationsPtr[stationNumber].operation = operation;
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::changeROBDependency(int stationNumber, int dep0, int dep1)
{
	bool returnVal = checkBounds(stationNumber);
	if (returnVal)
	{
		stationsPtr[stationNumber].robDependency0 = dep0;
		stationsPtr[stationNumber].robDependency1 = dep1;
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::changeROBDependency0(int stationNumber, int dep0)
{
	bool returnVal = checkBounds(stationNumber);
	if (returnVal)
	{
		stationsPtr[stationNumber].robDependency0 = dep0;
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::changeROBDependency1(int stationNumber, int dep1)
{
	bool returnVal = checkBounds(stationNumber);
	if (returnVal)
	{
		stationsPtr[stationNumber].robDependency1 = dep1;
	}
	return returnVal;
}
		
template <typename T, typename Op>
bool RS<T, Op>::changeRSVal0(int stationNumber, T val0)
{
	bool returnVal = checkBounds(stationNumber);
	if (returnVal)
	{
		stationsPtr[stationNumber].value0 = val0;
		stationsPtr[stationNumber].robDependency0 = -1;
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::changeRSVal1(int stationNumber, T val1)
{
	bool returnVal = checkBounds(stationNumber);
	if (returnVal)
	{
		stationsPtr[stationNumber].value1 = val1;
		stationsPtr[stationNumber].robDependency1 = -1;
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::compute(int stationNumber)
{
	bool returnVal = checkBounds(stationNumber);
	if (returnVal)
	{
		switch (stationsPtr[stationNumber].operation)
		{
			case EMPTY:
				std::cout << "No computation to do!\n";
				break;
			case ADD:
				stationsPtr[stationNumber].computation =
					static_cast<T>(stationsPtr[stationNumber].value0) + static_cast<T>(stationsPtr[stationNumber].value1);
				stationsPtr[stationNumber].computationDone = true;
				break;
			case SUB:
				stationsPtr[stationNumber].computation =
					static_cast<T>(stationsPtr[stationNumber].value0) - static_cast<T>(stationsPtr[stationNumber].value1);
				stationsPtr[stationNumber].computationDone = true;
				break;
			case MULT:
				stationsPtr[stationNumber].computation =
					static_cast<T>(stationsPtr[stationNumber].value0) * static_cast<T>(stationsPtr[stationNumber].value1);
				stationsPtr[stationNumber].computationDone = true;
				break;
			case DIV:
				stationsPtr[stationNumber].computation =
					static_cast<T>(stationsPtr[stationNumber].value0) / static_cast<T>(stationsPtr[stationNumber].value1);
				stationsPtr[stationNumber].computationDone = true;
				break;
			default:
				std::cout << "Invalid operation.\n";
				break;
		}
	}
	return returnVal;
}

template <typename T, typename Op>
bool RS<T, Op>::clearLocation(int stationNumber)
{
	bool returnVal = checkBounds(stationNumber) && (takenRobSpots.count(stationNumber) > 0);
	if (returnVal)
	{
		stationsPtr[stationNumber].operation = EMPTY;
		stationsPtr[stationNumber].robLocation = -1;
		stationsPtr[stationNumber].robDependency0 = -1;
		stationsPtr[stationNumber].robDependency1 = -1;
		stationsPtr[stationNumber].value0 = static_cast<T>(0);
		stationsPtr[stationNumber].value1 = static_cast<T>(0);
		stationsPtr[stationNumber].computation = static_cast<T>(0);
		stationsPtr[stationNumber].computationDone = false;
		takenRobSpots.erase(stationNumber);
	}
	return returnVal;
}

template <typename T, typename Op>
RS_type<T, Op>* RS<T, Op>::getValue(int stationNumber)
{
	return &stationsPtr[stationNumber];
}