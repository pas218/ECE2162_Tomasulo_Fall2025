#include <iostream>


template <typename T>
ARF<T>::ARF()
{
    // Arbitrarily decide 32 registers.
    numRegisters = 32;
    registersPtr = new T[32];
}

template <typename T>
ARF<T>::ARF(int numRegistersInput)
{
    numRegisters = numRegistersInput;
    registersPtr = new T(numRegistersInput);
	// Initialize all of the registers to 0.
	for (int i = 0; i < numRegistersInput; i++)
	{
		registersPtr[i] = static_cast<T>(0);
	}
}

template <typename T>
ARF<T>::ARF(std::vector<T> &values)
{
	numRegisters = values.size();
    registersPtr = new T(values.size());
	// Initialize all of the registers to 0.
	for (int i = 0; i < values.size(); i++)
	{
		registersPtr[i] = values[i];
	}
}

template <typename T>
ARF<T>::~ARF()
{
	delete[] registersPtr;
}

template <typename T>
int ARF<T>::getSize()
{
	return numRegisters;
}

template <typename T>
bool ARF<T>::changeValue(int registerNumber, T value)
{	
	bool returnVal = 0;
    if ((registerNumber >= 0) && (registerNumber < numRegisters))
    {
        registersPtr[registerNumber] = value;
		returnVal = 1;
    }
	return returnVal;
}


template <typename T>
T ARF<T>::getValue(int registerNumber)
{
	T returnVal = static_cast<T>(0);
    if ((registerNumber >= 0) && (registerNumber < numRegisters))
    {
        returnVal = registersPtr[registerNumber];
    }
	else
	{
		std::cout << "This regiser does not exist.";
	}
	return returnVal;
}