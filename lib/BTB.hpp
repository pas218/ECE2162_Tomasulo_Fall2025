// BTB.hpp
#ifndef BTB_H
#define BTB_H

struct BTB_Entry
{
	int instrAddress;
	int predictedAddress;
	bool isTaken;
};

class BTB
{
	private:
		BTB_Entry *table;
		// Default length is 8.
		const int length = 8;
	public:
	
		BTB();
		~BTB();
		int getLength();
		int freeSpot();
		bool clearEntry(int address);
		// If you are willing to kickout another entry, make kickout = 0.
		// If you enter an address that is already used, then that will work. The previous entry will be changed.
		// The taken value is XOR'd by the boolean input.
		bool changeFullEntry(int instrAddress, int predictedAddress, bool isTaken, bool kickout);
		// The taken value is XOR'd by the boolean input.
		bool changeIsTaken(int instrAddress, bool isTaken);
		// If no entry, return address will be nullptr.
		BTB_Entry* getTableEntry(int address);
		BTB_Entry* getTableEntryByIndex(int index);
		
	
};

#endif