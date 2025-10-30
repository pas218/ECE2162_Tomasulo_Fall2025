// main.cpp
//#include "../lib/ARF.hpp"
#include "ARF.hpp"
#include "CDB.hpp"
#include "RS.hpp"
#include "input_parser_v2.hpp"
#include "Tomasulo.hpp"
#include <iostream>
#include <vector>




// Global variables
int num_ROB=0;
int currentCycle = 0;




Operation opcode;
item dst,src,tgt;
InstBuf ib;

//IntARF *IntArf;
//FpARF *FpArf;
//IntRAT *IntRat;
//FpRAT *FpRat;
ReOrderBuf *ROB;

//RS *memRS;
RS_entry e_m, m_w, w_c;
AddIUnit *addiunit, *memunit, *memunit2;
AddFUnit *addfunit;
MulFUnit *mulfunit;


// Vectors for storing memory and instructions
std::vector<mem_unit> memory;
std::vector<inst> instruction;



int main()
{
	
	ARF<int> *IntARF;
	ARF<float> *FpARF;
	RAT<int> *IntRAT;
	RAT<float> *FpRAT;
	RS<int, Ops> *addiRS;
	RS<float, Ops> *addfRS;
	RS<float, Ops> *mulfRS;
	
    std::cout << "Begin main.cpp" << std::endl;
	
	int numARF = 32;
    
	
	InputParser parser(numARF);
    // Parse the input.txt file and print the input configuration that has been read
    parser.parse("src\\input.txt");
	
	
	IntARF = new ARF<int>(parser.intARFValues);
    FpARF  = new ARF<float>(parser.floatARFValues);
	IntRAT = new RAT<int>(numARF);
	FpRAT  = new RAT<float>(numARF);

	addiRS = new RS<int, Ops>(parser.num_addiRS);
	addfRS = new RS<float, Ops>(parser.num_addfRS);
	mulfRS = new RS<float, Ops>(parser.num_mulfRS);

	// Print out ARF to test.
	for (int i = 0; i < numARF; i ++)
	{
		std::cout << "IntARF[" << i << "] " << " = " << IntARF->getValue(i) << std::endl;
		std::cout << "FloatARF[" << i << "] " << " = " << std::to_string(FpARF->getValue(i)) << std::endl;
	}

	// I'm not sure if the CDB should be int or float...?
    CDB<int> intCDB(parser.num_CDB_buf);
    CDB<float> floatCDB(parser.num_CDB_buf);


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
	int rowNum = parser.instruction.size();
	//Tomasulo Tomasulo(rowNum, parser.cycle_addi, parser.cycle_addf, parser.cycle_mulf, parser.cycle_mem_exe , parser.cycle_mem_mem);
	Tomasulo Tomasulo(3, 1, 3, 20, 1 , 4);
	Tomasulo.issue(IntARF, FpARF, addiRS, addfRS, mulfRS, IntRAT, FpRAT);
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
        execute();
        mem();
        writeback();
        commit();
        


    }
    */

	// Output the final results to the terminal
	//parser.output();

	std::cout << "\nProgram End" << std::endl;




	/* I had to comment out the section below for now because it was causing the build to fail
	std::cout << "Hi!" << std::endl;
	ARF<int> myARF(10);;
	std::cout << "The value of ARF(0) " << myARF.getValue(0) << std::endl;
    myARF.changeValue(0, 5); 
	std::cout << "The new value of ARF(0) " << myARF.getValue(0) << std::endl;
	*/

	// Delete dynamically allocated poiters;
	delete IntARF;
    delete FpARF;
	delete IntRAT;
	delete FpRAT;
	delete addiRS;
	delete addfRS;
	delete mulfRS;
	
	return 0;
}