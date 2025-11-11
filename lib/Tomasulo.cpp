// Tomasulo.cpp
#include "Tomasulo.hpp"
#include <vector>

Tomasulo::Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore, int numCDB,
			ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RS<float, Ops> *memRS,
			RAT<int> *IntRAT, RAT<float> *FpRAT, ReOrderBuf *ROB, std::vector<inst> &instruction, std::vector<mem_unit> *memory)
	: numRow(numberInstructions) , numberInstructions(numberInstructions), numExInt(numExInt), numExFPAdd(numExFPAdd), numExFPMult(numExFPMult),
		numExLoadStore(numExLoadStore), numMemLoadStore(numMemLoadStore), numCDB(numCDB)
{
	this->IntARF         = IntARF;
	this->FpARF          = FpARF;
	this->addiRS         = addiRS;
	this->addfRS         = addfRS;
	this->mulfRS         = mulfRS;
	this->memRS          = memRS;
	this->IntRAT         = IntRAT;
	this->FpRAT          = FpRAT;
	this->ROB            = ROB;
	this->instruction    = instruction;
	this->memory         = memory;
	robPointer           = 0;
	done                 = 0;
	currentCycle         = 1;
	PC                   = 0;
	timingDiagramPointer = 0;
	branchPredictor      = 0;
	CDB                  = 0.0;
	
	/*
		struct timing_type{
	int startCycle;    
	int endCycle;
	int numROB;
	int instrNum;
	bool isInt;
	bool fpAdd;
	bool fpMult;
	bool isMem;
	bool isBranch;
	bool stepThisCycle;
};
	*/
	for(int i = 0; i < numRow*numCol; i++)
	{
		timing_type temp;
		temp.startCycle    = 0;
		temp.endCycle      = 0;
		temp.numROB        = 0;
		temp.instrNum      = 0;
		temp.isInt         = 0;
		temp.fpAdd         = 0;
		temp.isMem         = 0;
		temp.isBranch      = 0;
		temp.stepThisCycle = 0;
		timingDiagram.push_back(temp);
	}
}


void Tomasulo::addMoreTimingRows(int extraRows)
{
	numRow += extraRows;
	for(int i = 0; i < extraRows*numCol; i++)
	{
		timing_type temp;
		temp.startCycle    = 0;
		temp.endCycle      = 0;
		temp.numROB        = 0;
		temp.instrNum      = 0;
		temp.isInt         = 0;
		temp.fpAdd         = 0;
		temp.isMem         = 0;
		temp.isBranch      = 0;
		temp.stepThisCycle = 0;
		timingDiagram.push_back(temp);
	}
}

int Tomasulo::findCheckpointLocation(int timingDiagramRow)
{
	std::cout << "Find checkpoint location.\n";
	std::cout << "timingDiagramRow: " << timingDiagramRow << std::endl;
	// Remove related to checkpoints.
	int position = -1;
	for (int i = 0; i < branchInstrCheckpoints.size(); i++)
	{
		std::cout << "Checking index #" << i << std::endl;
		std::cout << "Vector value at index #" << i << ": " << branchInstrCheckpoints[i] << std::endl;
		if (branchInstrCheckpoints[i] == timingDiagramRow)
		{
			position = i;
			break;
		}
	}
	return position;
}

bool Tomasulo::predictionRemoveEverythingAfter(int timingDiagramRow)
{
	bool returnVal = 0;
	int position = findCheckpointLocation(timingDiagramRow);
	if (position == -1)
	{
		return returnVal;
	}
	returnVal = 1;
	
	IntRATCheckpoints.erase(IntRATCheckpoints.begin()+position, IntRATCheckpoints.end()-1);
	FloatRATCheckpoints.erase(FloatRATCheckpoints.begin()+position, FloatRATCheckpoints.end()-1);
	branchInstrCheckpoints.erase(branchInstrCheckpoints.begin()+position, branchInstrCheckpoints.end()-1);
	
	// Remove everything after the branch instruction in the timing diagram.
	for (int i = timingDiagramRow+1; i < numRow; i++)
	{
		for (int j = 0; j < numCol; j++)
		{
			
			timing_type temp;
			temp.startCycle           = 0;
			temp.endCycle             = 0;
			temp.numROB               = 0;
			temp.instrNum             = 0;
			temp.isInt                = 0;
			temp.fpAdd                = 0;
			temp.isMem                = 0;
			temp.isBranch             = 0;
			temp.stepThisCycle        = 0;
			timingDiagram[i*numCol+j] = temp;
		}
	}
	return returnVal;
}

bool Tomasulo::trimDiagramEnd()
{
	bool returnVal = 0;
	// Loop from the back of the timing diagram.
	for (int i = timingDiagram.size()-1; i >= 0; i = i - numCol)
	{
		if ((timingDiagram[i].startCycle == 0) && (timingDiagram[i].endCycle == 0))
		{
			// Remove one row at a time.
			for (int j = 0; j < numCol; j++)
			{
				timingDiagram.pop_back();
			}
			numRow--;
		}
		else{
			break;
		}
	}
	returnVal = 1;
	return returnVal;
}
bool Tomasulo::eraseSingleCheckpointSpot(int timingDiagramRow)
{
	bool returnVal = 0;
	int position = findCheckpointLocation(timingDiagramRow);
	if (position == -1)
	{
		return returnVal;
	}

	IntRATCheckpoints.erase(IntRATCheckpoints.begin()+position);
	FloatRATCheckpoints.erase(FloatRATCheckpoints.begin()+position);
	branchInstrCheckpoints.erase(branchInstrCheckpoints.begin()+position);

	returnVal = 1;
	return returnVal;
}
// Classes we will need for the issue cycle:
// ARF
// RAT
// ROB
// RS
// Instructions
TOMASULO_RETURN Tomasulo::issue()
{
	//std::cout << std::endl;
	//std::cout << "Issue()\n" << std::endl;
	//std::cout << "Program counter: " << PC << std::endl;
	//std::cout << "Timing diagram pointer: " << timingDiagramPointer << std::endl;
	TOMASULO_RETURN success = ERROR;
	
	// If the PC counter is higher than the number of instructions, and the last instruction has commited, the program is done.
	if (PC >= numberInstructions)
	{
		success = DONE;
		return success;
	}
	
	// Use program counter to find new instruction.
	inst &ins = instruction[PC];
	///// SECTION 1: JUST CHECK FOR ISSUE VIABILITY
	
	//std::cout << "PC: " << PC << std::endl;
	//std::cout << "Opcode: " << ins.opcode << std::endl;

	// Map the op code to an instruction operation type
	// operationType: 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, 5 = NOP
	int operationType;
	Ops physicalOperation;
	if (ins.opcode == add || ins.opcode == addi)
	{
		operationType = 0;
		physicalOperation = ADD;
	}
	else if(ins.opcode == sub)
	{
		operationType = 0;
		physicalOperation = SUB;
	}
	else if(ins.opcode == addf)
	{
		operationType = 1;
		physicalOperation = ADD;
	}
	else if(ins.opcode == subf)
	{
		operationType = 1;
		physicalOperation = SUB;
	}
	else if(ins.opcode == mulf)
	{
		operationType = 2;
		physicalOperation = MULT;
	}
	else if (ins.opcode == load || ins.opcode == store)
	{
		operationType = 3;
	}
	else if (ins.opcode == beq || ins.opcode == bne)
	{
		operationType = 4;
	}
	else
	{
		operationType = 5;
	}
	// std::cout << "Operation type: " << operationType << std::endl;

	// Find a free RS entry for the instruction
	// operationType: 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, 5 = NOP
	int freeRSSpot; 
	if (operationType == 0)
	{
		// Integer add or subtract.
		freeRSSpot = addiRS->freeSpot();
	}
	else if (operationType == 1)
	{
		// Floating point add or subtract.
		freeRSSpot = addfRS->freeSpot();
	}
	else if (operationType == 2)
	{
		// Floating point multiply.
		freeRSSpot = mulfRS->freeSpot();
	}
	else if (operationType == 3)
	{
		// Memory (Load/Store).
		freeRSSpot = memRS->freeSpot();
	}
	else if (operationType == 4)
	{
		// Branch predictor uses the integer ALU.
		freeRSSpot = addiRS->freeSpot();
	}
	else
	{
		// NOP.
	}
	// SECTION 2: ACTUALLY PERFORM ISSUE, ASSUMING THERE IS VIABILITY
	
	// Write the to ROB, RS, RAT, and timing table.
	// operationType: 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, 5 = NOP
	int freeROBSpot = ROB->freeSpot();
	std::cout << "Head pointer: " << ROB->headPtr << std::endl;
	std::cout << "Tail pointer: " << ROB->tailPtr << std::endl;
	std::cout << "Free ROB Spot\n";
	bool branchPrediction = false;
	if (freeRSSpot != -1)
	{
		// ROB.
		if ((operationType == 0) || (operationType == 4))
		{
			ROB->table[freeROBSpot].id = 0;
		}
		else if ((operationType == 1) || (operationType == 2) || (operationType == 3))
		{
			ROB->table[freeROBSpot].id = 1;
		}
		
		// Don't add a register ID for branching.
		if (operationType != 4)
		{
			ROB->table[freeROBSpot].dst_id = ins.rd.id;
		}
		else if (operationType == 4)
		{
			ROB->table[freeROBSpot].dst_id = -2;
		}
		ROB->headPtr++;
		
		// RS.
		if (operationType == 0)
		{
			// Integer add or subtract.
			addiRS->changeROBLocation(freeRSSpot, freeROBSpot);
			addiRS->changeOperation(freeRSSpot, physicalOperation);
			
			
			// Change dependencies/ values 0;
			int intDependency = 0;
			
			int regID0 = ins.rs.id;
			int robDep0 = ROB->findDependency(intDependency, regID0);
			std::cout << "Dep0: " << robDep0 << std::endl;
			if (robDep0 != -1)
			{
				addiRS->changeROBDependency0(freeRSSpot, robDep0);
			}
			else
			{
				addiRS->changeRSVal0(freeRSSpot, IntARF->getValue(regID0));
			}
			
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(intDependency, regID1);
			std::cout << "Dep1: " << robDep1 << std::endl;
			if (robDep1 != -1)
			{
				addiRS->changeROBDependency1(freeRSSpot, robDep1);
			}
			else
			{
				addiRS->changeRSVal1(freeRSSpot, IntARF->getValue(regID1));
			}
			
			IntRAT->changeValue(ins.rd.id, 0, freeROBSpot);
			
		}
		else if (operationType == 1)
		{
			// Floating point add or subtract.
			addfRS->changeROBLocation(freeRSSpot, freeROBSpot);
			addfRS->changeOperation(freeRSSpot, physicalOperation);
			
			// Change dependencies/ values 0;
			int floatDependency = 1;
			
			int regID0 = ins.rs.id;
			int robDep0 = ROB->findDependency(floatDependency, regID0);
			if (robDep0 != -1)
			{
				addfRS->changeROBDependency0(freeRSSpot, robDep0);
			}
			else
			{
				addfRS->changeRSVal0(freeRSSpot, FpARF->getValue(regID0));
			}
			
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(floatDependency, regID1);
			if (robDep1 != -1)
			{
				addfRS->changeROBDependency1(freeRSSpot, robDep1);
			}
			else
			{
				addfRS->changeRSVal1(freeRSSpot, FpARF->getValue(regID1));
			}
			
			FpRAT->changeValue(ins.rd.id, 0, freeROBSpot);
			
		}
		else if (operationType == 2)
		{
			// Floating point multiply.
			mulfRS->changeROBLocation(freeRSSpot, freeROBSpot);
			mulfRS->changeOperation(freeRSSpot, physicalOperation);
			
			// Change dependencies/ values 0;
			int floatDependency = 1;
			
			int regID0 = ins.rs.id;
			int robDep0 = ROB->findDependency(floatDependency, regID0);
			if (robDep0 != -1)
			{
				mulfRS->changeROBDependency0(freeRSSpot, robDep0);
			}
			else
			{
				mulfRS->changeRSVal0(freeRSSpot, FpARF->getValue(regID0));
			}
			
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(floatDependency, regID1);
			if (robDep1 != -1)
			{
				mulfRS->changeROBDependency1(freeRSSpot, robDep1);
			}
			else
			{
				mulfRS->changeRSVal1(freeRSSpot, FpARF->getValue(regID1));
			}
			
			FpRAT->changeValue(ins.rd.id, 0, freeROBSpot);
			
		}
		else if (operationType == 3)
		{
			// Memory operations (Load/Store).
			memRS->changeROBLocation(freeRSSpot, freeROBSpot);
			
			// For loads and stores, we need to calculate the effective address
			// Address = base register value + offset
			// The base register is rs, offset is in rt.value
			
			if (ins.opcode == load)
			{
				// Address calculation is an add that does not occupy the integer ALU (per the project description)
				memRS->changeOperation(freeRSSpot, ADD);
				
				// Check if base register has a dependency
				int intDependency = 0;
				int regID0 = ins.rs.id;
				int robDep0 = ROB->findDependency(intDependency, regID0);
				
				if (robDep0 != -1)
				{
					memRS->changeROBDependency0(freeRSSpot, robDep0);
				}
				else
				{
					// Base address from register
					memRS->changeRSVal0(freeRSSpot, static_cast<float>(IntARF->getValue(regID0)));
				}
				
				// Offset is an immediate value
				memRS->changeRSVal1(freeRSSpot, ins.rt.value);
				
				FpRAT->changeValue(ins.rd.id, 0, freeROBSpot);
			}
			else if (ins.opcode == store)
			{
				// Address calculation is an add that does not occupy the integer ALU (per the project description)
				memRS->changeOperation(freeRSSpot, ADD);
				
				// Check if base register has a dependency
				int intDependency = 0;
				int regID0 = ins.rd.id;
				int robDep0 = ROB->findDependency(intDependency, regID0);
				
				if (robDep0 != -1)
				{
					memRS->changeROBDependency0(freeRSSpot, robDep0);
				}
				else
				{
					// Base address from register
					memRS->changeRSVal0(freeRSSpot, static_cast<float>(IntARF->getValue(regID0)));
				}
				
				// Offset
				memRS->changeRSVal1(freeRSSpot, ins.rt.value);
				
				// Store also needs to track data value dependency. The data to store comes from rs register,
				// and we need to make sure this data is ready before the store can commit
				int floatDependency = 1;
				int dataRegID = ins.rs.id;
				int dataDep = ROB->findDependency(floatDependency, dataRegID);
				
				// Store the data dependency in ROB for checking at commit time

				// TODO: I suspect there is something wrong or missing here
			}
		}
		
		// For branch need to do the BTB as well. 
		else if (operationType == 4)
		{
			// Branch takes up space in the integer RS.
			addiRS->changeROBLocation(freeRSSpot, freeROBSpot);
			// Change dependencies/ values 0;
			int intDependency = 0;
			
			int regID0 = ins.rs.id;
			int robDep0 = ROB->findDependency(intDependency, regID0);
			std::cout << "Found rob dep0: " << robDep0 << std::endl;
			if (robDep0 != -1)
			{
				addiRS->changeROBDependency0(freeRSSpot, robDep0);
			}
			else
			{
				addiRS->changeRSVal0(freeRSSpot, IntARF->getValue(regID0));
			}
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(intDependency, regID1);
			std::cout << "Found rob dep1: " << robDep1 << std::endl;
			if (robDep1 != -1)
			{
				addiRS->changeROBDependency1(freeRSSpot, robDep1);
			}
			else
			{
				addiRS->changeRSVal1(freeRSSpot, IntARF->getValue(regID1));
			}
			// Need to convert the PC to word address.
			// Only use the last three bits to track the address.
			// Do not change the predicted value, and kickout a branch if need be.
			// If the entry does not exist, add it.
			BTB_Entry* result = btb.getTableEntry((PC*4) & 0b111);
			if (result == NULL)
			{
				btb.changeFullEntry((PC*4) & 0b111, PC+1+static_cast<int>(ins.rt.value), 0, 1);
			}
			std::cout << "GETTING TABLE ENTRY!\n";
			// Get the branch prediction.
			branchPrediction = btb.getTableEntry((PC*4) & 0b111)->isTaken;
			std::cout << "PREDICTED: " << (int)branchPrediction << std::endl;
			// Add the actual operation into the RS.
			if (ins.opcode == beq)
			{
				addiRS->changeOperation(freeRSSpot, BEQ);
			}
			else if (ins.opcode == bne)
			{
				addiRS->changeOperation(freeRSSpot, BNE);
			}
		} 
		
		// If there is no space in the timing diagram, add it.
		if (timingDiagramPointer >= numRow)
		{
			// Just add one more row.
			addMoreTimingRows(1);
		}
		// Update the timing diagram.
		timingDiagram[timingDiagramPointer*numCol + 0].startCycle    = currentCycle;
		timingDiagram[timingDiagramPointer*numCol + 0].endCycle      = currentCycle;
		timingDiagram[timingDiagramPointer*numCol + 0].numROB   	 = freeROBSpot;
		timingDiagram[timingDiagramPointer*numCol + 0].instrNum      = PC;
		timingDiagram[timingDiagramPointer*numCol + 0].stepThisCycle = true;
		
		if (!(operationType == 3) && !(operationType == 4))
		{
			timingDiagram[timingDiagramPointer*numCol + 0].depID0 = ins.rs.id;
			timingDiagram[timingDiagramPointer*numCol + 0].depID1 = ins.rt.id;
		}
		
		
		// Change the flag type in the timing diagram depending on operation type.
		// operationType: 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, 5 = NOP
		if (operationType == 0)
		{
			timingDiagram[timingDiagramPointer*numCol + 0].isInt = true;
		}
		else if (operationType == 1)
		{
			timingDiagram[timingDiagramPointer*numCol + 0].fpAdd = true;
		}
		else if (operationType == 2)
		{
			timingDiagram[timingDiagramPointer*numCol + 0].fpMult = true;
		}
		else if (operationType == 3)
		{
			timingDiagram[timingDiagramPointer*numCol + 0].isMem = true;
		}
		else if (operationType == 4)
		{
			timingDiagram[timingDiagramPointer*numCol + 0].isBranch = true;
		}
		
		// Change the PC accordingly.
		if ((timingDiagram[timingDiagramPointer*numCol + 0].isBranch == true))
		{
			//std::cout << "TAKING BRANCH ----------------------------------------.\n";
			// Push the RAT and keep track of the branch number.
			std::vector<RAT_type> tempIntVector = IntRAT->exportRAT();
			std::vector<RAT_type> tempFloatVector = FpRAT->exportRAT();
			IntRATCheckpoints.push_back(tempIntVector);
			FloatRATCheckpoints.push_back(tempFloatVector);
			branchInstrCheckpoints.push_back(timingDiagramPointer);
			// If branch, set the PC to the predicted address.
			PC = btb.getTableEntry((PC*4) & 0b111)->predictedAddress;
		}
		else{
			// If not branch, increment normally.
			PC++;
		}
		//std::cout << "Seven.\n";
		// Increase timing diagram pointer.
		timingDiagramPointer++;
		
		success = NO_ERROR;
	}
	
	
	
	
	return success;
}


// Classes we will need for execute stage
TOMASULO_RETURN Tomasulo::execute()
{
	//std::cout << "Execute()\n" << std::endl;
	// std::cout << "EX cycle: " << currentCycle << std::endl;
	TOMASULO_RETURN success = ERROR;

	// This loop is to START execution.
    for (size_t h = 0; h < numRow; ++h) 
	{  
		// Find next instruction that has been issued, but not executed.
		if (!((timingDiagram[h*numCol + 0].endCycle != 0) && (timingDiagram[h*numCol + 1].startCycle == 0)
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false)))
		{
			continue;
		}

		std::cout << "Starting to execute instruction #" << h << std::endl;
		// See if there are any unaddressed dependencies.
		bool unaddressedDeps = true;
		int robSpot = timingDiagram[h*numCol + 0].numROB;
		std::cout << "robSpot: " << robSpot << std::endl;
		
		// Check to see if any of the dependencies have already been written back. In this case, we will have to take directly from the ARF.
		
		// Both the integer and branch units use the adder reservation station.
		
		// When the dependency can no longer be found in the rob, there are two cases. 1) The dependency wrote back literally last cycle. In that case we need to take from the CDB.
		// 2) The dependency already commited. In that case we need to read from the ARF.
					
		if ((timingDiagram[h*numCol + 0].isInt == true) || (timingDiagram[h*numCol + 0].isBranch == true))
		{
			std::cout << "Starting int or branch.\n";
			
			// If the dependency no longer exists in the ROB, that means it might have already been commited.
			// Go to the ARF to get those values.
			int RSSpot = addiRS->findRSFromROB(robSpot);
			int dep0 = addiRS->getValue(RSSpot)->robDependency0;
			int dep1 = addiRS->getValue(RSSpot)->robDependency1;
			unaddressedDeps = addiRS->hasUnaddressedDependencies(robSpot);
			
			if (unaddressedDeps == true)
			{
				if (ROB->table[dep0].id == -1)
				{
					if (timingDiagram[dep0*numCol + 4].startCycle == 0)
					{
						addiRS->changeRSVal0(RSSpot, static_cast<int>(CDB));
					}
					else
					{
						addiRS->changeRSVal0(RSSpot, IntARF->getValue(timingDiagram[dep0*numCol+0].depID0));
					}
				}
				if (ROB->table[dep1].id == -1)
				{
					if (timingDiagram[dep1*numCol + 4].startCycle == 0)
					{
						addiRS->changeRSVal1(RSSpot, static_cast<int>(CDB));
					}
					else
					{
						addiRS->changeRSVal1(RSSpot, IntARF->getValue(timingDiagram[dep1*numCol+0].depID1));
					}
				}
			}
			
			// Recalculate.
			unaddressedDeps = addiRS->hasUnaddressedDependencies(robSpot);
		}
		else if (timingDiagram[h*numCol + 0].fpAdd == true)
		{
			std::cout << "Starting fp add.\n";
			int RSSpot = addfRS->findRSFromROB(robSpot);
			int dep0 = addfRS->getValue(RSSpot)->robDependency0;
			int dep1 = addfRS->getValue(RSSpot)->robDependency1;
			unaddressedDeps = addfRS->hasUnaddressedDependencies(robSpot);
			
			// If already commited, get the value from the ARF;
			if (unaddressedDeps == true)
			{
				if (ROB->table[dep0].id == -1)
				{
					if (timingDiagram[dep0*numCol + 4].startCycle == 0)
					{
						addfRS->changeRSVal0(RSSpot, CDB);
					}
					else{
						addfRS->changeRSVal0(RSSpot, FpARF->getValue(timingDiagram[dep0*numCol+0].depID0));
					}
				}
				if (ROB->table[dep1].id == -1)
				{
					if (timingDiagram[dep1*numCol + 4].startCycle == 0)
					{
						addfRS->changeRSVal1(RSSpot, CDB);
					}
					else
					{
						addfRS->changeRSVal1(RSSpot, FpARF->getValue(timingDiagram[dep1*numCol+0].depID1));
					}
				}
			}
			
			// Recalculate.
			unaddressedDeps = addfRS->hasUnaddressedDependencies(robSpot);
		}
		else if (timingDiagram[h*numCol + 0].fpMult == true)
		{
			std::cout << "Starting fp mult.\n";
			int RSSpot = mulfRS->findRSFromROB(robSpot);
			int dep0 = mulfRS->getValue(RSSpot)->robDependency0;
			int dep1 = mulfRS->getValue(RSSpot)->robDependency1;
			unaddressedDeps = mulfRS->hasUnaddressedDependencies(robSpot);
			
			if (unaddressedDeps == true)
			{
				if (ROB->table[dep0].id == -1)
				{
					if (timingDiagram[dep0*numCol + 4].startCycle == 0)
					{
						mulfRS->changeRSVal0(RSSpot, CDB);
					}
					else
					{
						mulfRS->changeRSVal0(RSSpot, FpARF->getValue(timingDiagram[dep0*numCol+0].depID0));
					}
				}
				
				if (ROB->table[dep1].id == -1)
				{
					if (timingDiagram[dep1*numCol + 4].startCycle == 0)
					{
						mulfRS->changeRSVal1(RSSpot, CDB);
					}
					else
					{
						mulfRS->changeRSVal1(RSSpot, FpARF->getValue(timingDiagram[dep1*numCol+0].depID1));
					}
				}
			}
			
			// Recalculate.
			unaddressedDeps = mulfRS->hasUnaddressedDependencies(robSpot);
		}
		else if (timingDiagram[h*numCol + 0].isMem == true)
		{
			std::cout << "Starting memory.\n";
			int RSSpot = memRS->findRSFromROB(robSpot);
			int dep0 = memRS->getValue(RSSpot)->robDependency0;
			int dep1 = memRS->getValue(RSSpot)->robDependency1;
			unaddressedDeps = memRS->hasUnaddressedDependencies(robSpot);
		}
		/*
		else if (timingDiagram[h*numCol + 0].isBranch == true)
		{
			int RSSpot = addiRS->findRSFromROB(robSpot);
			int dep0 = addiRS->getValue(RSSpot)->robDependency0;
			int dep1 = addiRS->getValue(RSSpot)->robDependency1;
			unaddressedDeps = addiRS->hasUnaddressedDependencies(robSpot);
			std::cout << "Exec RSSpot: " << RSSpot << std::endl;
			std::cout << "Exec robSpot: " << robSpot << std::endl;
			std::cout << "Exec dep0: " << dep0 << std::endl;
			std::cout << "Exec dep1: " << dep1 << std::endl;
			std::cout << "Exec unaddressedDeps: " << (int)unaddressedDeps << std::endl;
			if (unaddressedDeps == true)
			{
				if (ROB->table[dep0].id == -1)
				{
					std::cout << "Two.\n";
					if (timingDiagram[dep0*numCol + 4].startCycle == 0)
					{
						addiRS->changeRSVal0(RSSpot, static_cast<int>(CDB));
					}
					else{
						addiRS->changeRSVal0(RSSpot, IntARF->getValue(timingDiagram[dep0*numCol+0].depID0);
					}
				}
				if (ROB->table[dep1].id == -1)
				{
					std::cout << "Three.\n";
					addiRS->changeRSVal1(RSSpot, static_cast<int>(CDB));
				}
			}
			
			// Recalculate.
			unaddressedDeps = addiRS->hasUnaddressedDependencies(robSpot);
		}
		*/
		std::cout << "Unaddresssed dependencies: " << (int)unaddressedDeps << std::endl;
		
		if (unaddressedDeps == true)
		{	
			continue;
		}
		
		// Write down the start cycle for execute
		timingDiagram[h*numCol + 1].startCycle = currentCycle;
		
		// Only do this once.
		break;
		
		success = NO_ERROR;
	}
	
	// Find next instruction that has started executing, but not finished.
	// This loop is to END execution.
    for (size_t h = 0; h < numRow; ++h) 
	{
		//std::cout << "Execute() end check row #" << h << std::endl;
		
		// Find instruction that has been issued, but not executed.
		if (!((timingDiagram[h*numCol + 1].startCycle != 0) && (timingDiagram[h*numCol + 1].endCycle == 0)))
		{
			continue;
		}
		
		std::cout << "Continuing to execute instruction #" << h << std::endl;
		//std::cout << "Insdie Execute(0 end with row #" << h << std::endl;
		
		// Get the ROB spot.
		int robSpot = timingDiagram[h*numCol + 0].numROB;
		
		//std::cout << "Check the type.\n";
		// Check the instruction type
		inst &ins = instruction[h];
		success = NO_ERROR;
		if (timingDiagram[h*numCol + 0].isInt == true)
		{
			//std::cout << "Integer.\n";
			// Integer type.
			// See if finished executing.
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExInt)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
				std::cout << "Finishing to execute instruction (isInt) #" << h << std::endl;
			}
			
		}
		else if (timingDiagram[h*numCol + 0].fpAdd == true)
		{
			//std::cout << "Fp Adder.\n";
			// Fp type add.
			
			// See if finished executing.
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExFPAdd)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
				std::cout << "Finishing to execute instruction (fpAdd) #" << h << std::endl;
			}
			
		}
		else if (timingDiagram[h*numCol + 0].fpMult == true)
		{
			// std::cout << "Fp Mult.\n";
			// Fp type mult.
			
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExFPMult)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
				std::cout << "Finishing to execute instruction (fpMult) #" << h << std::endl;
			}
		}
		else if (timingDiagram[h*numCol + 0].isMem == true)
		{
			// std::cout << "Memory.\n";
			// Memory (Load/Store)
			
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExLoadStore)
			{
				std::cout << "Ending memory instruction.";
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
				
				// Compute the effective address as part of execute
				int RSSpot = memRS->findRSFromROB(robSpot);
				if (RSSpot != -1)
				{
					memRS->compute(RSSpot); // This calculates address = base + offset
				}
			}
		}
		else if (timingDiagram[h*numCol +0].isBranch  == true)
		{
			// Branch (Bne and Beq)
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExBranch)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
				
				printRS(0);
				// Compute whether branch is actually taken or not.
				int RSSpot = addiRS->findRSFromROB(robSpot);
				if (RSSpot != -1)
				{
					addiRS->compute(RSSpot); // This calculates the branch decision
				}
			
				std::cout << "Actual branch value: " << (int)addiRS->getValue(RSSpot)->takeBranch << std::endl;
				std::cout << "Earlier predicted value: " << (int)btb.getTableEntry((timingDiagram[h*numCol+0].instrNum*4) & 0b111)->isTaken << std::endl;
				// Check if the real branch decision agrees with the predicted.
				if ((addiRS->getValue(RSSpot)->takeBranch) != (btb.getTableEntry((timingDiagram[h*numCol+0].instrNum*4) & 0b111)->isTaken))
				{
					misprediction.timingDiagramRow = h;
					misprediction.robSpot          = robSpot;

					btb.changeIsTaken((timingDiagram[h*numCol+0].instrNum*4) & 0b111, 1);
					success = BRANCH_MISPREDICT;

				}
				addiRS->clearLocation(RSSpot);
				ROB->table[robSpot].cmt_flag = 1;
			}
		}
		
	}

	return success;
}

TOMASULO_RETURN Tomasulo::mem()
{
	// std::cout << "Mem()\n" << std::endl;
    TOMASULO_RETURN success = ERROR;
    
	// Record mem cycle for table
	// Check for "forwarding-from-a-store" as mentioned in project document. It takes 1 cycle to perform the forwarding if a match is found. If not found then the load accesses the memory for data.
	// Once a load returns from the memory or gets the value from a previous store, its entry in the load/store queue is cleared.
	// Note that it is correct not to clear this entry, but the queue can quickly fill up, causing structure hazards for future loads/stores.    

    // Stores don't actually write to memory here - they write in commit stage
    // Loads can access memory or forward from a store
    
 	// Find instructions that have completed EX stage but not completed MEM stage
    for (size_t h = 0; h < numRow; ++h)
    {
        // Check if this instruction needs MEM stage
        if (!((timingDiagram[h*numCol + 0].isMem == true) &&
              (timingDiagram[h*numCol + 1].endCycle != 0) &&
              (timingDiagram[h*numCol + 2].endCycle == 0) &&
              (timingDiagram[h*numCol + 0].stepThisCycle == false)))
        {
            continue;
        }
        
        //std::cout << std::endl << "Made it past the --Check if this instruction needs MEM stage-- continue statement" << std::endl;

        inst &ins = instruction[h];
        int ROBSpot = timingDiagram[h*numCol + 0].numROB;
        int RSSpot = memRS->findRSFromROB(ROBSpot);
        
        if (RSSpot == -1)
        {
			// RS entry not found
            continue;
        }
        
        // Get the computed effective address
        float effectiveAddress = memRS->getValue(RSSpot)->computation;
        int address = static_cast<int>(effectiveAddress);
       
        if (ins.opcode == load)
        {
            // Load: Check for store-to-load forwarding first
            bool forwardedFromStore = false;
            
            // Only check forwarding if we haven't started MEM yet
            if (timingDiagram[h*numCol + 2].startCycle == 0)
            {
                // Search for older stores (earlier in program order) that match this address
                for (size_t i = 0; i < h; ++i)
                {
                    inst &prevInst = instruction[i];
                    
                    if (prevInst.opcode == store)
                    {
                        int prevROBSpot = timingDiagram[i*numCol + 0].numROB;
                        int prevRSSpot = memRS->findRSFromROB(prevROBSpot);
                        
                        // Check if the store has calculated its address
                        if (prevRSSpot != -1 && 
                            timingDiagram[i*numCol + 1].endCycle != 0 &&
                            memRS->getValue(prevRSSpot)->computationDone)
                        {
                            float prevAddress = memRS->getValue(prevRSSpot)->computation;
                            
                            // If addresses match, forward from the store
                            if (static_cast<int>(prevAddress) == address)
                            {
                                // Forward the store's data value
                                // The store data is in rs register for stores
                                int storeDataReg = prevInst.rs.id;
                                float forwardedValue = FpARF->getValue(storeDataReg);
                                
                                // Write forwarded value to ROB
                                ROB->table[ROBSpot].value = forwardedValue;
                                
                                forwardedFromStore = true;
                                
                                // Forwarding takes 1 cycle
                                timingDiagram[h*numCol + 2].startCycle = currentCycle;
                                timingDiagram[h*numCol + 2].endCycle = currentCycle;
                                timingDiagram[h*numCol + 0].stepThisCycle = true;
                                
                                // Clear the load from memRS
                                memRS->clearLocation(RSSpot);
                                
                                success = NO_ERROR;
                                break;
                            }
                        }
                    }
                }
            }
            
            // If no forwarding, access memory like normal
            if (!forwardedFromStore)
            {
                //std::cout << std::endl << "Made it into access memory step" << std::endl;

                // Check if we've already started memory access
                if (timingDiagram[h*numCol + 2].startCycle == 0)
                {
                    // Start memory access at this cycle
                    timingDiagram[h*numCol + 2].startCycle = currentCycle;
                }
                
                // Check if memory access is complete
                if ((currentCycle - timingDiagram[h*numCol + 2].startCycle + 1) >= numMemLoadStore)
                {
                    //std::cout << std::endl << "Memory access complete - read value from memory" << std::endl;

                    // Memory access complete - read value from memory
                    float memValue = 0.0f;
                    
                    // Search for the address in memory vector
                    for (const auto &memEntry : *memory)
                    {
                        if (memEntry.first == address)
                        {
                            memValue = memEntry.second;
                            break;
                        }
                    }
                    
                    // Write value to ROB
                    ROB->table[ROBSpot].value = memValue;
                    
                    timingDiagram[h*numCol + 2].endCycle = currentCycle;
                    timingDiagram[h*numCol + 0].stepThisCycle = true;
                    
                    // Clear the load from memRS (loads can be cleared after MEM stage)
                    memRS->clearLocation(RSSpot);
                    
                    success = NO_ERROR;
                }
            }
        }
        else if (ins.opcode == store)
        {
            // Store: Record that MEM stage has happened
            // Stores don't write to memory until commit stage, but we need to record the MEM cycle for the table
            timingDiagram[h*numCol + 2].startCycle = currentCycle;
            timingDiagram[h*numCol + 2].endCycle = currentCycle;
            timingDiagram[h*numCol + 0].stepThisCycle = true;
            
            // Store the address in ROB for later use in commit
            ROB->table[ROBSpot].value = effectiveAddress;
            
            // Note: Don't clear memRS for stores yet (they stay until commit stage)
            
            success = NO_ERROR;
        }
        
        // Only process one instruction per cycle
        break;
    }
    
    return success;
}

TOMASULO_RETURN Tomasulo::wb()
{
	//std::cout << "wb()\n" << std::endl;
	TOMASULO_RETURN success = ERROR;

	//std::cout << "WB cycle: " << currentCycle << std::endl;

	// Broadcast results onto the CDB (other instructions waiting on it will need to pick it up), or buffer if the CDB is full
	// Write result back to RS and ROB entry (record WB cycle for table)
	// Mark the ready/finished bit in ROB since the instruction has completed execution
	// Free reservation stations for future reuse when finished
	// Store instructions write to memory in this stage
	
	// std::cout << "Starting wb().\n";
	// std::cout << "Number CDB: " << numCDB << std::endl;

	// Assume CDB is size 1.
	if (numCDB == 1)
	{
		//std::cout << "Inside numCBD if statement.\n";

		// Find instruction that has completed execution cycle and (if load/store) memory as well.
		for (size_t h = 0; h < numRow; ++h) 
		{
			//std::cout << "wb loop " << h << std::endl;


			/*
			// Find instruction that has been issued, but not executed.
			if (!((timingDiagram[h*numCol + 1].endCycle != 0) && (timingDiagram[h*numCol + 3].startCycle == 0) 
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false)))
			{
				continue;
			}
			*/

			bool canWriteback = false;
			
			// For memory instructions, check if MEM stage is complete
			if (timingDiagram[h*numCol + 0].isMem == true)
			{
				canWriteback = (timingDiagram[h*numCol + 2].endCycle != 0) && 
				               (timingDiagram[h*numCol + 3].startCycle == 0) &&
				               (timingDiagram[h*numCol + 0].stepThisCycle == false);
			}
			// Skip branch instructions.
			else if (timingDiagram[h*numCol + 0].isBranch == true)
			{
				continue;
			}
			// For non-memory instructions, check if EX stage is complete
			else
			{
				canWriteback = (timingDiagram[h*numCol + 1].endCycle != 0) && 
				               (timingDiagram[h*numCol + 3].startCycle == 0) &&
				               (timingDiagram[h*numCol + 0].stepThisCycle == false);
			}
			
			if (!canWriteback)
			{
				continue;
			}

			
			//std::cout << "In wb.\n";

			int ROBSpot = timingDiagram[h*numCol + 0].numROB;
			int RSSpot;

			if (timingDiagram[h*numCol + 0].isInt == true)
			{
				RSSpot = addiRS->findRSFromROB(ROBSpot);
				if (RSSpot != -1)
				{
					//std::cout << "Wb inInt: " << currentCycle << std::endl;

					addiRS->compute(RSSpot);
					CDB = static_cast<float>(addiRS->getValue(RSSpot)->computation);
					ROB->table[ROBSpot].value = static_cast<float>(addiRS->getValue(RSSpot)->computation);
					ROB->table[ROBSpot].cmt_flag = 1;
					addiRS->clearLocation(RSSpot);

				}
			}
			else if (timingDiagram[h*numCol + 0].fpAdd == true)
			{
				RSSpot = addfRS->findRSFromROB(ROBSpot);

				//std::cout << "WB, FpAdd, RSSpot: " << RSSpot << ", current cycle: " << currentCycle << std::endl;

				if (RSSpot != -1)
				{
					//std::cout << "Wb fpAdd: " << currentCycle << std::endl;

					addfRS->compute(RSSpot);
					CDB = addfRS->getValue(RSSpot)->computation;
					ROB->table[ROBSpot].value = (addfRS->getValue(RSSpot)->computation);
					ROB->table[ROBSpot].cmt_flag = 1;
					addfRS->clearLocation(RSSpot);
				}
			}
			else if (timingDiagram[h*numCol + 0].fpMult == true)
			{
				RSSpot = mulfRS->findRSFromROB(ROBSpot);

				//std::cout << "WB, FpMult, RSSpot: " << RSSpot << ", current cycle: " << currentCycle << std::endl;

				if (RSSpot != -1)
				{
					//std::cout << "Wb fpMult: " << currentCycle << std::endl;

					mulfRS->compute(RSSpot);
					CDB = mulfRS->getValue(RSSpot)->computation;
					ROB->table[ROBSpot].value = (mulfRS->getValue(RSSpot)->computation);
					ROB->table[ROBSpot].cmt_flag = 1;
					mulfRS->clearLocation(RSSpot);
				}
			}
			else if (timingDiagram[h*numCol + 0].isMem == true)
			{
				inst &ins = instruction[h];
				
				if (ins.opcode == load)
				{
					// Load already has value in ROB from MEM stage
					ROB->table[ROBSpot].cmt_flag = 1;
				}
				else if (ins.opcode == store)
				{
					// Store just marks ready to commit
					ROB->table[ROBSpot].cmt_flag = 1;
				}
			}
			
			
			addiRS->replaceROBDependency(ROBSpot, static_cast<int>(ROB->table[ROBSpot].value));
			addfRS->replaceROBDependency(ROBSpot, static_cast<float>(ROB->table[ROBSpot].value));
			mulfRS->replaceROBDependency(ROBSpot, static_cast<float>(ROB->table[ROBSpot].value));
		
			timingDiagram[h*numCol + 3].startCycle = currentCycle; timingDiagram[h*numCol + 3].endCycle = currentCycle;
			timingDiagram[h*numCol + 0].stepThisCycle = true;

			success = NO_ERROR;
			break;
		}
	}

	return success;
}

TOMASULO_RETURN Tomasulo::commit()
{
	// std::cout << "Commit()\n" << std::endl;
	TOMASULO_RETURN success = ERROR;
	
	//std::cout << "Commit cycle: " << currentCycle << std::endl;

	// Commit an instruction when it is the oldest in the ROB (ROB head points to it) and the ready/finished bit is set.
	// Note: we can only commit 1 instruction per cycle.
	// If store instructions --> write into memory
	// If other instruction --> write to ARF
	// Free ROB entry and update RAT (clear aliases). Advance ROB head to the next instruction.
	// Mark instruction as committed (record commit cycle for table).

	int ROBSpot;
	for (size_t h = 0; h < numRow; ++h) 
	{
		//std::cout << "Instr number: " << h << std::endl;
		//// std::cout << "one\n";

		ROBSpot = timingDiagram[h*numCol + 0].numROB;

		//// std::cout << "two\n";
		
		//int ableToCommit = 0;
		//// std::cout << ROBSpot << std::endl;
		//printROB();
		//if (ROB->ableToCommit(ROBSpot) == true)
		//{
		//	ableToCommit = true;
		//}

		//// std::cout << "Able to commit: " << ableToCommit << std::endl;
		std::cout << "Checking for commit row # " << h << std::endl;
		
		
		// First check for branch conditions.
		if (timingDiagram[h*numCol + 0].isBranch == true)
		{
			// Branch does not have a memory or write back stage.
			if (!((timingDiagram[h*numCol + 1].endCycle != 0) && (timingDiagram[h*numCol + 4].startCycle == 0) 
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false) && (ROB->ableToCommit(ROBSpot) == true)))
			{
				continue;
			}
		}
		// Check for all other instructions.
		else if (!((timingDiagram[h*numCol + 3].endCycle != 0) && (timingDiagram[h*numCol + 4].startCycle == 0) 
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false) && (ROB->ableToCommit(ROBSpot) == true)))
		{
			continue;
		}
		
		std::cout << "Commiting instruction #" << h << std::endl;
		std::cout << "Rob spot: " << ROBSpot << std::endl;
		std::cout << "timingDiagram[h*numCol + 3].endCycle = " << timingDiagram[h*numCol + 3].endCycle << std::endl;
		std::cout << "timingDiagram[h*numCol + 4].startCycle = " << timingDiagram[h*numCol + 4].startCycle << std::endl;
		std::cout << "timingDiagram[h*numCol + 0].stepThisCycle = " << timingDiagram[h*numCol + 0].stepThisCycle <<  std::endl;
		std::cout << "ROB->ableToCommit(ROBSpot) = " <<  (int)ROB->ableToCommit(ROBSpot) << std::endl;
		// std::cout << "Row # " << h << "should be able to commit" << std::endl;
		//// std::cout << "three\n";
		
		// Check if this is a store instruction
		inst &ins = instruction[h];
		if (ins.opcode == store)
		{
			// Write to memory
			int address = static_cast<int>(ROB->table[ROBSpot].value); // Address was stored in ROB
			float storeValue = FpARF->getValue(ins.rs.id); // Get data from register
			
			// Officially commit by updating the memory vector
			bool found = false;
			for (auto &memEntry : *memory)
			{
				if (memEntry.first == address)
				{
					memEntry.second = storeValue;
					found = true;
					break;
				}
			}
			
			// If address doesn't exist in memory yet, add the new entry
			if (!found)
			{
				memory->push_back(std::make_pair(address, storeValue));
			}
			
			// *Now* we can clear the store from memRS
			int RSSpot = memRS->findRSFromROB(ROBSpot);
			if (RSSpot != -1)
			{
				memRS->clearLocation(RSSpot);
			}
		}
		
		/*
		struct commitReturn
{
	bool validCommit;
	int regType;
	int registerNum;
	float returnValue;
};
*/
		commitReturn robCommit = ROB->commit(ROBSpot);
		std::cout << "Printing out commit of ROB #" << ROBSpot << std::endl;
		std::cout << "robCommit.validCommit = " <<  (int)robCommit.validCommit << std::endl;
		std::cout << "robCommit.regType = " << robCommit.regType << std::endl;
		std::cout << "robCommit.registerNum = " << robCommit.registerNum << std::endl;
		std::cout << "robCommit.returnValue = " << std::to_string(robCommit.returnValue) << std::endl;
		//ROB->printSpot(ROBSpot);
		
		// Branch instructions do not need a case.

		int regLocation;
		if (robCommit.regType == 0)
		{
			regLocation = IntRAT->getNextARFLocation(ROBSpot);
			std::cout << "regLocation: " << regLocation << std::endl;
			// if regLocation == -1, then that means the RAT spot has been overrided.
			if (regLocation == -1)
			{
				std::cout << "Changing IntARF location #" << ROB->table[ROBSpot].dst_id << " with value " << static_cast<int>(robCommit.returnValue) << std::endl;
				IntARF->changeValue(robCommit.registerNum, static_cast<int>(robCommit.returnValue));
			}
			else
			{
				while(regLocation != -1)
				{
					IntARF->changeValue(regLocation, static_cast<int>(robCommit.returnValue));
					regLocation = IntRAT->getNextARFLocation(ROBSpot);
				}
			}
		}
		else if (robCommit.regType == 1)
		{
			regLocation = FpRAT->getNextARFLocation(ROBSpot);
			if (regLocation == -1)
			{
				FpARF->changeValue(robCommit.registerNum, robCommit.returnValue);
			}
			else
			{
				while(regLocation != -1)
				{
					FpARF->changeValue(regLocation, robCommit.returnValue);
					regLocation = FpRAT->getNextARFLocation(ROBSpot);
				}
			}
		}
		
		timingDiagram[h*numCol + 4].startCycle = currentCycle;
		timingDiagram[h*numCol + 4].endCycle = currentCycle;
		
		success = NO_ERROR;
		break;
	}

	return success;
}

TOMASULO_RETURN Tomasulo::speculationRecovery()
{
	std::cout << "speculationRecovery()\n" << std::endl;
	TOMASULO_RETURN success = ERROR;

	std::cout << "Flush after rob spot.\n";
	// Flush the ROB.
	ROB->flushAfterSpot(misprediction.robSpot);

	// Reinstate the RAT;
	std::cout << "Recovering the RATs.\n";
	int checkpointLocation = findCheckpointLocation(misprediction.timingDiagramRow);
	std::cout << "Checkpoint location: " << checkpointLocation << std::endl;
	IntRAT->recoverRAT(IntRATCheckpoints[findCheckpointLocation(misprediction.timingDiagramRow)]);
	FpRAT->recoverRAT(FloatRATCheckpoints[findCheckpointLocation(misprediction.timingDiagramRow)]);

	std::cout << "Removing everything after prediction.\n";
	// Flush the RAT checkpoints and the timing diagram.
	predictionRemoveEverythingAfter(misprediction.timingDiagramRow);
	
	// Get back to the right PC and timind diagram row;
	timingDiagramPointer = misprediction.timingDiagramRow+1;
	PC = timingDiagram[misprediction.timingDiagramRow*numCol+0].instrNum+1;
	
	success = NO_ERROR;
	return success;
}

void Tomasulo::clearSteps()
{
    for (int i = 0; i < numRow; i++)
    {
		timingDiagram[i*numCol + 0].stepThisCycle = false;
        }
    }
timing_type Tomasulo::getValue(int numInstr, int numCycle)
{
	return timingDiagram[numInstr*numCol + numCycle];
}

void Tomasulo::printARF(bool select) // 0 for integer, 1 for float.
{
	std::cout << "Printing out: ";
	if (!select)
	{
		std::cout << "IntARF\n";
		for (int i = 0; i < IntARF->getSize(); i++)
		{
			std::cout << "R" << i << ": " << IntARF->getValue(i) << std::endl;
		}
	}
	else
	{
		std::cout << "FpARF\n";
		for (int i = 0; i < FpARF->getSize(); i++)
		{
			std::cout << "F" << i << ": " << std::to_string(FpARF->getValue(i)) << std::endl;
		}
	}
}
void Tomasulo::printRAT(bool select) // 0 for iteger, 1 for float.
{
	std::cout << "Printing out: ";
	if (!select)
	{
		std::cout << "IntRAT\n";
		std::cout << "Num locations: " << IntRAT->getSize() << std::endl;
		for (int i = 0; i < IntRAT->getSize(); i++)
		{
			std::cout << i << ": " << IntRAT->getValue(i)->locationType << IntRAT->getValue(i)->locationNumber << std::endl;
		}
	}
	else
	{
		std::cout << "FpRAT\n";
		for (int i = 0; i < FpRAT->getSize(); i++)
		{
			std::cout << i << ": " << FpRAT->getValue(i)->locationType << FpRAT->getValue(i)->locationNumber << std::endl;
		}
	}
}


void Tomasulo::printROB(int start = -1, int finish = -1)
{
	int actualStart = 0;
	int actualFinish = ROB->size;
	if (start != -1)
	{
		actualStart = start;
	}
	if (finish != -1)
	{
		actualFinish = finish;
	}
	std::cout << "Printing out ROB:\n";
	std::cout << "HeadPtr: " << ROB->headPtr << std::endl;
	std::cout << "TailPtr: " << ROB->tailPtr << std::endl;
	for (int i = actualStart; i < actualFinish; i++)
	{
		std::cout << i << ": " << "id: " << ROB->table[i].id << ", dst_id: " 
			<< ROB->table[i].dst_id << ", value: " << std::to_string(ROB->table[i].value)
				<< ", cmt_flag: " << ROB->table[i].cmt_flag << std::endl;
	}
}
	
void Tomasulo::printNonZeroRegVals()
{
	std::cout << "\nNonzero integer registers: \n";
	int value;
	for (int i = 0; i < IntARF->getSize(); i++)
	{
		value = IntARF->getValue(i);
		if (value != 0)
		{
			std::cout << "R" << i << ": " << value << std::endl;
		}
	}

	std::cout << "\nNonzero float registers: \n";
	float valueFP;
	for (int i = 0; i < FpARF->getSize(); i++)
	{
		valueFP = FpARF->getValue(i);
		if (valueFP != 0.0)
		{
			std::cout << "F" << i << ": " << std::to_string(valueFP) << std::endl;
		}
	}
}

void Tomasulo::printNonZeroMemVals()
{
    std::cout << "\nNonzero memory locations:\n";

    // memory is a pointer to std::vector<mem_unit>, where mem_unit is a pair<int, float>
    for (const auto &memEntry : *memory)
    {
        int address = memEntry.first;
        float value = memEntry.second;

        if (value != 0.0)
        {
            std::cout << "Mem[" << address << "]: " << std::to_string(value) << std::endl;
        }
    }

    std::cout << "-----------------------------------------------\n";
}
	
void Tomasulo::printRS(int select) // 0 = addiRS, 1 = addfRS, mulfRS = 2
{
	std::cout << "Printing out ";
	if (select == 0)
	{
		std::cout << "Addi RS:\n";
		for (int i = 0; i < addiRS->getSize(); i++)
		{
			std::cout << i << ": " << "Operation: " << addiRS->getValue(i)->operation << ", Rob Location: " << addiRS->getValue(i)->robLocation
				<< ", Rob Dependency 0: " << addiRS->getValue(i)->robDependency0 << ", Rob Dependency 1: " << addiRS->getValue(i)->robDependency1
					<< ", Value 0: " << addiRS->getValue(i)->value0 << ", Value 1: " << addiRS->getValue(i)->value1 << ", Computation: " << addiRS->getValue(i)->computation << std::endl;
		}
	}
	else if (select == 1)
	{
		std::cout << "Addf RS:\n";
		for (int i = 0; i < addfRS->getSize(); i++)
		{
			std::cout << i << ": " << "Operation: " << addfRS->getValue(i)->operation << ", Rob Location: " << addfRS->getValue(i)->robLocation
				<< ", Rob Dependency 0: " << addfRS->getValue(i)->robDependency0 << ", Rob Dependency 1: " << addfRS->getValue(i)->robDependency1
					<< ", Value 0: " << std::to_string(addfRS->getValue(i)->value0) << ", Value 1: " << std::to_string(addfRS->getValue(i)->value1) 
						<< ", Computation: " << std::to_string(addfRS->getValue(i)->computation)<< std::endl;
		}
	}
	else if (select == 2)
	{
		std::cout << "Mulf RS:\n";
		for (int i = 0; i < mulfRS->getSize(); i++)
		{
			std::cout << i << ": " << "Operation: " << mulfRS->getValue(i)->operation << ", Rob Location: " << mulfRS->getValue(i)->robLocation
				<< ", Rob Dependency 0: " << mulfRS->getValue(i)->robDependency0 << ", Rob Dependency 1: " << mulfRS->getValue(i)->robDependency1
					<< ", Value 0: " << std::to_string(mulfRS->getValue(i)->value0) << ", Value 1: " << std::to_string(mulfRS->getValue(i)->value1) 
						<< ", Computation: " << std::to_string(mulfRS->getValue(i)->computation) << std::endl;
		}
	}
	else
	{
		std::cout << "Invalid selection of RS table\n";
	}
}

void Tomasulo::printBTB()
{
	std::cout << "Printing out BTB.\n";
	BTB_Entry *tempHolder;
	for (int i = 0; i < btb.getLength(); i++)
	{
		tempHolder = btb.getTableEntryByIndex(i);
		if (tempHolder != nullptr)
		{
			std::cout << i << ": Address = " << tempHolder->instrAddress << ", Predicted Address = " << tempHolder->predictedAddress
				<< ", Take Branch = " << (int)tempHolder->isTaken << std::endl;
		}
	}
}
void Tomasulo::printOutTimingTable()
{
	std::cout << "\n-----------------------------------------------\n";
	std::cout << "Timing table:\n";
	std::cout << "\t\tISSUE\tEX\tMEM\tWB\tCOMMIT\n";


	for (int i = 0; i < numRow; i++)
	{
		std::cout << "Instr #" << timingDiagram[i*numCol+0].instrNum << ":\t";
		for (int j = 0; j < numCol; j++)
		{
			std::cout << timingDiagram[i*numCol+j].startCycle << "," << timingDiagram[i*numCol+j].endCycle << "\t";
		}
		std::cout << std::endl;
	}
}

void Tomasulo::printOutput()
{
	printOutTimingTable();
	printNonZeroRegVals();
	printNonZeroMemVals();
}

bool Tomasulo::fullAlgorithm()
{
	/*
	TOMASULO_RETURN returnVal = NO_ERROR;
	bool keepRunning = true;

	while (keepRunning == true)
	{
		clearSteps();
		returnVal = issue();

		if (returnVal != DONE)
		{
			returnVal = execute();
			// If we find a branch misprediction, we need to recover. Otherwise, keep operating.
			if (returnVal == BRANCH_MISPREDICT)
			{
				// Assume the speculation recovery happens in its own cycle;
				currentCycle++;
				returnVal = speculationRecovery();
			}
			else{
				returnVal = mem();
				returnVal = wb();
				returnVal = commit();
			}
			currentCycle++;
		}
		else
		{
			keepRunning = false;
		}
	}*/
	TOMASULO_RETURN returnVal;
	std::cout << std::endl;
	std::cout << "BEFORE:\n";
	std::cout << "CYVLE NUMBER: " << currentCycle << std::endl;
	std::cout << "CDB: " << std::to_string(CDB) << std::endl;
	printRS(0);
	printBTB();
	printOutTimingTable();
	printROB(0, 25);
	printARF(0);
	printRAT(0);
	clearSteps();
	issue();
	returnVal = execute();
	if (returnVal == BRANCH_MISPREDICT)
	{
		speculationRecovery();
	}
	mem();
	wb();
	commit();
	//printROB();
	std::cout << "AFTER:\n";
	std::cout << "CDB: " << std::to_string(CDB) << std::endl;
	printROB(0, 25);
	printARF(0);
	printRAT(0);
	currentCycle++;
	return 1;
}