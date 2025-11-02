// main.cpp
//#include "../lib/ARF.hpp"
#include "ARF.hpp"
#include "CDB.hpp"
#include "RS.hpp"
#include "input_parser_v2.hpp"
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

//IntARF *IntArf;
//FpARF *FpArf;
//IntRAT *IntRat;
//FpRAT *FpRat;

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
	ReOrderBuf *ROB;
	
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
	
	ROB    = new ReOrderBuf(parser.num_ROB);
	
	/*
	class inst
{
public:
	Operation opcode;
	item rd;
	item rs;
	item rt;
	std::string name;
	std::string sd_offset;
	int t_issue;
	int t_ex;
	int t_mem;
	int t_wb;
	int t_commit;
	bool cmt;
	inst():t_issue(0),t_ex(0),t_mem(0),t_wb(0),t_commit(0),cmt(0){}
	void reset(){
		t_issue=0;
		t_ex=0;
		t_mem=0;
		t_wb=0;
		t_commit=0;
		cmt=false;
	}
};
*/
/*
	// Test out the instructions.
	std::cout << "Instruction testing.\n";
	for (int i = 0; i < parser.instruction.size(); i++)
	{
		std::cout << parser.instruction[i].opcode << " " << parser.instruction[i].rd.id << " " 
			<< parser.instruction[i].rd.id << " " << parser.instruction[i].rt.id << std::endl;
	}
	*/

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
	Tomasulo *Tommy;
	Tommy = new Tomasulo(parser.instruction.size(), parser.cycle_addi, parser.cycle_addf, parser.cycle_mulf, 
		parser.cycle_mem_exe, parser.cycle_mem_mem, IntARF, FpARF, addiRS, addfRS, mulfRS, IntRAT, FpRAT, ROB, parser.instruction);
	Tommy->issue();
	Tommy->issue();
	Tommy->issue();
	Tommy->issue();
	Tommy->printOutTimingTable();
	//Tommy->printRAT(false);
	//Tommy->printRAT(true);
	//Tommy->printARF(false);
	//Tommy->printARF(true);
	Tommy->printRS(0);
	Tommy->printRS(1);
	Tommy->printROB();
	Tommy->printRAT(0);
	Tommy->printRAT(1);
	Tommy->printARF(0);
	Tommy->printARF(1);
	std::cout << "After\n";
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
	delete ROB;
	delete Tommy;
	
	return 0;
}