// main.cpp
//#include "../lib/ARF.hpp"
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
extern int num_CDB;
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
ADDIER *addier, *memer, *memer2;
ADDFER *addfer;
MULFER *mulfer;


// Vectors for storing memory and instructions
std::vector<mem_unit> memory;
std::vector<inst> instruction;



int main()
{
    std::cout << "Begin main.cpp" << std::endl;

    // Initialize 32 int and FP registers
    IntArf = new IntARF(32);
    FpArf  = new FpARF(32);

    // Parse the input.txt file in input_parser.cpp
    InputParser::parse("input.txt");

	
    // Print the parsed results from input.txt to the terminal
    //------------------------------------------------------------------------------------------
    std::cout << "\n-----------------------------------------------\n";
    std::cout << "Printing information parsed from input.txt...\n\n";
    std::cout << "Integer adder: " << num_addiRS << " RS, " << cycle_addi << " EX cycle(s), " << num_addi << " FU\n";
    std::cout << "FP adder: " << num_addfRS << " RS, " << cycle_addf << " EX cycle(s), " << num_addf << " FU\n";
    std::cout << "FP multiplier: " << num_mulfRS << " RS, " << cycle_mulf << " EX cycle(s), " << num_mulf << " FU\n";
    std::cout << "Load/store unit: " << num_memRS << " RS, " << cycle_mem_exe << " EX cycle(s), " << cycle_mem_mem << " MEM cycle(s), " << num_mem << " FU\n\n";
    std::cout << "ROB entries: " << num_ROB << "\n";
    std::cout << "CDB count: " << num_CDB << "\n\n";

    // Print nonzero int registers
    if (IntArf)
    {
        //std::cout << "Int Registers (nonzero):\n";
        for (int i = 0; i < IntArf->pointer; i++)
        {
            if (IntArf->table[i].value != 0)
            {
                std::cout << "R" << i << " = " << IntArf->table[i].value << "\n";
            }
        }
    }

    // Print nonzero FP registers
    if (FpArf)
    {
        //std::cout << "FP Registers (nonzero):\n";
        for (int i = 0; i < FpArf->pointer; i++)
        {
            if (FpArf->table[i].value != 0)
            {
                std::cout << "F" << i << " = " << FpArf->table[i].value << "\n";
            }
        }
    }

    // Print nonzero memory
    if (memory.empty())
    {
        std::cout << "(No memory entries found)\n";
    }
    else
    {
        //std::cout << "Memory (nonzero):\n";
        for (auto &m : memory) {
            std::cout << "Mem[" << m.first << "] = " << m.second << "\n";
        }
    }

    // Print all instructions in the same format as they were read
    if (!instruction.empty())
    {
        std::cout << "\nInstructions loaded:\n";
        for (size_t i = 0; i < instruction.size(); ++i)
        {
            const inst &it = instruction[i];
            std::cout << i + 1 << ": ";

            switch (it.opcode)
            {
                case addi:
                    std::cout << "Add R" << it.rd.id << ", R" << it.rs.id << ", R" << it.rt.id;
                    break;
                case subi:
                    std::cout << "Sub R" << it.rd.id << ", R" << it.rs.id << ", R" << it.rt.id;
                    break;
                case addf:
                    std::cout << "Add.d F" << it.rd.id << ", F" << it.rs.id << ", F" << it.rt.id;
                    break;
                case subf:
                    std::cout << "Sub.d F" << it.rd.id << ", F" << it.rs.id << ", F" << it.rt.id;
                    break;
                case mulf:
                    std::cout << "Mul.d F" << it.rd.id << ", F" << it.rs.id << ", F" << it.rt.id;
                    break;
                case load:
                    std::cout << "Ld F" << it.rd.id << ", " << it.rt.value << "(R" << it.rs.id << ")";
                    break;
                case store:
                    std::cout << "Sd F" << it.rs.id << ", " << it.rt.value << "(R" << it.rd.id << ")";
                    break;
                case beq:
                    std::cout << "Beq R" << it.rs.id << ", R" << it.rt.id << ", " << it.rt.value;
                    break;
                case bne:
                    std::cout << "Bne R" << it.rs.id << ", R" << it.rt.id << ", " << it.rt.value;
                    break;
                default:
                    std::cout << "(Unknown opcode: " << it.name << ")";
                    break;
            }

            std::cout << "\n";
        }
    }
    else
    {
        std::cout << "\nNo instructions found.\n";
    }

    std::cout << "-----------------------------------------------\n";
    //------------------------------------------------------------------------------------------






	/* I had to comment out the section below for now because it was causing the build to fail
	std::cout << "Hi!" << std::endl;
	ARF<int> myARF(10);;
	std::cout << "The value of ARF(0) " << myARF.getValue(0) << std::endl;
    myARF.changeValue(0, 5); 
	std::cout << "The new value of ARF(0) " << myARF.getValue(0) << std::endl;
	*/

	return 0;
}