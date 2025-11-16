// Tomasulo.hpp
#ifndef TOMASULO_H
#define TOMASULO_H
#include <vector>
#include "ARF.hpp"
#include "RS.hpp"
#include "RAT.hpp"
#include "ReOrderBuf.hpp"
#include "InputParser.hpp"

// A timing type that will contain the start and end cycle.
// If an instruction just goes for one cycle, then both values will be identical.
// Having both values as -1 indicated n/a for a cycle.

struct timing_type{
	int startCycle;    
	int endCycle;
	int numROB;
	bool isInt;
	bool fpAdd;
	bool fpMult;
	bool isMem;
	bool isBranch;
	bool isNop;
	bool stepThisCycle;
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
		std::vector<inst> instruction;
		std::vector<mem_unit> *memory; // TODO: not 100% sure if this should be memory or *memory
		
		int storeCommitInProgress;      // ROB spot of store currently committing (-1 if none)
		int storeCommitStartCycle;      // Cycle when current store commit started
		int storeCommitInstrIndex;      // Instruction index of store being committed
		int memoryBusFreeAtCycle;       // Track when the single-ported memory bus is free to be accessed.

		const int numRow;
		const int numCol = 5;
		const int numberInstructions;
		const int numExInt;
		const int numExFPAdd;
		const int numExFPMult;
		const int numExLoadStore;
		const int numMemLoadStore;
		const int numCDB;
		int PC;
		int robPointer;
		ARF<int> *IntARF;
		ARF<float> *FpARF;
		RS<int, Ops> *addiRS;
		RS<float, Ops> *addfRS;
		RS<float, Ops> *mulfRS;
		RS<float, Ops> *memRS;
		RAT<int> *IntRAT; 
		RAT<float> *FpRAT;
		ReOrderBuf *ROB;

		bool done;
		int currentCycle;

    public:
		
		Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore, int numCDB,
			ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RS<float, Ops> *memRS,
			RAT<int> *IntRAT, RAT<float> *FpRAT, ReOrderBuf *ROB, std::vector<inst> &instruction, std::vector<mem_unit> *memory); // TODO: should this be "*memory" or "&memory"?
		bool issue();
		bool execute();
		bool mem();
		bool wb();
		bool commit();
		void clearSteps();
		timing_type getValue(int numInstr, int numCycle);
		void printARF(bool select); // 0 for integer, 1 for float.
		void printRAT(bool select); // 0 for iteger, 1 for float.
		void printROB();
		void printNonZeroRegVals();
		void printNonZeroMemVals();
		void printRS(int select); // 0 = addiRS, 1 = addfRS, mulfRS = 2
		void printOutTimingTable();
		void printOutput();
		bool fullAlgorithm();
		bool allInstructionsCommitted();
		
};


#endif