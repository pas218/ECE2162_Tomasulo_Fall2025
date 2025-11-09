// Tomasulo.cpp
#include "Tomasulo.hpp"
#include <vector>

Tomasulo::Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore, int numCDB,
			ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RS<float, Ops> *memRS,
			RAT<int> *IntRAT, RAT<float> *FpRAT, ReOrderBuf *ROB, std::vector<inst> &instruction, std::vector<mem_unit> *memory)
	: numRow(numberInstructions) , numberInstructions(numberInstructions), numExInt(numExInt), numExFPAdd(numExFPAdd), numExFPMult(numExFPMult),
		numExLoadStore(numExLoadStore), numMemLoadStore(numMemLoadStore), numCDB(numCDB)
{
	this->IntARF      = IntARF;
	this->FpARF       = FpARF;
	this->addiRS      = addiRS;
	this->addfRS      = addfRS;
	this->mulfRS      = mulfRS;
	this->memRS       = memRS;
	this->IntRAT      = IntRAT;
	this->FpRAT       = FpRAT;
	this->ROB         = ROB;
	this->instruction = instruction;
	this->memory      = memory;
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
	
	
	//std::cout << "Operation type: " << operationType << std::endl;

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
		// Branch.
	}
	else
	{
		// NOP.
	}
	
	// SECTION 2: ACTUALLY PERFORM ISSUE, ASSUMING THERE IS VIABILITY
	
	// Write the to ROB, RS, RAT, and timing table.
	// operationType: 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, 5 = NOP
	int freeROBSpot = ROB->freeSpot();
	if (freeRSSpot != -1)
	{
		// ROB.
		if (operationType == 0)
		{
			ROB->table[freeROBSpot].id = 0;
		}
		else if ((operationType == 1) || (operationType == 2) || (operationType == 3))
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
			else
			{
				addiRS->changeRSVal0(freeRSSpot, IntARF->getValue(regID0));
			}
			
			// Change dependencies/ values 1;
			int regID1 = ins.rt.id;
			int robDep1 = ROB->findDependency(intDependency, regID1);
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
		
		timingDiagram[PC*numCol + 0].startCycle         = currentCycle;
		timingDiagram[PC*numCol + 0].endCycle           = currentCycle;
		timingDiagram[PC*numCol + 0].numROB   	        = freeROBSpot;
		timingDiagram[PC*numCol + 0].stepThisCycle   	= true;


		// Change the flag type in the timing diagram depending on operation type.
		// operationType: 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, 5 = NOP
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
		else if (operationType == 2)
		{
			timingDiagram[PC*numCol + 0].fpMult = true;
			
			//std::cout << "Inside fpMult\n";
		}
		else if (operationType == 3)
		{
			timingDiagram[PC*numCol + 0].isMem = true;

			//std::cout << "Inside isMem\n";
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

	// This loop is to START execution.
    for (size_t h = 0; h < numberInstructions; ++h) 
	{  
		// Find next instruction that has been issued, but not executed.
		if (!((timingDiagram[h*numCol + 0].endCycle != 0) && (timingDiagram[h*numCol + 1].startCycle == 0)
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false)))
		{
			continue;
		}

		// See if there are any unaddressed dependencies.
		bool unaddressedDeps;
		int robSpot = timingDiagram[h*numCol + 0].numROB;
		//bool yes = false;

		if (timingDiagram[h*numCol + 0].isInt == true)
		{
			unaddressedDeps = addiRS->hasUnaddressedDependencies(robSpot);
		}
		else if (timingDiagram[h*numCol + 0].fpAdd == true)
		{
			//yes = true;
			unaddressedDeps = addfRS->hasUnaddressedDependencies(robSpot);
		}
		else if (timingDiagram[h*numCol + 0].fpMult == true)
		{
			//yes = true;
			unaddressedDeps = mulfRS->hasUnaddressedDependencies(robSpot);
		}
		else if (timingDiagram[h*numCol + 0].isMem == true)
		{
			//yes = true;
			unaddressedDeps = memRS->hasUnaddressedDependencies(robSpot);
		}

		if (unaddressedDeps == true)
		{
			/*
			if (yes == true)
			{
				std::cout << std::endl << "float failed dependencies" << std::endl;
				printRS(3);
				//printRS(2);
			}
			*/
			continue;
		}
		
		// Write down the start cycle for execute
		timingDiagram[h*numCol + 1].startCycle = currentCycle;
		
		// Only do this once.
		break;
		
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
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExInt)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
			}
			
		}
		else if (timingDiagram[h*numCol + 0].fpAdd == true)
		{
			// Fp type add.
			
			// See if finished executing.
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExFPAdd)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
			}
			
		}
		else if (timingDiagram[h*numCol + 0].fpMult == true)
		{
			// Fp type mult.
			
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExFPMult)
			{
				timingDiagram[h*numCol + 1].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
			}
		}
		else if (timingDiagram[h*numCol + 0].isMem == true)
		{
			// Memory (Load/Store)
			
			if ((currentCycle - timingDiagram[h*numCol + 1].startCycle + 1) >= numExLoadStore)
			{
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

		/*
		if (timingDiagram[h*numCol + 1].startCycle == currentCycle)
		{
    		std::cout << "Starting execution of instruction #" << h << " at cycle " << currentCycle << std::endl;
		}
		*/
		success = true;
	}

	return success;
}

bool Tomasulo::mem()
{
    bool success = false;
    
	// Record mem cycle for table
	// Check for "forwarding-from-a-store" as mentioned in project document. It takes 1 cycle to perform the forwarding if a match is found. If not found then the load accesses the memory for data.
	// Once a load returns from the memory or gets the value from a previous store, its entry in the load/store queue is cleared.
	// Note that it is correct not to clear this entry, but the queue can quickly fill up, causing structure hazards for future loads/stores.    

    // Stores don't actually write to memory here - they write in commit stage
    // Loads can access memory or forward from a store
    
 	// Find instructions that have completed EX stage but not completed MEM stage
    for (size_t h = 0; h < numberInstructions; ++h)
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
                                
                                success = true;
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
                    
                    success = true;
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
            
            success = true;
        }
        
        // Only process one instruction per cycle
        break;
    }
    
    return success;
}

bool Tomasulo::wb()
{
	bool success = 0;

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
		for (size_t h = 0; h < numberInstructions; ++h) 
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

			success = true;
			break;
		}
	}

	return success;
}

bool Tomasulo::commit()
{
	bool success = 0;
	
	//std::cout << "Commit cycle: " << currentCycle << std::endl;

	// Commit an instruction when it is the oldest in the ROB (ROB head points to it) and the ready/finished bit is set.
	// Note: we can only commit 1 instruction per cycle.
	// If store instructions --> write into memory
	// If other instruction --> write to ARF
	// Free ROB entry and update RAT (clear aliases). Advance ROB head to the next instruction.
	// Mark instruction as committed (record commit cycle for table).

	int ROBSpot;
	for (size_t h = 0; h < numberInstructions; ++h) 
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

		if (!((timingDiagram[h*numCol + 3].endCycle != 0) && (timingDiagram[h*numCol + 4].startCycle == 0) 
				&& (timingDiagram[h*numCol + 0].stepThisCycle == false) && (ROB->ableToCommit(ROBSpot) == true)))
		{
			continue;
		}
		
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
		
		commitReturn robCommit = ROB->commit(ROBSpot);

		int regLocation;
		if (robCommit.regType == 0)
		{
			regLocation = IntRAT->getNextARFLocation(ROBSpot);
			while(regLocation != -1)
			{
				IntARF->changeValue(regLocation, static_cast<int>(robCommit.returnValue));
				regLocation = IntRAT->getNextARFLocation(ROBSpot);
			}
		}
		else if (robCommit.regType == 1)
		{
			regLocation = FpRAT->getNextARFLocation(ROBSpot);
			while(regLocation != -1)
			{
				FpARF->changeValue(regLocation, robCommit.returnValue);
				regLocation = FpRAT->getNextARFLocation(ROBSpot);
			}
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
	
void Tomasulo::printOutTimingTable()
{
	std::cout << "\n-----------------------------------------------\n";
	std::cout << "Timing table:\n";
	std::cout << "\t\tISSUE\tEX\tMEM\tWB\tCOMMIT\n";


	for (int i = 0; i < numRow; i++)
	{
		std::cout << "Instr #" << (i+1) << ":\t";
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
	clearSteps();

	issue();
	execute();
	mem();
	wb();
	commit();

	currentCycle++;
	
	return 1;
}