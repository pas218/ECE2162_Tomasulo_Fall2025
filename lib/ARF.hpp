// ARF.hpp
#ifndef ARF_H
#define ARF_H
#include <vector>

template <typename T>
class ARF
{
    private:
        int numRegisters;
        T  *registersPtr;      // Our actual registers will be dynamically allocated to account for specified input.
		static_assert(std::is_same<T, int>::value || std::is_same<T, float>::value,
                "The ARF class can only be instantied with either type int or float.");
    public:
        ARF(); 
        ARF(int numRegistersInput);
		ARF(std::vector<T> &values);
		~ARF();
		int getSize();
        // Change the value of a register. Returns 1 if successful, otherwise return 0.
        bool changeValue(int registerNumber, T value);
        bool replaceValues(std::vector<int> registers, T value);
        T getValue(int registerNumber);
};


#include "ARF.tpp"

#endif