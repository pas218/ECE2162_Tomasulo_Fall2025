#include "ARF.hpp"

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
}

template <typename T>
void ARF<T>::changeValue(int registerNumber, T value)
{
    if ((registerNumber > 0) && (registerNumber < numRegisters))
    {
        registersPtr[registerNumber] = value;
    }
}