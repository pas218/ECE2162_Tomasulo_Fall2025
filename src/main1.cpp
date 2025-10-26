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



	// I'm not sure if we need a separate int and float CDB...?
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
	// ...
	// The actual issue/ex/mem/wb/commit process happens here
	// ...





	// Output the final results to the terminal
	InputParser::output();





	/* I had to comment out the section below for now because it was causing the build to fail
	std::cout << "Hi!" << std::endl;
	ARF<int> myARF(10);;
	std::cout << "The value of ARF(0) " << myARF.getValue(0) << std::endl;
    myARF.changeValue(0, 5); 
	std::cout << "The new value of ARF(0) " << myARF.getValue(0) << std::endl;
	*/

	return 0;
}