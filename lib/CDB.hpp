// CDB.hpp
#ifndef ARF_H
#define ARF_H

#include <iostream>

template <typename T>
struct CDB_entry
{
    int robID;
    int destReg;
    T value;

    CDB_entry();
};

template <typename T>
class CDB
{
private:
    int bufferSize;
    int buffersCurrentlyFilled;
    CDB_entry<T>* entries;

public:
    CDB();
    explicit CDB(int n);
    ~CDB();

    bool push(int robID, int destReg, T value);
    bool pop(CDB_entry<T>& out);
    bool isEmpty() const;
};

#include "CDB.tpp"


#endif