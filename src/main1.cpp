// main.cpp
//#include "../lib/ARF.hpp"
#include "../lib/CDB.hpp"
#include "../lib/input_parser.h"
#include <iostream>
#include <vector>




// Global variables
int num_ROB=0;
int num_addiRS=0;
int num_addfRS=0;
int num_mulfRS=0;
int num_memRS=0;
int cycle_addi=0;
int cycle_addf=0;
int cycle_mulf=0;
int cycle_mem_exe=0;
int cycle_mem_mem=0;
int num_addi=0;
int num_addf=0;
int num_mulf=0;
int num_mem=0;
int currentCycle = 0;

// External globals declared in input_parser.cpp
extern int num_CDB_buf;
extern std::vector<mem_unit> memory;
extern std::vector<inst> instruction;




Operation opcode;
item dst,src,tgt;
InstBuf ib;

IntARF *IntArf;
FpARF *FpArf;
IntRAT *IntRat;
FpRAT *FpRat;
ReOrderBuf *ROB;

RS *addiRS,*addfRS,*mulfRS,*memRS;
RS_entry e_m, m_w, w_c;
AddIUnit *addiunit, *memunit, *memunit2;
AddFUnit *addfunit;
MulFUnit *mulfunit;


// Vectors for storing memory and instructions
std::vector<mem_unit> memory;
std::vector<inst> instruction;



int main()
{
    std::cout << "Begin main.cpp" << std::endl;

    // Initialize 32 int and FP registers
    IntArf = new IntARF(32);
    FpArf  = new FpARF(32);

    // Parse the input.txt file and print the input configuration that has been read
    InputParser::parse("input.txt");



	// I'm not sure if the CDB should be int or float...?
    CDB<int> intCDB(num_CDB_buf);
    CDB<float> floatCDB(num_CDB_buf);


	// Example: push a value to the CDB
    std::cout << "Pushing value 42 to integer CDB..." << std::endl;
    bool success = intCDB.push(1, 5, 42);  // robID=1, destReg=5, value=42
    if (!success)
        std::cout << "Failed to push to CDB." << std::endl;

    // Example: pop a value from the CDB
    CDB_entry<int> entry;
    if (intCDB.pop(entry))
    {
        std::cout << "Popped from CDB: robID=" << entry.robID
                  << ", destReg=" << entry.destReg
                  << ", value=" << entry.value << std::endl;
    }
    else
    {
        std::cout << "CDB is empty, nothing to pop." << std::endl;
    }




    

	std::cout << "Begin Tomasulo algorithm" << std::endl;

    /*
	// ...
	// The actual issue/ex/mem/wb/commit process happens here
	// ...
    while (the last instruction has not been committed)
    {

        currentCycle++;

        // We could consider creating a new file for each of the functions below if there will be a lot of helper functions,
        // or maybe just one with everything.
        issue();
            // Fetch and decode instruction from instruction queue (record issue cycle for table)
            // Check if there are free RS for instruction and ROB entry (stall if not available), then allocate for new instruction
            // Read operands in registers (if not available yet, record which RS will eventually produce the result)
            // Register renaming, update RAT with source/target for new ROB entry
            // Branch prediction will eventually happen here.
        execute();
            // Execute instructions once all operands are ready (record execute cycle for table)
            // "Compete" to use functional unit (oldest first)
            // Advance instruction count for each instruction in execution. Wait for the number of cycles read from the input before marking the instruction as ready (cycle_addi, cycle_addf, etc.)
            // When an FU's execution completes, mark the RS entry as 'finished' so that it can be written on the next writeback cycle
            // For load/store, also calculate effective address (does not occupy integer ALU to do this).
            // Eventually, branch resolution will happen here
        mem();
            // Record mem cycle for table
            // Check for "forwarding-from-a-store" as mentioned in project document. It takes 1 cycle to perform the forwarding if a match is found. If not found then the load accesses the memory for data.
            // Once a load returns from the memory or gets the value from a previous store, its entry in the load/store queue is cleared.
            // Note that it is correct not to clear this entry, but the queue can quickly fill up, causing structure hazards for future loads/stores.
        writeback();
            // Broadcast results onto the CDB (other instructions waiting on it will need to pick it up), or buffer if the CDB is full
            // Write result back to RS and ROB entry (record WB cycle for table)
            // Mark the ready/finished bit in ROB since the instruction has completed execution
            // Free reservation stations for future reuse when finished
            // Store instructions write to memory in this stage
        commit();
            // Commit an instruction when it is the oldest in the ROB (ROB head points to it) and the ready/finished bit is set.
            // Note: we can only commit 1 instruction per cycle.
            // If store instructions --> write into memory
            // If other instruction --> write to ARF
            // Free ROB entry and update RAT (clear aliases). Advance ROB head to the next instruction.
            // Mark instruction as committed (record commit cycle for table).



    }
    */

	// Output the final results to the terminal
	InputParser::output();

	std::cout << "\nProgram End" << std::endl;




	/* I had to comment out the section below for now because it was causing the build to fail
	std::cout << "Hi!" << std::endl;
	ARF<int> myARF(10);;
	std::cout << "The value of ARF(0) " << myARF.getValue(0) << std::endl;
    myARF.changeValue(0, 5); 
	std::cout << "The new value of ARF(0) " << myARF.getValue(0) << std::endl;
	*/

	return 0;
}