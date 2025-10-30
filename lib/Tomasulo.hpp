// Tomasulo.hpp
#ifndef TOMASULO_H
#define TOMASULO_H
#include <vector>
#include "ARF.hpp"
#include "RS.hpp"
#include "RAT.hpp"

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
		// So, to get instruction 1 wb column, it would be 1*5 + 3.
        std::vector<timing_type> timingDiagram;
		const int numRow;
		const int numCol = 5;
		const int numberInstructions;
		const int numExInt;
		const int numExFPAdd;
		const int numExFPMult;
		const int numExLoadStore;
		const int numMemLoadStore;

    public:
		
		Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore);
		bool issue(ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RAT<int> *IntRAT, RAT<float> *FpRAT);
		bool execute();
		bool mem();
		bool wb();
		bool commit();
		
};


#endif