#include "Tomasulo.hpp"


Tomasulo::Tomasulo(int numRow, int numberInstructions, int numExInt, int numExFPAdd, int numExFPMult, int numExLoadStore, int numMemLoadStore)
	: numRow(numRow) , numberInstructions(numberInstructions), numExInt(numExInt), numExFPAdd(numExFPAdd), numExFPMult(numExFPMult),
		numExLoadStore(numExLoadStore), numMemLoadStore(numMemLoadStore)
{

}


// TODO: There are errors here because, for example, instruction cannot be accessed here. I'm ignoring for now since I can't build anyway
//       I may also just be accessing things in the wrong way.\
// Classes we will need for the issue cycle:
// ARF
// RAT
// ROB
// RS
// Instructions
bool Tomasulo::issue(ARF<int> IntARF, ARF<float> FpARF)
{
	bool success = false;

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




	
	
	// Read operands in registers (if not available yet, record which RS will eventually produce the result)
	// Register renaming, update RAT with source/target for new ROB entry




	// Branch prediction will eventually happen here.
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