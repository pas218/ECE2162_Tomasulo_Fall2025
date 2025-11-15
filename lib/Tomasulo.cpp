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
	storeCommitInProgress = -1;
	storeCommitStartCycle = 0;
	storeCommitInstrIndex = -1;
	memoryBusFreeAtCycle = 1;  // Memory bus is initially free

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
	
	// Write to the ROB, RS, RAT, and timing table.
	// operationType: 0 = int add/subtract, 1 = Fp add/subtract, 2 = fp mult, 3 = Memory, 4 = Branch, 5 = NOP
	int freeROBSpot = ROB->freeSpot();
	if (freeRSSpot != -1 && freeROBSpot != -1) // Structure hazards
	{
		// ROB.
		if (operationType == 0)
		{
			ROB->table[freeROBSpot].id = 0;
			ROB->table[freeROBSpot].dst_id = ins.rd.id;
		}
		else if (operationType == 1 || operationType == 2)
		{
			ROB->table[freeROBSpot].id = 1;
			ROB->table[freeROBSpot].dst_id = ins.rd.id;
		}
		else if (operationType == 3)
		{
			// Memory operations
			if (ins.opcode == load)
			{
				ROB->table[freeROBSpot].id = 1;
				ROB->table[freeROBSpot].dst_id = ins.rd.id;
			}
			else if (ins.opcode == store)
			{
				// Use -2 to indicate "in use (so don't reuse) but there's no destination register".
				// This is because stores don't have a destination register and must be handled in a special way.
				ROB->table[freeROBSpot].id = -2;     
				ROB->table[freeROBSpot].dst_id = -1;
			}
		}
		
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
				
				// Store a marker in ROB indicating there's a data dependency to check
				if (dataDep != -1)
				{
					// Mark that store has unresolved data dependency
					ROB->table[freeROBSpot].cmt_flag = -1; // Use -1 to mean "waiting for data"
				}
				
				// NOTE: Stores do NOT update RAT since they don't write to registers
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

		// Only increment PC if the issue was successful
		PC++;
	}

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
			unaddressedDeps = memRS->hasUnaddressedDependencies(robSpot);
			
			// For stores, also check if the data value is ready
			inst &ins = instruction[h];
			if (ins.opcode == store && !unaddressedDeps)
			{
				// Check if the data register (rs for stores) is ready
				int floatDependency = 1;
				int dataRegID = ins.rs.id;
				int dataDep = ROB->findDependency(floatDependency, dataRegID);
				
				// Block if there's still a dependency that hasn't been resolved
				if (dataDep != -1 && dataDep != robSpot)
				{
					// Check if that ROB entry has completed writeback
					if (ROB->table[dataDep].cmt_flag != 1)
					{
						unaddressedDeps = true;
					}
				}
			}
		}

		if (unaddressedDeps == true)
		{
			/*
			if (yes == true)
			{
				//std::cout << std::endl << "float failed dependencies" << std::endl;
				//printRS(3);
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
			bool forwardedFromStore = false;
			bool foundMatchingStore = false;
			
			int mostRecentMatchingStore = -1;
			float mostRecentForwardedValue = 0.0f;
			
			// Search all older stores to find the most recent match. We want to forward the most recent store.
			// This accounts for the edge case where multiple stores write to the same location back-to-back.
			for (size_t i = 0; i < h; ++i)
			{
				inst &prevInst = instruction[i];
				
				if (prevInst.opcode == store)
				{
					// Get the previous ROB and RS spots
					int prevROBSpot = timingDiagram[i*numCol + 0].numROB;
					int prevRSSpot = memRS->findRSFromROB(prevROBSpot);
					
					if (prevRSSpot != -1 && 
						timingDiagram[i*numCol + 1].endCycle != 0 &&
						memRS->getValue(prevRSSpot)->computationDone)
					{
						float prevAddress = memRS->getValue(prevRSSpot)->computation;
						
						if (static_cast<int>(prevAddress) == address)
						{
							foundMatchingStore = true;
							
							// Check if data is ready
							int storeDataReg = prevInst.rs.id;
							int floatDependency = 1;
							int storeDataDep = ROB->findDependency(floatDependency, storeDataReg);
							
							bool dataReady = false;
							float forwardedValue = 0.0f;
							
							if (storeDataDep != -1)
							{
								if (ROB->table[storeDataDep].cmt_flag == 1)
								{
									forwardedValue = ROB->table[storeDataDep].value;
									dataReady = true;
								}
							}
							else
							{
								forwardedValue = FpARF->getValue(storeDataReg);
								dataReady = true;
							}
							
							if (dataReady)
							{
								// Update based on the most recent forwarded match
								mostRecentMatchingStore = i;
								mostRecentForwardedValue = forwardedValue;
							}
						}
					}
				}
			}
			
			// After checking all stores, forward from the most recent one
			if (mostRecentMatchingStore != -1)
			{
				//std::cout << "Forwarding from store #" << (mostRecentMatchingStore + 1) << std::endl;
				//std::cout << "  forwardedValue: " << mostRecentForwardedValue << std::endl;
				
				ROB->table[ROBSpot].value = mostRecentForwardedValue;
				forwardedFromStore = true;
				
				timingDiagram[h*numCol + 2].startCycle = currentCycle;
				timingDiagram[h*numCol + 2].endCycle = currentCycle;
				timingDiagram[h*numCol + 0].stepThisCycle = true;
				
				// If we had reserved the memory bus earlier but are now forwarding, release it
				// This handles the case where load started memory access before store's address was ready
				// Memory bus becomes free next cycle (forwarding takes 1 cycle)
				memoryBusFreeAtCycle = currentCycle + 1;

				memRS->clearLocation(RSSpot);
				success = true;
			}
			
			// If no forwarding happened and no matching store was found, access memory
			if (!forwardedFromStore && !foundMatchingStore)
			{
				// No matching store found - access memory normally
				
				// Check if we need to start memory access
				if (timingDiagram[h*numCol + 2].startCycle == 0)
				{
					// Check if single-ported memory bus is available before starting
					if (currentCycle >= memoryBusFreeAtCycle)
					{
						// Start memory access
						timingDiagram[h*numCol + 2].startCycle = currentCycle;

						// Reserve memory bus
						memoryBusFreeAtCycle = currentCycle + numMemLoadStore;

						//std::cout << ">>> Cycle " << currentCycle << ": Load #" << (h+1) << " reserving memory bus until cycle " << memoryBusFreeAtCycle << std::endl;
					}
					else
					{
						// Do nothing -- wait for memory bus to become available
					}
				}
				else
				{
					// Memory access already in progress - check if complete
					// (This runs regardless of memoryBusFreeAtCycle)
					if ((currentCycle - timingDiagram[h*numCol + 2].startCycle + 1) >= numMemLoadStore)
					{
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
						
						// Clear the load from memRS
						memRS->clearLocation(RSSpot);
						
						success = true;
					}
				}
			}
		}
		else if (ins.opcode == store)
		{
			// Stores skip the MEM stage entirely
			
			// Only process if we haven't set the ROB value yet
			if (timingDiagram[h*numCol + 2].startCycle == 0)
			{
				// Store the address in ROB for later use in commit
				ROB->table[ROBSpot].value = effectiveAddress;
				
				// Mark MEM as "done" so this store doesn't keep entering mem() every cycle. Use 0,0 to indicate skipped.
				// This special handling is necessary because it will otherwise hold up the rest of the instructions
				// since we use the timing table for sequencing most things.
				timingDiagram[h*numCol + 2].startCycle = -1;  // Use -1 to indicate "processed but skipped"
				timingDiagram[h*numCol + 2].endCycle = -1;
				
				success = true;
			}
			else
			{
				// Do nothing -- if already processed, just skip this store
			}
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
					// Stores skip WB stage, they go straight to commit
					continue;
				}
			}
			
			
			addiRS->replaceROBDependency(ROBSpot, static_cast<int>(ROB->table[ROBSpot].value));
			addfRS->replaceROBDependency(ROBSpot, static_cast<float>(ROB->table[ROBSpot].value));
			mulfRS->replaceROBDependency(ROBSpot, static_cast<float>(ROB->table[ROBSpot].value));
			memRS->replaceROBDependency(ROBSpot, static_cast<float>(ROB->table[ROBSpot].value));

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
	// Note: we can only commit 1 instruction per cycle EXCEPT stores take multiple cycles but don't block other commits.
	// If store instructions --> write into memory (takes multiple cycles, doesn't block)
	// If other instruction --> write to ARF
	// Free ROB entry and update RAT (clear aliases). Advance ROB head to the next instruction.
	// Mark instruction as committed (record commit cycle for table).

	// First, check if a store commit is in progress
	if (storeCommitInProgress != -1)
	{
		inst &storeInst = instruction[storeCommitInstrIndex];
		
		// Check if the store has finished its memory write (takes numMemLoadStore cycles)
		if ((currentCycle - storeCommitStartCycle + 1) >= numMemLoadStore)
		{
			// Store commit is complete
			int ROBSpot = storeCommitInProgress;
			
			// Write to memory now (at the end of the multi-cycle commit)
			int address = static_cast<int>(ROB->table[ROBSpot].value); // Address was stored in ROB
			float storeValue = FpARF->getValue(storeInst.rs.id); // Get data from register
			
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
			
			// Clear the store from memRS
			int RSSpot = memRS->findRSFromROB(ROBSpot);
			if (RSSpot != -1)
			{
				memRS->clearLocation(RSSpot);
			}
			
			// Commit the store from ROB
			ROB->commit(ROBSpot);
			
			// Mark commit complete in timing diagram
			timingDiagram[storeCommitInstrIndex*numCol + 4].endCycle = currentCycle;
			
			// Now that we've committed, the memory bus will be free at the next cycle
			memoryBusFreeAtCycle = currentCycle + 1;
			
			// Clear the store commit tracking
			storeCommitInProgress = -1;
			storeCommitStartCycle = 0;
			storeCommitInstrIndex = -1;
			
			success = true;

			// If store has finished memory write, return now
			return success;
		}
		else
		{
			// Store is still committing, but we can commit other instructions in parallel
		}
	}

	// Try to commit the next ready instruction (even if store is in progress)
	int ROBSpot;
	for (size_t h = 0; h < numberInstructions; ++h) 
	{
		// Skip if this instruction already committed
		if (timingDiagram[h*numCol + 4].endCycle != 0)
		{
			continue;
		}
		
		ROBSpot = timingDiagram[h*numCol + 0].numROB;

		//std::cout << "\nCycle " << currentCycle << ": Checking if instr #" << (h+1) << " can commit (ROB spot " << ROBSpot << ")" << std::endl;

		inst &ins = instruction[h];
		
		// Determine if instruction can commit
		bool canCommit = false;
		
		if (ins.opcode == store)
		{

			/*
			std::cout << "\nCycle " << currentCycle << ": Checking store (instr #" << (h+1) << ")" << std::endl;
			std::cout << "  EX endCycle: " << timingDiagram[h*numCol + 1].endCycle << std::endl;
			std::cout << "  storeCommitInProgress: " << storeCommitInProgress << std::endl;
			std::cout << "  currentCycle >= memoryBusFreeAtCycle: " << currentCycle << " >= " << memoryBusFreeAtCycle << std::endl;
			std::cout << "  stepThisCycle: " << timingDiagram[h*numCol + 0].stepThisCycle << std::endl;
			*/


			// Store can commit if:
			// 1. EX stage is done
			// 2. No store is currently committing
			// 3. Data dependency is resolved
			// 4. It's the oldest non-committed instruction (program order)
			if (timingDiagram[h*numCol + 1].endCycle != 0 &&
				storeCommitInProgress == -1 &&
				currentCycle >= memoryBusFreeAtCycle &&
				timingDiagram[h*numCol + 0].stepThisCycle == false)
			{
				// Check if all earlier instructions have committed
				bool allEarlierCommitted = true;
				for (size_t i = 0; i < h; i++)
				{
					if (timingDiagram[i*numCol + 4].endCycle == 0)
					{
						allEarlierCommitted = false;
						//std::cout << "  Earlier instr #" << (i+1) << " not committed yet" << std::endl;
						break;
					}
				}
				//std::cout << "  allEarlierCommitted: " << allEarlierCommitted << std::endl;

				if (allEarlierCommitted)
				{
					// Check if data is ready
					int floatDependency = 1;
					int dataRegID = ins.rs.id;
					int dataDep = ROB->findDependency(floatDependency, dataRegID);
					
					/*
					std::cout << "  dataDep: " << dataDep << std::endl;
					if (dataDep != -1)
					{
						std::cout << "  ROB[" << dataDep << "].cmt_flag: " << ROB->table[dataDep].cmt_flag << std::endl;
					}
					*/

					if (dataDep == -1 || ROB->table[dataDep].cmt_flag == 1)
					{
						canCommit = true;
						//std::cout << "  >>> Store can commit!" << std::endl;
					}
				}
			}
		}
		else
		{
			// Non-store instruction can commit if WB is done and it's oldest
			if (timingDiagram[h*numCol + 3].endCycle != 0 &&
				timingDiagram[h*numCol + 0].stepThisCycle == false)
			{
				// Check if all earlier instructions have committed
				bool allEarlierCommitted = true;
				for (size_t i = 0; i < h; i++)
				{
					if (timingDiagram[i*numCol + 4].endCycle == 0)
					{
						allEarlierCommitted = false;
						break;
					}
				}
				
				if (allEarlierCommitted)
				{
					canCommit = true;
				}
			}
		}
		
		if (!canCommit)
		{
			continue;
		}
		
		// Commit this instruction
		if (ins.opcode == store)
		{
			// Start multi-cycle store commit
			storeCommitInProgress = ROBSpot;
			storeCommitStartCycle = currentCycle;
			storeCommitInstrIndex = h;
			
			// Mark start of commit in timing diagram
			timingDiagram[h*numCol + 4].startCycle = currentCycle;
			// endCycle will be set when store finishes
			
			success = true;
			break;
		}
		else
		{
			// Regular, non-store instructions commit in one cycle
			commitReturn robCommit = ROB->commit(ROBSpot);

			/*
			std::cout << "\nCommitting instr #" << (h+1) << " at cycle " << currentCycle << std::endl;
			std::cout << "  regType: " << robCommit.regType << std::endl;
			std::cout << "  registerNum: " << robCommit.registerNum << std::endl;
			std::cout << "  returnValue: " << robCommit.returnValue << std::endl;
			*/

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

				/*
				std::cout << "\nCommitting float result:" << std::endl;
				std::cout << "  ROB spot: " << ROBSpot << std::endl;
				std::cout << "  Value: " << robCommit.returnValue << std::endl;
				std::cout << "  Dest register: F" << robCommit.registerNum << std::endl;
				std::cout << "  RAT lookup returned: " << regLocation << std::endl;
				*/

				while (regLocation != -1)
				{
					//std::cout << "  Writing " << robCommit.returnValue << " to F" << regLocation << std::endl;
					FpARF->changeValue(regLocation, robCommit.returnValue);
					regLocation = FpRAT->getNextARFLocation(ROBSpot);
				}
			}
			else if (robCommit.regType == -2)
			{
				// This section is intentionally blank
				// For store instructions we set regType to -2 since no register write is needed. We can't
				// use -1 since that indicates that a location is free.
			}

			// Regular, non-store instructions commit in one cycle
			timingDiagram[h*numCol + 4].startCycle = currentCycle;
			timingDiagram[h*numCol + 4].endCycle = currentCycle;
			
			success = true;
			break;
		}
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
            int start = timingDiagram[i*numCol+j].startCycle;
            int end = timingDiagram[i*numCol+j].endCycle;
            
            // Display -1,-1 as 0,0 (for skipped stages like store MEM/WB)
            if (start == -1)
			{
				start = 0;
			}
            if (end == -1)
			{
				end = 0;
			}
            
            std::cout << start << "," << end << "\t";
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

bool Tomasulo::allInstructionsCommitted()
{
    for (int i = 0; i < numberInstructions; i++)
    {
        if (timingDiagram[i*numCol + 4].endCycle == 0)
        {
            return false;
        }
    }
    return true;
}