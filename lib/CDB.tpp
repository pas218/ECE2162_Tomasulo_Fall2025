// CDB.tpp

#include <iostream>


// There are intellisense warnings in this file, but I'm not sure why because I believe it's correct and working...?


template <typename T>
CDB_entry<T>::CDB_entry()
{
    robID = -1;
    destReg = -1;
    value = T{};
}

template <typename T>
CDB<T>::CDB()
{
    // Use a buffer size of 1 if not specified
    bufferSize = 1;
    buffersCurrentlyFilled = 0;
    entries = new CDB_entry<T>[1];
}

template <typename T>
CDB<T>::CDB(int n)
{
    bufferSize = n;
    buffersCurrentlyFilled = 0;
    entries = new CDB_entry<T>[n];
}

template <typename T>
CDB<T>::~CDB()
{
    delete[] entries;
}

// Allows a functional unit add a new broadcast to the buffer
template <typename T>
bool CDB<T>::push(int robID, int destReg, T value)
{
    if (buffersCurrentlyFilled >= bufferSize)
    {
        std::cout << "CDB Buffer full\n";
        return false;
    }

    // Fill out the CDB_entry fields
    entries[buffersCurrentlyFilled].robID = robID;
    entries[buffersCurrentlyFilled].destReg = destReg;
    entries[buffersCurrentlyFilled].value = value;

    buffersCurrentlyFilled++;
    
    return true;
}

// Removes the oldest broadcast from the buffer
template <typename T>
bool CDB<T>::pop(CDB_entry<T>& out)
{
    if (buffersCurrentlyFilled == 0)
        return false;

    // Return the first entry since it's the oldest
    out = entries[0];

    // Shift the remaining entries forward
    for (int i = 1; i < buffersCurrentlyFilled; i++)
    {
        entries[i - 1] = entries[i];
    }

    buffersCurrentlyFilled--;

    return true;
}

// Check if any of the buffers are in use
template <typename T>
bool CDB<T>::isEmpty() const
{
    return (buffersCurrentlyFilled == 0);
}
