// Tomasulo.hpp
#ifndef TOMASULO_H
#define TOMASULO_H
#include <vector>

// A timing type that will contain the start and end cycle.
// If an instruction just goes for one cycle, then both values will be identical.
// Having both values as -1 indicated n/a for a cycle.

struct timing_type{
	int startCycle;    
	int endCycle;
};

class Tomasulo
{
    private:
		// Flattend timing diagram. Use the following formula to access instr_num*numCol + stateType.
		// Issue = 0
		// Execute = 1
		// Memory = 2
		// Write back = 3
		// Commit = 4
		// So, to get instruction 1 wb column, it would be 1*4
        std::vector<timing_type> timingDiagram;
		const int numRow;
		const int numCol = 5;
		int numberInstructions
		
    public:
		
		
		bool issue();
		bool execute();
		bool mem();
		bool wb();
		bool commit();
		
};


#include "ARF.tpp"

#endif