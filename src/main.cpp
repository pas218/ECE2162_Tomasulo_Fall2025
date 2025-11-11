// main.cpp
#include "ARF.hpp"
#include "CDB.hpp"
#include "RS.hpp"
#include "InputParser.hpp"
#include "ReOrderBuf.hpp"
#include "Tomasulo.hpp"
#include <iostream>
#include <vector>

// Global variables
int num_ROB=0;
int currentCycle = 0;

Operation opcode;
item dst,src,tgt;
InstBuf ib;

RS_entry e_m, m_w, w_c;
AddIUnit *addiunit, *memunit, *memunit2;
AddFUnit *addfunit;
MulFUnit *mulfunit;


// Vectors for storing memory and instructions
std::vector<mem_unit> memory;
std::vector<inst> instruction;



int main(int argc, char *argv[])
{
	// Find input filepath.
	std::string filepath;
	if (argc == 2)
	{
		std::cout << "File path provided. Using path: " << argv[1] << std::endl;
		filepath = argv[1];
	}
	else
	{
		std::cout << "NOTE: No file path provided. Using default file path: src/input.txt.\n";
		filepath = "src/input.txt";
	}	
	
	ARF<int> *IntARF;
	ARF<float> *FpARF;
	RAT<int> *IntRAT;
	RAT<float> *FpRAT;
	RS<int, Ops> *addiRS;
	RS<float, Ops> *addfRS;
	RS<float, Ops> *mulfRS;
	RS<float, Ops> *memRS;
	ReOrderBuf *ROB;
	
    std::cout << "Begin main.cpp" << std::endl;
	
	int numARF = 16;
    
	
	InputParser parser(numARF);

    // Parse the input.txt file and print the input configuration that has been read
    parser.parse(filepath);
	
	IntARF = new ARF<int>(parser.intARFValues);
    FpARF  = new ARF<float>(parser.floatARFValues);
	IntRAT = new RAT<int>(numARF);
	FpRAT  = new RAT<float>(numARF);
	
	addiRS = new RS<int, Ops>(parser.num_addiRS);
	addfRS = new RS<float, Ops>(parser.num_addfRS);
	mulfRS = new RS<float, Ops>(parser.num_mulfRS);
	memRS = new RS<float, Ops>(parser.num_memRS);
	
	ROB    = new ReOrderBuf(parser.num_ROB);
	


	// I'm not sure if the CDB should be int or float...?
    //CDB<int> intCDB(parser.num_CDB_buf);
    //CDB<float> floatCDB(parser.num_CDB_buf);



	std::cout << "Begin Tomasulo algorithm..." << std::endl;

	Tomasulo *Tommy;
	Tommy = new Tomasulo(parser.instruction.size(), parser.cycle_addi, parser.cycle_addf, parser.cycle_mulf,
		parser.cycle_mem_exe, parser.cycle_mem_mem, parser.num_CDB_buf, IntARF, FpARF, addiRS, addfRS, mulfRS,
		memRS, IntRAT, FpRAT, ROB, parser.instruction, &parser.memory);

	// Run the Tomasulo algorithm
	for (int i = 0; i < 50; i++) // TODO: Dynamically set the total number of iterations? Or, just make this a parameter.
	{
		Tommy->fullAlgorithm();
	}
	Tommy->trimDiagramEnd();

	//Tommy->printRAT(false);
	//Tommy->printRAT(true);
	//Tommy->printARF(false);
	//Tommy->printARF(true);

	std::cout << "Tomasulo algorithm finished." << std::endl;


	// Print all results to the terminal
	Tommy->printOutput();

	std::cout << "\nProgram End" << std::endl;

	// Delete dynamically allocated pointers;
	delete IntARF;
    delete FpARF;
	delete IntRAT;
	delete FpRAT;
	delete addiRS;
	delete addfRS;
	delete mulfRS;
	delete memRS;
	delete ROB;
	delete Tommy;
	
	return 0;
}