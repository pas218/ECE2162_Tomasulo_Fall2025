#include "Tomasulo.hpp"


Tomasulo::Tomasulo(int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore,
			ARF<int> *IntARF, ARF<float> *FpARF, RS<int, Ops> *addiRS, RS<float, Ops> *addfRS, RS<float, Ops> *mulfRS, RAT<int> *IntRAT, 
				RAT<float> *FpRAT, ReOrderBuf *ROB)
	: numRow(numberInstructions) , numberInstructions(numberInstructions), numExInt(numExInt), numExFPAdd(numExFPAdd), numExFPMult(numExFPMult),
		numExLoadStore(numExLoadStore), numMemLoadStore(numMemLoadStore)
{
	this->IntARF = IntARF;
	this->FpARF  = FpARF;
	this->addiRS = addiRS;
	this->addfRS = addfRS;
	this->mulfRS = mulfRS;
	this->IntRAT = IntRAT;
	this->FpRAT  = FpRAT;
	this->ROB    = ROB;
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
    std::cout << "IntARF pointer inside issue: " << IntARF << std::endl;
	std::cout << "In issue.\n";
    std::cout << IntARF->getValue(2) << std::endl;
	std::cout << std::to_string(FpARF->getValue(2)) << std::endl; 
	std::cout << "Now the RAT.\n";
	std::cout << IntRAT->getValue(2).locationType << IntRAT->getValue(2).locationNumber << std::endl;

	bool success = false;
/*
    // Loop over all instructions to find the first one that hasn't been issued
    for (size_t i = 0; i < numberInstructions; ++i)
    {
        inst &ins = instruction[i];

		// If the t_issue field is greater than 0, the instruction has already been issued
        if (ins.t_issue > 0) continue;

        int freeRS = -1;
        int freeROB = -1;

		// Check if there is a free RS for instruction (stall if not available)
        if (ins.opcode == addi || ins.opcode == subi)
        {
            for (int i = 1; i < addiRS->size; i++)
			{
                if (addiRS->table[i].robLocation == -1)
				{
					freeRS = i;
					break;
				}
			}
        }
        else if (ins.opcode == addf || ins.opcode == subf)
        {
            for (int i = 1; i < addfRS->size; i++)
			{
                if (addfRS->table[i].robLocation == -1)
				{
					freeRS = i; break;
				}
			}
        }
        else if (ins.opcode == mulf)
        {
            for (int i = 1; i < mulfRS->size; i++)
			{
                if (mulfRS->table[i].robLocation == -1)
				{
					freeRS = i;
					break;
				}
			}
        }

		// If there are no free RS, we must stall (success is false)
        if (freeRS == -1)
		{
			return success;
		}


		// Check if there is a free ROB entry for instruction (stall if not available)
		for (int i = 1; i < ROB->size; i++)
        {
			if (ROB->table[i].id == 0) // If ROB ID is 0, it's free
            {
                freeROB = i;
                ROB->table[i].id = i + 1; // set ID to indicate it's no longer free
                ROB->table[i].dst_id = ins.rd.id;
                ROB->table[i].cmt_flag = 0;
                break;
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
			addiRS->table[freeRS].operation = ADD;
		}
        else if (ins.opcode == subi)
		{
			addiRS->table[freeRS].operation = SUB;
		}
        else if (ins.opcode == addf)
		{
			addfRS->table[freeRS].operation = FP_ADD;
		}
        else if (ins.opcode == subf)
		{
			addfRS->table[freeRS].operation = FP_SUB;
		}
        else if (ins.opcode == mulf)
		{
			mulfRS->table[freeRS].operation = FP_MULT;
		}

        if (ins.opcode == addi || ins.opcode == subi)
        {
            addiRS->table[freeRS].robLocation = freeROB + 1;
            addiRS->table[freeRS].value0 = IntARF->table[ins.rs.id].value;
            addiRS->table[freeRS].value1 = IntARF->table[ins.rt.id].value;
            addiRS->table[freeRS].robDependency0 = -1;
            addiRS->table[freeRS].robDependency1 = -1; // Assume no dependency for now?
        }
        else if (ins.opcode == addf || ins.opcode == subf)
        {
            addfRS->table[freeRS].robLocation = freeROB + 1;
            addfRS->table[freeRS].value0 = FpARF->table[ins.rs.id].value;
            addfRS->table[freeRS].value1 = FpARF->table[ins.rt.id].value;
            addfRS->table[freeRS].robDependency0 = -1;
            addfRS->table[freeRS].robDependency1 = -1; // Assume no dependency for now?
        }
        else if (ins.opcode == mulf)
        {
            mulfRS->table[freeRS].robLocation = freeROB + 1;
            mulfRS->table[freeRS].value0 = FpARF->table[ins.rs.id].value;
            mulfRS->table[freeRS].value1 = FpARF->table[ins.rt.id].value;
            mulfRS->table[freeRS].robDependency0 = -1;
            mulfRS->table[freeRS].robDependency1 = -1; // Assume no dependency for now?
        }

		// Since issue was successful, record issue cycle for timing table using gloabl counter
        ins.t_issue = currentCycle;
        success = true;

		// We can only issue one instr per cycle, so go ahead and break from for loop
        break;
    }

    return success;


*/

	
	// Read operands in registers (if not available yet, record which RS will eventually produce the result)
	// Register renaming, update RAT with source/target for new ROB entry




	// Branch prediction will eventually happen here.
	std::cout << "end\n";
    return success;
}


// Classes we will need for execute stage
bool Tomasulo::execute()
{
	bool success = false;
	
	// Execute instructions once all operands are ready (record execute cycle for table)
	// "Compete" to use functional unit (oldest first)
	// Advance instruction count for each instruction in execution. Wait for the number of cycles read from the input before marking the instruction as ready (cycle_addi, cycle_addf, etc.)
	// When an FU's execution completes, mark the RS entry as 'finished' so that it can be written on the next writeback cycle
	// For load/store, also calculate effective address (does not occupy integer ALU to do this).
	// Eventually, branch resolution will happen here






	return success;
}


// Classes we will need for the mem cycle:
// ROB
// RS
// Instructions
bool Tomasulo::mem()
{
	bool success = false;
	
	// Go through instructions to find loads/stores ready for MEM stage
    for (int i = 1; i < numberInstructions; i++)
    {
        inst &ins = instruction[i];

        // Find the first load/store instruction that has completed execution but not mem yet.
        if ((ins.t_ex == 1) && (ins.t_mem == 0) && (ins.opcode == load || ins.opcode == store))
        {
            // Record MEM cycle for the table
            ins.t_mem = currentCycle;
            success = true;

            // TODO: check for "forwarding-from-a-store" as mentioned in project document
            // (This is when previous store writes to same address).
            // It takes 1 cycle to perform the forwarding if a match is found.
            // If not found then the load accesses the memory for data.



            // TODO: once a load returns from the memory or gets the value from a previous store, its entry in the load/store queue is cleared.
	        // Note that it is correct not to clear this entry, but the queue can quickly fill up, causing structure hazards for future loads/stores.
        



            // Only do one memory access per cycle, so break
            break;
        }
    }


	return success;
}


// Classes we will need for the wb cycle:
// ARF
// RAT
// ROB
// Instructions
bool Tomasulo::wb()
{
	bool success = false;
	    
	// Find the first instruction that finished MEM but not WB yet
    for (int i = 1; i < numberInstructions; i++)
    {
        inst &ins = instruction[i];
        bool readyForWB = false;

        // We have to distinguish between memory and non-memory instructions.
        // Loads and stores writeback after MEM. Other instructions writeback after EX.
        if ((ins.opcode == load) || (ins.opcode == store))
        {
            // See if the memory instruction has completed MEM but not WB yet
            if ((ins.t_mem > 0) && (ins.t_wb == 0))
            {
                readyForWB = true;
            }
        }
        else
        {
            // Non-memory instructions will not have t_mem set, so check t_ex instead
            if ((ins.t_ex > 0) && (ins.t_wb == 0))
            {
                readyForWB = true;
            }
        }

        // If the instruction isn't ready for WB yet, check the next intruction in the next iteration
        if (!readyForWB)
            continue;

        // Instruction is ready for write-back, so update the table and mark as successful
        ins.t_wb = currentCycle;
        success = true;




        // TODO: set resultValue (what's written back and broadcast) based on the results from EX
        double resultValue = 0.0;
        if (ins.opcode == addi || ins.opcode == subi)
        {
            resultValue = 1; // TODO: resultValue = something, will come from EX stage?
        }
        else if (ins.opcode == addf || ins.opcode == subf)
        {
            resultValue = 1.0; // TODO: resultValue = something, will come from EX stage?

        }
        else if (ins.opcode == mulf)
        {
            resultValue = 1.0; // TODO: resultValue = something, will come from EX stage?
        }
        else if (ins.opcode == load)
        {
            resultValue = 1; // TODO: resultValue = something, will come from EX or MEM stage?
        }




        bool cdbPushed = false;

        // Broadcast (push) results on CDB if there is room. Otherwise, we must buffer it.
        // TODO: I'm not sure if we will actually have an intCDB and floatCDB or if these will be unified.
        if (ins.opcode != store)
        {
            if (ins.opcode == addi || ins.opcode == subi)
            {
                // Try to push onto integer CDB. If it returns false, it's full.
                if (!intCDB.push(ins.robID, ins.rd.id, (int)resultValue))
                {
                    std::cout << "Integer CDB full — instruction " << i << " will retry WB next cycle\n";
                    continue;
                }
                else
                {
                    cdbPushed = true;
                }
            }
            else
            {
                // Try to push onto float CDB. If it returns false, it's full.
                if (!floatCDB.push(ins.robID, ins.rd.id, (float)resultValue))
                {
                    std::cout << "Float CDB full — instruction " << i << " will retry WB next cycle\n";
                    continue;
                }
                else
                {
                    cdbPushed = true;
                }
            }
        }

        // Only update ROB and RS if we successfully broadcasted above
        if ((cdbPushed == true) || (ins.opcode == store))
        {
            // Update the ROB entry and mark as ready to commit
            for (int j = 0; j < ROB->size; j++)
            {
                if (ROB->table[j].id == ins.robID)
                {
                    ROB->table[j].value = resultValue;
                    ROB->table[j].cmt_flag = 1;
                    break;
                }
            }

            // Free any RS entries that were waiting on this ROB entry to broadcast
            for (int j = 0; j < addiRS->numStations; j++)
            {
                RS_type<int, Ops>* rs = addiRS->getValue(j);
                if (rs->robDependency0 > 0 || rs->robDependency1 > 0)
                {
                    if (rs->robDependency0 == ins.robID || rs->robDependency1 == ins.robID)
                    {
                        addiRS->clearLocation(j);
                    }
                }
            }
            for (int j = 0; j < addfRS->numStations; j++)
            {
                RS_type<float, Ops>* rs = addfRS->getValue(j);
                if (rs->robDependency0 > 0 || rs->robDependency1 > 0)
                {
                    if (rs->robDependency0 == ins.robID || rs->robDependency1 == ins.robID)
                    {
                        addfRS->clearLocation(j);
                    }
                }
            }
            for (int j = 0; j < mulfRS->numStations; j++)
            {
                RS_type<float, Ops>* rs = mulfRS->getValue(j);
                if (rs->robDependency0 > 0 || rs->robDependency1 > 0)
                {
                    if (rs->robDependency0 == ins.robID || rs->robDependency1 == ins.robID)
                    {
                        mulfRS->clearLocation(j);
                    }
                }
            }

            // Store instructions write from the ROB into memory during the writeback stage
            if (ins.opcode == store)
            {
                int addr = ins.rd.id;
                if ((addr >= 0) && (addr < (int)memory.size()))
                {
                    memory[addr].first = (int)ROB->table[ins.robID].value;
                }
            }
        }

        // We can only writeback 1 instruction per cycle, so break
        break;
    }

	return success;
}


// Classes we will need for the commit cycle:
// ARF
// RAT
// ROB
// Instructions
bool Tomasulo::commit()
{
	bool success = false;

	// Get the ROB entry at the current head position
    ReOrderBuf_entry &headEntry = ROB->table[ROB->head];

	// Commit oldest instruction (head of ROB) if cmt_flag is set (is ready to commit)
    if ((headEntry.id != 0) && (headEntry.cmt_flag == 1))
    {
        // Find matching instruction
        for (int i = 1; i < numberInstructions; i++)
        {
            inst &ins = instruction[i];

			// Find the first instruction that has been written back but not committed yet
            if ((ins.t_wb > 0) && (ins.t_commit == 0))
            {
				// Mark instruction as committed (record commit cycle for table).
                ins.t_commit = currentCycle;
                ins.cmt = true;
                success = true;

                // If store instructions --> write into memory
                if (ins.opcode == store)
                {
                    memory[ins.rd.id].first = (int)headEntry.value;
                }
				// If other instruction --> write to ARF
                else
                {
                    IntARF->changeValue(ins.rd.id, (int)headEntry.value);
                }

				// We can only commit 1 instruction per cycle, so break
                break;
            }
        }

		// Free ROB entry and update RAT (clear aliases).
		headEntry.id = 0;
        headEntry.dst_id = 0;
        headEntry.value = 0.0;
        headEntry.cmt_flag = 0;

        // Advance ROB head to the next instruction. This is a circular buffer index.
        ROB->head = (ROB->head + 1) % ROB->size;
    }

	return success;
}