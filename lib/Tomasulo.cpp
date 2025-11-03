#include "Tomasulo.hpp"
#include <vector>

Tomasulo::Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore, int numCDB,
			ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RAT<int> *IntRAT, 
				RAT<float> *FpRAT, ReOrderBuf *ROB, std::vector<inst> &instruction)
	: numRow(numberInstructions) , numberInstructions(numberInstructions), numExInt(numExInt), numExFPAdd(numExFPAdd), numExFPMult(numExFPMult),
		numExLoadStore(numExLoadStore), numMemLoadStore(numMemLoadStore), numCDB(numCDB)
{
	this->IntARF      = IntARF;
	this->FpARF       = FpARF;
	this->addiRS      = addiRS;
	this->addfRS      = addfRS;
	this->mulfRS      = mulfRS;
	this->IntRAT      = IntRAT;
	this->FpRAT       = FpRAT;
	this->ROB         = ROB;
	this->instruction = instruction;
	robPointer        = 0;
	done              = 0;
	currentCycle      = 1;
	PC = 0;
	
	for(int i = 0; i < numRow*numCol; i++)
	{
		timing_type temp;
		temp.startCycle = 0;
		temp.endCycle = 0;
		timingDiagram.push_back(temp);
	}
}


// TODO: There are errors here because, for example, instruction cannot be accessed here. I'm ignoring for now since I can't build anyway
//       I may also just be accessing things in the wrong way.\
// Classes we will need for the issue cycle:
// ARF
// RAT
// ROB
// RS
// Instructions
bool Tomasulo::issue()
{
	//std::cout << "IS cycle: " << currentCycle << std::endl;
	bool success = 0;
	
	
	if (PC >= numberInstructions)
	{
		return success;
	}
	
	// Use program counter to find new instruction.
	inst &ins = instruction[PC];
	
	///// SECTION 1: JUST CHECK FOR ISSUE VIABILITY
	
	//std::cout << "PC: " << PC << std::endl;
	//std::cout << "Opcode: " << ins.opcode << std::endl;
	// 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, -1 = NOP
	int operationType;
	Ops physicalOperation;
	if (ins.opcode == add)
	{
		operationType = 0;
		physicalOperation = ADD;
	}
	else if(ins.opcode == addi)
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
	
	
	//std::cout << "Operation type: " << operationType << std::endl;
	int freeRSSpot;
	// 0 = int, 1 = Fp, 2 = Memory, 3 = Branch 
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
		// Memory.
	}
	else if (operationType == 4)
	{
		// Branch.
	}
	else
	{
		// NOP.
	}
	
	// SECTION 2: ACTUALLY PERFORM ISSUE, ASSUMING THERE IS VIABILITY
	
	// Write the to ROB, RS, RAT, and timing table.
	int freeROBSpot = ROB->freeSpot();
	if (freeRSSpot != -1)
	{
		// ROB.
		if (operationType == 0)
		{
			ROB->table[freeROBSpot].id = 0;
		}
		else if ((operationType == 1) || (operationType == 2))
		{
			ROB->table[freeROBSpot].id = 1;
		}
	
		ROB->table[freeROBSpot].dst_id = ins.rd.id;
		
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
			if (robDep0 != -1)
			{
				addiRS->changeROBDependency0(freeRSSpot, robDep0);
			}
			else{
				addiRS->changeRSVal0(freeRSSpot, IntARF->getValue(regID0));
			}
			
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(intDependency, regID1);
			if (robDep1 != -1)
			{
				addiRS->changeROBDependency1(freeRSSpot, robDep1);
			}
			else{
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
			else{
				addfRS->changeRSVal0(freeRSSpot, FpARF->getValue(regID0));
			}
			
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(floatDependency, regID1);
			if (robDep1 != -1)
			{
				addfRS->changeROBDependency1(freeRSSpot, robDep1);
			}
			else{
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
			else{
				mulfRS->changeRSVal0(freeRSSpot, FpARF->getValue(regID0));
			}
			
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(floatDependency, regID1);
			if (robDep1 != -1)
			{
				mulfRS->changeROBDependency1(freeRSSpot, robDep1);
			}
			else{
				mulfRS->changeRSVal1(freeRSSpot, FpARF->getValue(regID1));
			}
			
			FpRAT->changeValue(ins.rd.id, 0, freeROBSpot);
			
		}
		
		timingDiagram[PC*numCol + 0].startCycle         = currentCycle;
		timingDiagram[PC*numCol + 0].endCycle           = currentCycle;
		timingDiagram[PC*numCol + 0].numROB   	        = freeROBSpot;
		timingDiagram[PC*numCol + 0].stepThisCycle   	= true;
		// Change the flag type in the timing diagram depending on operation type.
		
		
		if (operationType == 0)
		{
			timingDiagram[PC*numCol + 0].isInt = true;
			
			//std::cout << "Inside isInt\n";
		}
		else if (operationType == 1)
		{
			timingDiagram[PC*numCol + 0].fpAdd = true;
			
			//std::cout << "Inside fpAdd\n";
		}
		else if(operationType == 2)
		{
			timingDiagram[PC*numCol + 0].fpMult = true;
			
			//// std::cout << "Inside fpMult\n";
		}
		
		success = true;
	}
	
	PC++;
	return success;
}


// Classes we will need for execute stage
bool Tomasulo::execute()
{
	// std::cout << "EX cycle: " << currentCycle << std::endl;
	bool success = 0;
	// Find next instruction that has been issued but not executed.
	// This loop is to START execution.
    for (size_t h = 0; h < PC; ++h) 
	{  
		// Find instruction that has been issued, but not executed.
		if (!((timingDiagram[h*numCol + 0].endCycle != 0) && (timingDiagram[h*numCol + 1].startCycle == 0)
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false)))
		{
			continue;
		}
		
		// Write down the start cycle for executel
		timingDiagram[h*numCol + 1].startCycle = currentCycle;
		
		break; // Only do this once.
		
		success = true;
	}
	
	// Find next instruction that has started executing, but not finished.
	// This loop is to END execution.
    for (size_t h = 0; h < numberInstructions; ++h) 
	{  
		// Find instruction that has been issued, but not executed.
		if (!((timingDiagram[h*numCol + 1].startCycle != 0) && (timingDiagram[h*numCol + 1].endCycle == 0)))
		{
			continue;
		}
		
		
		// Get the ROB spot.
		int robSpot = timingDiagram[h*numCol + 0].numROB;
		// Check the instruction type
		inst &ins = instruction[h];
		if (timingDiagram[h*numCol + 0].isInt == true)
		{
			// Integer type.
			// See if finished executing.
			if (currentCycle - timingDiagram[h*numCol + 1].startCycle + 1 >= numExInt)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
			}
			
		}
		else if (timingDiagram[h*numCol + 0].fpAdd == true)
		{
			// Fp type add.
			
			// See if finished executing.
			if (currentCycle - timingDiagram[h*numCol + 1].startCycle + 1 >= numExFPAdd)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
			}
			
		}
		else if (timingDiagram[h*numCol + 0].fpMult == true)
		{
			// Fp type mult.
			
			if (currentCycle - timingDiagram[h*numCol + 1].startCycle + 1 >= numExFPMult)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
			}
		}
		
		success = true;
	}

	return success;
}


bool Tomasulo::mem()
{
	bool success = 0;
	
	
	// Record mem cycle for table
	// Check for "forwarding-from-a-store" as mentioned in project document. It takes 1 cycle to perform the forwarding if a match is found. If not found then the load accesses the memory for data.
	// Once a load returns from the memory or gets the value from a previous store, its entry in the load/store queue is cleared.
	// Note that it is correct not to clear this entry, but the queue can quickly fill up, causing structure hazards for future loads/stores.
        


	return success;
}


bool Tomasulo::wb()
{
	bool success = 0;
	//std::cout << "Wb cycle: " << currentCycle << std::endl;
	// std::cout << "WB cycle: " << currentCycle << std::endl;
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
		// std::cout << "Inside numCBD if statement.\n";
		// Find instruction that has completed execution cycle and (if load/store) memory as well.
		for (size_t h = 0; h < PC; ++h) 
		{
			// std::cout << "wb loop " << h << std::endl;
			// Find instruction that has been issued, but not executed.
			if (!((timingDiagram[h*numCol + 1].endCycle != 0) && (timingDiagram[h*numCol + 3].startCycle == 0) 
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false)))
			{
				continue;
			}
			/*
			else if (!((timingDiagram[h*numCol + 2].endCycle != 0) && (timingDiagram[h*numCol + 3].startCycle == 1) 
				&& (timingDiagram[h*numCol + 0].isMem)))
			{
				// Memory case;
				continue;
			}
			*/
			
			// std::cout << "In wb.\n";
			int ROBSpot = timingDiagram[h*numCol + 0].numROB;
			
			int RSSpot;
			if (timingDiagram[h*numCol + 0].isInt == true)
			{
				RSSpot = addiRS->findRSFromROB(ROBSpot);
				if (RSSpot != -1)
				{
					//std::cout << "Wb inInt: " << currentCycle << std::endl;
					addiRS->compute(RSSpot);
					ROB->table[ROBSpot].value = static_cast<float>(addiRS->getValue(RSSpot)->computation);
					ROB->table[ROBSpot].cmt_flag = 1;
					addiRS->clearLocation(RSSpot);

				}
			}
			else if (timingDiagram[h*numCol + 0].fpAdd == true)
			{
				RSSpot = addfRS->findRSFromROB(ROBSpot);
				if (RSSpot != -1)
				{
					//std::cout << "Wb fpAdd: " << currentCycle << std::endl;
					addfRS->compute(RSSpot);
					ROB->table[ROBSpot].value = static_cast<float>(addfRS->getValue(RSSpot)->computation);
					ROB->table[ROBSpot].cmt_flag = 1;
					addfRS->clearLocation(RSSpot);
				}
			}
			else if (timingDiagram[h*numCol + 0].fpMult == true)
			{
				RSSpot = mulfRS->findRSFromROB(ROBSpot);
				if (RSSpot != -1)
				{
					//std::cout << "Wb fpMult: " << currentCycle << std::endl;
					mulfRS->compute(RSSpot);
					ROB->table[ROBSpot].value = static_cast<float>(mulfRS->getValue(RSSpot)->computation);
					ROB->table[ROBSpot].cmt_flag = 1;
					mulfRS->clearLocation(RSSpot);
				}
			}
			
			
			addiRS->replaceROBDependency(ROBSpot, static_cast<int>(ROB->table[ROBSpot].value));
			addfRS->replaceROBDependency(ROBSpot, static_cast<float>(ROB->table[ROBSpot].value));
			mulfRS->replaceROBDependency(ROBSpot, static_cast<float>(ROB->table[ROBSpot].value));
		
			timingDiagram[h*numCol + 3].startCycle = currentCycle; timingDiagram[h*numCol + 3].endCycle = currentCycle;
			timingDiagram[h*numCol + 0].stepThisCycle = true;
			success = true;
			break;
		}
	}




	return success;
}

bool Tomasulo::commit()
{
	bool success = 0;
	
	// std::cout << "Commit cycle: " << currentCycle << std::endl;
	// Commit an instruction when it is the oldest in the ROB (ROB head points to it) and the ready/finished bit is set.
	// Note: we can only commit 1 instruction per cycle.
	// If store instructions --> write into memory
	// If other instruction --> write to ARF
	// Free ROB entry and update RAT (clear aliases). Advance ROB head to the next instruction.
	// Mark instruction as committed (record commit cycle for table).
	int ROBSpot;
	for (size_t h = 0; h < PC; ++h) 
	{
		//// std::cout << "Instr number: " << h << std::endl;
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
		if (!((timingDiagram[h*numCol + 3].endCycle != 0) && (timingDiagram[h*numCol + 4].startCycle == 0) 
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false) && (ROB->ableToCommit(ROBSpot) == true)))
		{
			continue;
		}
		
		//// std::cout << "three\n";

		commitReturn robCommit = ROB->commit(ROBSpot);

		if (robCommit.regType == 0)
		{
			std::vector<int> regLocations;
			regLocations = IntRAT->getARFLocations(ROBSpot);
			IntARF->replaceValues(regLocations, static_cast<int>(robCommit.returnValue));
		}
		else if (robCommit.regType == 1)
		{
			std::vector<int> regLocations;
			regLocations = FpRAT->getARFLocations(ROBSpot);
			IntARF->replaceValues(regLocations, static_cast<float>(robCommit.returnValue));
		}

		timingDiagram[h*numCol + 4].startCycle = currentCycle;
		timingDiagram[h*numCol + 4].endCycle = currentCycle;
		success = true;
		break;
	}




	return success;
}

void Tomasulo::clearSteps()
{
	for (int i = 0; i < numberInstructions; i++)
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

/*
class ReOrderBuf_entry
{
public:
	int id;				//reorder buffer id is 0 or 1. 0 indicates integer, 1 indicates float. -1 indicates it is free.
	int dst_id;			// destination register id.
	float value;
	int cmt_flag;		// 0 for not commit, 1 for commit;	
 
	ReOrderBuf_entry():id(0),dst_id(0),value(0.0),cmt_flag(0){}
};

class ReOrderBuf
{
public:
	ReOrderBuf_entry *table;
	int head;
	int tail;
	int n;
	int size;

	int get_size();
	bool empty();
	bool full();
	
	ReOrderBuf(int);
	~ReOrderBuf(){delete[]table;}
};
*/
void Tomasulo::printROB()
{
	std::cout << "Printing out ROB:\n";
	for (int i = 0; i < ROB->size; i++)
	{
		std::cout << i << ": " << "id: " << ROB->table[i].id << ", dst_id: " 
			<< ROB->table[i].dst_id << ", value: " << std::to_string(ROB->table[i].value)
				<< ", cmt_flag: " << ROB->table[i].cmt_flag << std::endl;
	}
}
	

void Tomasulo::printRS(int select) // 0 = addiRS, 1 = addfRS, mulfRS = 2
{
	std::cout << "Printing out ";
	if(select == 0)
	{
		std::cout << "Addi RS:\n";
		for (int i = 0; i < addiRS->getSize(); i++)
		{
			std::cout << i << ": " << "Operation: " << addiRS->getValue(i)->operation << ", Rob Location: " << addiRS->getValue(i)->robLocation
				<< ", Rob Dependency 0: " << addiRS->getValue(i)->robDependency0 << ", Rob Dependency 1: " << addiRS->getValue(i)->robDependency1
					<< ", Value 0: " << addiRS->getValue(i)->value0 << ", Value 1: " << addiRS->getValue(i)->value1 << ", Computation: " << addiRS->getValue(i)->computation << std::endl;
		}
	}
	else if(select == 1)
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
	else if(select == 2)
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
	
void Tomasulo::printOutTimingTable()
{
	std::cout << "Printing out timing table:\n";
	std::cout << "  IS, EX, MEM, WB, CM\n";
	for (int i = 0; i < numRow; i++)
	{
		for (int j = 0; j < numCol; j++)
		{
		std::cout << timingDiagram[i*numCol+j].startCycle << "," << timingDiagram[i*numCol+j].endCycle << " ";
		}
		std::cout << std::endl;
	}
}
		
bool Tomasulo::fullAlgorithm()
{
	clearSteps();
	issue();
	execute();
	wb();
	commit();
	currentCycle++;
	return 1;
}