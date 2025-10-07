// ARF.hpp
#ifndef ARF_H
#define ARF_H

template <typename T>
class ARF
{
    private:
        int numRegisters;
        T  *registersPtr;      // Our actual registers will be dynamically allocated to account for specified input.
    public:
        ARF(); 
        ARF(int numRegistersInput);
        // Change the value of a register. Returns 1 if successful, otherwise return 0.
        bool changeValue(int registerNumber, T value);
};


#endif