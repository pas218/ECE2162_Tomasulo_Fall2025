#include "Tomasulo.hpp"


Tomasulo::Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore,
			ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RAT<int> *IntRAT, 
				RAT<float> *FpRAT, ReOrderBuf *ROB, std::vector<inst> &instruction)
	: numRow(numberInstructions) , numberInstructions(numberInstructions), numExInt(numExInt), numExFPAdd(numExFPAdd), numExFPMult(numExFPMult),
		numExLoadStore(numExLoadStore), numMemLoadStore(numMemLoadStore)
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
	//std::cout << "IntARF pointer inside issue: " << IntARF << std::endl;
	//std::cout << "In issue.\n";
	//std::cout << IntARF->getValue(2) << std::endl;
	//std::cout << std::to_string(FpARF->getValue(2)) << std::endl; 
	//std::cout << "Now the RAT.\n";
	//std::cout << IntRAT->getValue(2)->locationType << IntRAT->getValue(2)->locationNumber << std::endl;
	
	bool success = false;


    // Loop over all instructions to find the first one that hasn't been issued
    for (size_t h = 0; h < numberInstructions; ++h)
    {
        inst &ins = instruction[h];

		// If the t_issue field is greater than 0, the instruction has already been issued
        if (ins.t_issue > 0) continue;

        int freeRS = -1;
        int freeROB = -1;
		int operationType = -1;
		
		// Check if there is a free RS for instruction (stall if not available)
        if (ins.opcode == addi || ins.opcode == subi)
        {
            for (int i = 0; i < addiRS->getSize(); i++)
			{
                if (addiRS->getValue(i)->robLocation == -1)
				{
					freeRS = i;
					operationType = 0; // Interger operation.
					break;
				}
			}
        }
        else if (ins.opcode == addf || ins.opcode == subf)
        {
            for (int i = 0; i < addfRS->getSize(); i++)
			{
                if (addfRS->getValue(i)->robLocation == -1)
				{
					freeRS = i; 
					operationType = 1; // FP operation.
					break;
				}
			}
        }
        else if (ins.opcode == mulf)
        {
            for (int i = 0; i < mulfRS->getSize(); i++)
			{
                if (mulfRS->getValue(i)->robLocation == -1)
				{
					freeRS = i;
					operationType = 1; // FP operation.
					break;
				}
			}
        }

		// If there are no free RS, we must stall (success is false)
        if (freeRS == -1)
		{
			return success;
		}


		
		if (ROB->full() == 0)  // If full then 1. If not then 0.
		{
			ROB->table[robPointer].id = operationType; // set ID to indicate it's no longer free
			freeROB = robPointer;
			ROB->table[robPointer].dst_id = ins.rd.id;
			ROB->table[robPointer].cmt_flag = 0;
			// Change the RAT.
			if (operationType == 0)
			{
				IntRAT->changeValue(ins.rd.id, 0, freeROB);
			}
			else
			{
				FpRAT->changeValue(ins.rd.id, 0, freeROB);
			}
			if (ROB->full() == 0)
			{
				if (robPointer + 1 < ROB->size)
				{
						robPointer++;
				}
				else{
					robPointer = 0;
				}
			}
		}

		// If there are no free ROB entries, we must stall (success is false)
        if (freeROB == -1)
		{
			return success;
		}

        // Fill RS using the fields for each that are defined in RS.hpp
        if (ins.opcode == addi)
		{
			addiRS->changeOperation(freeRS, ADD);
		}
        else if (ins.opcode == subi)
		{
			addiRS->changeOperation(freeRS, SUB);
		}
        else if (ins.opcode == addf)
		{
			addfRS->changeOperation(freeRS, ADD);
		}
        else if (ins.opcode == subf)
		{
			addfRS->changeOperation(freeRS, SUB);
		}
        else if (ins.opcode == mulf)
		{
			mulfRS->changeOperation(freeRS, MULT);
		}

        if (ins.opcode == addi || ins.opcode == subi)
        {
            addiRS->changeROBLocation(freeRS, freeROB);
            addiRS->changeRSVal0(freeRS, IntARF->getValue(ins.rs.id));
            addiRS->changeRSVal1(freeRS, IntARF->getValue(ins.rt.id));
            addiRS->changeROBDependency(freeRS, -1, -1); // Assume no dependency for now?
        }
        else if (ins.opcode == addf || ins.opcode == subf)
        {
			addfRS->changeROBLocation(freeRS, freeROB);
            addfRS->changeRSVal0(freeRS, IntARF->getValue(ins.rs.id));
            addfRS->changeRSVal1(freeRS, IntARF->getValue(ins.rt.id));
            addfRS->changeROBDependency(freeRS, -1, -1); // Assume no dependency for now?
        }
        else if (ins.opcode == mulf)
        {
            mulfRS->changeROBLocation(freeRS, freeROB);
            mulfRS->changeRSVal0(freeRS, IntARF->getValue(ins.rs.id));
            mulfRS->changeRSVal1(freeRS, IntARF->getValue(ins.rt.id));
            mulfRS->changeROBDependency(freeRS, -1, -1); // Assume no dependency for now?
        }

		// Since issue was successful, record issue cycle for timing table using gloabl counter
		timingDiagram[h*numCol + 0].startCycle = currentCycle;
		timingDiagram[h*numCol + 0].endCycle = currentCycle;
		currentCycle++;
        ins.t_issue = currentCycle;
        success = true;

		// We can only issue one instr per cycle, so go ahead and break from for loop
        break;
    }

    return success;


	
	// Read operands in registers (if not available yet, record which RS will eventually produce the result)
	// Register renaming, update RAT with source/target for new ROB entry




	// Branch prediction will eventually happen here.
	std::cout << "end\n";
	return success;
}


// Classes we will need for execute stage
bool Tomasulo::execute()
{
	bool success = 0;
	
	// Execute instructions once all operands are ready (record execute cycle for table)
	// "Compete" to use functional unit (oldest first)
	// Advance instruction count for each instruction in execution. Wait for the number of cycles read from the input before marking the instruction as ready (cycle_addi, cycle_addf, etc.)
	// When an FU's execution completes, mark the RS entry as 'finished' so that it can be written on the next writeback cycle
	// For load/store, also calculate effective address (does not occupy integer ALU to do this).
	// Eventually, branch resolution will happen here






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
	

	// Broadcast results onto the CDB (other instructions waiting on it will need to pick it up), or buffer if the CDB is full
	// Write result back to RS and ROB entry (record WB cycle for table)
	// Mark the ready/finished bit in ROB since the instruction has completed execution
	// Free reservation stations for future reuse when finished
	// Store instructions write to memory in this stage
        




	return success;
}

bool Tomasulo::commit()
{
	bool success = 0;
	

	// Commit an instruction when it is the oldest in the ROB (ROB head points to it) and the ready/finished bit is set.
	// Note: we can only commit 1 instruction per cycle.
	// If store instructions --> write into memory
	// If other instruction --> write to ARF
	// Free ROB entry and update RAT (clear aliases). Advance ROB head to the next instruction.
	// Mark instruction as committed (record commit cycle for table).




	return success;
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
					<< ", Value 0: " << addiRS->getValue(i)->value0 << ", Value 1: " << addiRS->getValue(i)->value1 << std::endl;
		}
	}
	else if(select == 1)
	{
		std::cout << "Addf RS:\n";
		for (int i = 0; i < addfRS->getSize(); i++)
		{
			std::cout << i << ": " << "Operation: " << addfRS->getValue(i)->operation << ", Rob Location: " << addfRS->getValue(i)->robLocation
				<< ", Rob Dependency 0: " << addfRS->getValue(i)->robDependency0 << ", Rob Dependency 1: " << addfRS->getValue(i)->robDependency1
					<< ", Value 0: " << std::to_string(addfRS->getValue(i)->value0) << ", Value 1: " << std::to_string(addfRS->getValue(i)->value1) << std::endl;
		}
	}
	else if(select == 2)
	{
		std::cout << "Mulf RS:\n";
		for (int i = 0; i < mulfRS->getSize(); i++)
		{
			std::cout << i << ": " << "Operation: " << mulfRS->getValue(i)->operation << ", Rob Location: " << mulfRS->getValue(i)->robLocation
				<< ", Rob Dependency 0: " << mulfRS->getValue(i)->robDependency0 << ", Rob Dependency 1: " << mulfRS->getValue(i)->robDependency1
					<< ", Value 0: " << std::to_string(mulfRS->getValue(i)->value0) << ", Value 1: " << std::to_string(mulfRS->getValue(i)->value1) << std::endl;
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
	bool keepGoing  = 0;
	while (!done)
	{
		keepGoing = issue();
		if (keepGoing)
		{
				keepGoing = execute();
		}
		if (keepGoing)
		{
				keepGoing = execute();
		}
		if (keepGoing)
		{
				keepGoing = mem();
		}
		if (keepGoing)
		{
				keepGoing = wb();
		}
		if (keepGoing)
		{
				keepGoing = commit();
		}
	}
	return done;
}