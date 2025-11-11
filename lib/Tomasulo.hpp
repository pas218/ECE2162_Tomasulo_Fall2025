// Tomasulo.hpp
#ifndef TOMASULO_H
#define TOMASULO_H
#include <vector>
#include "ARF.hpp"
#include "RS.hpp"
#include "RAT.hpp"
#include "ReOrderBuf.hpp"
#include "InputParser.hpp"
#include "BTB.hpp"

// A timing type that will contain the start and end cycle.
// If an instruction just goes for one cycle, then both values will be identical.
// Having both values as -1 indicated n/a for a cycle.

struct mispredict_packet{
	int timingDiagramRow;
	int robSpot;
};

struct timing_type{
	int startCycle;    
	int endCycle;
	int numROB;
	int instrNum;
	int depID0;
	int depID1;
	bool isInt;
	bool fpAdd;
	bool fpMult;
	bool isMem;
	bool isBranch;
	bool stepThisCycle;
};

struct branch_type{
	int instrLocation;
	int branchNumber;
};


enum TOMASULO_RETURN
{
	NO_ERROR,
	DONE,
	BRANCH_MISPREDICT,
	ERROR
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
		std::vector<std::vector<RAT_type>> IntRATCheckpoints;
		std::vector<std::vector<RAT_type>> FloatRATCheckpoints;
		std::vector<int> branchInstrCheckpoints; 
		int numRow;
		const int numCol = 5;
		int numberInstructions;
		const int numExInt;
		const int numExFPAdd;
		const int numExFPMult;
		const int numExLoadStore;
		const int numExBranch = 1;
		const int numMemLoadStore;
		const int numCDB;
		int PC;
		int timingDiagramPointer;
		int robPointer;
		mispredict_packet misprediction;
		ARF<int> *IntARF;
		ARF<float> *FpARF;
		RS<int, Ops> *addiRS;
		RS<float, Ops> *addfRS;
		RS<float, Ops> *mulfRS;
		RS<float, Ops> *memRS;
		RAT<int> *IntRAT; 
		RAT<float> *FpRAT;
		ReOrderBuf *ROB;
		BTB btb;
		float CDB;
		
		bool branchPredictor;
		bool done;
		int currentCycle;

    public:
		
		Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore, int numCDB,
			ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RS<float, Ops> *memRS,
			RAT<int> *IntRAT, RAT<float> *FpRAT, ReOrderBuf *ROB, std::vector<inst> &instruction, std::vector<mem_unit> *memory); // TODO: should this be "*memory" or "&memory"?
		void addMoreTimingRows(int numRow);
		// Returns -1 if does not exist.
		int findCheckpointLocation(int timingDiagramRow);
		
		// Removes all the checkpoint information including and after the branch.
		// Removes all the timing diagram information AFTER the branch (keeps the branch).
		// Returns 0 if fail.
		bool predictionRemoveEverythingAfter(int timingDiagramRow);
		bool trimDiagramEnd();
		bool eraseSingleCheckpointSpot(int timingDiagramRow);
		TOMASULO_RETURN issue();
		TOMASULO_RETURN execute();
		TOMASULO_RETURN mem();
		TOMASULO_RETURN wb();
		TOMASULO_RETURN commit();
		TOMASULO_RETURN speculationRecovery();
		void clearSteps();
		timing_type getValue(int numInstr, int numCycle);
		void printARF(bool select); // 0 for integer, 1 for float.
		void printRAT(bool select); // 0 for iteger, 1 for float.
		void printROB(int start, int finish);
		void printNonZeroRegVals();
		void printNonZeroMemVals();
		void printRS(int select); // 0 = addiRS, 1 = addfRS, mulfRS = 2
		void printBTB();
		void printOutTimingTable();
		void printOutput();
		bool fullAlgorithm();
		
};


#endif