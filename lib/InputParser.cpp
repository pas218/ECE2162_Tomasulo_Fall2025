// InputParser.cpp
#include "InputParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>


using namespace std;



extern Operation opcode;
extern item dst,src,tgt;
//extern InstBuf ib;
//extern IntARF *IntArf;
//extern FpARF *FpArf;
//extern IntRAT *IntRat;
//extern FpRAT *FpRat;
//extern ReOrderBuf *ROB;
extern AddIUnit *addiunit, *memunit, *memunit2;
extern AddFUnit *addfunit;
extern MulFUnit *mulfunit;

extern int currentCycle;		


item::item() : value(0.0f), id(0), imme_flag(false), ready(false) {}


AddIUnit::AddIUnit()
{
    empty = true;
    cycle = 0;
}

AddFUnit::AddFUnit()
{
    n = 1;
    line = new RS_entry[n];
    head = 0;
    tail = 0;
    size = 0;
    cycle = 0;
}

int AddFUnit::get_size()
{
    return size;
}
bool AddFUnit::empty()
{
    return size == 0;
}
bool AddFUnit::full()
{
    return size == n;
}

MulFUnit::MulFUnit()
{
    n = 1;
    line = new RS_entry[n];
    head = 0;
    tail = 0;
    size = 0;
    cycle = 0;
}

int MulFUnit::get_size()
{
    return size;
}
bool MulFUnit::empty()
{
    return size == 0;
}
bool MulFUnit::full()
{
    return size == n;
}


InputParser::InputParser(int numARF)
{
	int num_ROB = 0;
	int num_CDB_buf = 0;

	int num_addiRS = 0;
	int num_addfRS = 0;
	int num_mulfRS = 0;
	int num_memRS = 0;

	int cycle_addi = 0;
	int cycle_addf = 0;
	int cycle_mulf = 0;
	int cycle_mem_exe = 0;
	int cycle_mem_mem = 0;

	int num_addi = 0;
	int num_addf = 0;
	int num_mulf = 0;
	int num_mem = 0;
	
	for (int i = 0; i < numARF; i++)
	{
		intARFValues.push_back(0);
		floatARFValues.push_back(0.0f);

	}
}

// Main parsing function that reads the input file and stores all necessary parameters
void InputParser::parse(const std::string &filename)
{
    // Open the input text file
    ifstream input;
    input.open(filename.c_str());

    if (input.fail())
    {
        cout << "Can't open file: " << filename << endl;
        exit(-1);
    }

    char buf[MAX_BUFFER];

    // Skip the first line
    input.getline(buf, MAX_BUFFER);


    // Read the first table with 4 rows
	for (int i = 0; i < 4; i++)
    {
		input.getline(buf, MAX_BUFFER);
        
		char * word;
		vector<char *> words;
		string token;

        // Break apart into tokens based on spaces/tabs
		word = strtok(buf, DELIM0); // DELIM0 is " \t"
		while (word)
        {
			words.push_back(word);
			word = strtok(NULL, DELIM0);
		}

		token = words[0];

        // Read each row of the table, which will be given in a known order
		if (i == 0)
        {
            // Integer adder
            num_addiRS = atoi(words[2]);
            cycle_addi = atoi(words[3]);
            num_addi = atoi(words[4]);
		}
        else if (i == 1)
        {
            // FP adder
            num_addfRS = atoi(words[2]);
            cycle_addf = atoi(words[3]);
            num_addf = atoi(words[4]);
        }
        else if (i == 2)
        {
            // FP multiplier
            num_mulfRS = atoi(words[2]);
            cycle_mulf = atoi(words[3]);
            num_mulf = atoi(words[4]);
        }
        else
        {
            // Load/store unit
			num_memRS=atoi(words[2]);
			cycle_mem_exe=atoi(words[3]);
			cycle_mem_mem=atoi(words[4]);
			num_mem=atoi(words[5]);
		}
	}
	
    
    // Initalize reservation stations and functional units (this will likely change later)
	//memRS = new RS(num_memRS);
	addiunit = new AddIUnit[num_addi];
	memunit= new AddIUnit;
	memunit2= new AddIUnit;
	addfunit = new AddFUnit[num_addf];
	mulfunit = new MulFUnit[num_mulf];



    // Skip blank line
	input.getline(buf, MAX_BUFFER);



    // Read the second table with 4 rows
	for (int i = 0; i < 4; i++)
    {
		input.getline(buf, MAX_BUFFER);
		char * word;
		vector<char *> words;
		string token;

        // Break apart into tokens based on spaces/equal/commas/brackets
        word = strtok(buf, DELIM1); // DELIM1 is " =,[]"
        while (word)
        {
			words.push_back(word);
			word = strtok(NULL, DELIM1); //DELIM1
		}

		token = words[0];
		
        // First row is ROB entries
        if (i == 0) 
        {
			num_ROB=atoi(words[2]);
		}

        // Second row is number of CDB buffer entries
        else if (i == 1) 
        {
			num_CDB_buf = atoi(words[3]);
		}

        // Third row is initial integer register values
        else if (i == 2)
        {
            // Initial registers
			int k = 0;
			int max = words.size();

			while (k < max && words[k][0] == 'R')
            {
                int index = atoi(words[k] + 1);
                if (index < intARFValues.size())
                {
                    if (k + 1 < max)
                    {
                        intARFValues[index] = atoi(words[k + 1]);
                    }
                }
                else
                {
                    cout << "Warning: Register index " << index << " out of bounds\n";
                }
                k += 2;
            }
			
            //for(int j=0;j<36;j++) cout<<IntArf->table[j].id<<"	"<<IntArf->table[j].value<<endl;
			
            // Initialize floating-point registers
            for (int k = 0; k < words.size(); k += 2)
            {
                std::string token = words[k];
                float value = std::stof(words[k+1]);

                if (token[0] == 'F')
                {
                    int index = std::stoi(token.substr(1));
                    if (index < floatARFValues.size()) {
						floatARFValues[index] = value;
                    }
                    else
                    {
                        std::cout << "Warning: FP Register index " << index << " out of bounds\n";
                    }
                }
            }
		}

        // Last row is initial memory state
        else
        {
			int k = 0;
			int max = words.size();

			while (token.at(0) == 'M')
            {
				int index;
				index = atoi(words[k + 1]);

				mem_unit memtmp = make_pair(index, atof(words[k + 2]));
				memory.push_back(memtmp);

				k += 3;
				if (k <= max-2)
                {
                    token = words[k];
                }
                else
                {
                    break;
                }
            }
		}
	}



    // Instruction parsing

    // Skip blank line separating tables
    input.getline(buf, MAX_BUFFER);

    //cout << "\nReading instructions...\n";
    while (input.getline(buf, MAX_BUFFER))
    {
        vector<char *> words;
        char *word;

        // Choose token delimiter depending on instruction type
        if (strncmp(buf, "Ld", 2) == 0 || strncmp(buf, "Sd", 2) == 0)
            word = strtok(buf, DELIM2);  // DELIM2 is " =,[]"
        else
            word = strtok(buf, DELIM3);  // DELIM3 is " ,\t()" for Add/Sub/Beq/Bne

        while (word)
        {
            words.push_back(word);
            word = strtok(NULL, (strncmp(buf, "Ld", 2) == 0 || strncmp(buf, "Sd", 2) == 0) ? DELIM2 : DELIM3);
        }

        if (words.empty())
        {
            continue;
        }

        string op = words[0];

        // Create a temporary instruction
        inst itmp;
        itmp.name = op;

                
        //std::cout << "\n" << words[0] << " ";
        //for (size_t i = 1; i < words.size(); ++i)
        //    std::cout << words[i] << " ";
        //std::cout << std::endl;
		
		

        // Decode instruction and store all of the values that were read into the appropriate fields
        if (op == "Add" || op == "add" || op == "ADD")
        {
            itmp.opcode = add;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
		else if (op == "Addi" || op == "addi" || op == "ADDI")
		{
			itmp.opcode = addi;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.immediate = stoi(words[3] + 1);
		}
        else if (op == "Sub" || op == "sub" || op == "SUB")
        {
            itmp.opcode = sub;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Add.d" || op == "add.d" || op == "ADD.D")
        {
            itmp.opcode = addf;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Sub.d" || op == "sub.d" || op == "SUB.D")
        {
            itmp.opcode = subf;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Mult.d" || op == "mult.d" || op == "MULT.D")
        {
            itmp.opcode = mulf;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Ld" || op == "ld" || op == "LD")
        {
            itmp.opcode = load;
            itmp.rt.imme_flag = true;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rt.value = atof(words[2]);
            itmp.rs.id = stoi(words[3] + 1);
        }
        else if (op == "Sd" || op == "sd" || op == "SD")
        {
            itmp.opcode = store;
            itmp.rs.id = stoi(words[1] + 1);
            itmp.rt.value = atof(words[2]);
            itmp.rd.id = stoi(words[3] + 1);
            itmp.rt.imme_flag = true;
        }
        else if (op == "Beq" || op == "beq" || op == "BEQ")
        {
            itmp.opcode = beq;
            itmp.rs.id = stoi(words[1] + 1);
            itmp.rt.id = stoi(words[2] + 1);
            itmp.rt.value = atof(words[3]);
        }
        else if (op == "Bne" || op == "bne" || op == "BNE")
        {
            itmp.opcode = bne;
            itmp.rs.id = stoi(words[1] + 1);
            itmp.rt.id = stoi(words[2] + 1);
            itmp.rt.value = atof(words[3]);
        }
        else if (op == "Nop" || op == "nop" || op == "NOP")
        {
            itmp.opcode = nop;
        }
        else
        {
            cout << "Warning: Unknown instruction: " << op << endl;
            continue;
        }

        // Store the temporary parsed instruction for later use
        instruction.push_back(itmp);
    }

    // Close input file
    input.close();

    // Print the parsed results from input.txt to the terminal
    //------------------------------------------------------------------------------------------
    std::cout << "\n-----------------------------------------------\n";
    std::cout << "Printing information parsed from input.txt...\n\n";
    std::cout << "Integer adder: " << num_addiRS << " RS, " << cycle_addi << " EX cycle(s), " << num_addi << " FU\n";
    std::cout << "FP adder: " << num_addfRS << " RS, " << cycle_addf << " EX cycle(s), " << num_addf << " FU\n";
    std::cout << "FP multiplier: " << num_mulfRS << " RS, " << cycle_mulf << " EX cycle(s), " << num_mulf << " FU\n";
    std::cout << "Load/store unit: " << num_memRS << " RS, " << cycle_mem_exe << " EX cycle(s), " << cycle_mem_mem << " MEM cycle(s), " << num_mem << " FU\n\n";
    std::cout << "ROB entries: " << num_ROB << "\n";
    std::cout << "CDB buffer entries: " << num_CDB_buf << "\n\n";

    // Print nonzero int registers
    if (intARFValues.size() != 0)
    {
        //std::cout << "Int Registers (nonzero):\n";
        for (int i = 0; i < intARFValues.size(); i++)
        {
            if (intARFValues[i] != 0)
            {
                std::cout << "R" << i << " = " << intARFValues[i] << "\n";
            }
        }
    }

    // Print nonzero FP registers
    if (floatARFValues.size() != 0)
    {
        //std::cout << "FP Registers (nonzero):\n";
        for (int i = 0; i < floatARFValues.size(); i++)
        {
            if (floatARFValues[i] != 0)
            {
                std::cout << "F" << i << " = " << floatARFValues[i] << "\n";
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
		    std::cout << "Instr #" << (i+1) << ": ";

            switch (it.opcode)
            {
				case add:
					std::cout << "Add R" << it.rd.id << ", R" << it.rs.id << ", R" << it.rt.id;
                    break;
                case addi:
                    std::cout << "Add R immediate" << it.rd.id << ", R" << it.rs.id << ", " << it.immediate;
                    break;
                case sub:
                    std::cout << "Sub R" << it.rd.id << ", R" << it.rs.id << ", R" << it.rt.id;
                    break;
                case addf:
                    std::cout << "Add.d F" << it.rd.id << ", F" << it.rs.id << ", F" << it.rt.id;
                    break;
                case subf:
                    std::cout << "Sub.d F" << it.rd.id << ", F" << it.rs.id << ", F" << it.rt.id;
                    break;
                case mulf:
                    std::cout << "Mult.d F" << it.rd.id << ", F" << it.rs.id << ", F" << it.rt.id;
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
                case nop:
                    std::cout << "NOP";
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

    std::cout << "-----------------------------------------------\n" << std::endl;
    //------------------------------------------------------------------------------------------
}

/*
// Function that can be used to validate that all inputs are being read from the file correctly
void InputParser::output()
{
    std::cout << "\n-----------------------------------------------\n";
    std::cout << "Printing outputs...\n\n";

    // Results table formatting
    std::cout << "\t\t\t\tISSUE\t\tEX\t\tMEM\t\tWB\t\tCOMMIT\n";

    // Display ISSUE/EX/MEM/WB/COMMIT cycle results
    if (!instruction.empty())
    {
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
                    std::cout << "Mult.d F" << it.rd.id << ", F" << it.rs.id << ", F" << it.rt.id;
                    break;
                case load:
                    std::cout << "Ld F" << it.rd.id << ", " << it.rt.value << "(R" << it.rs.id << ")\t";
                    break;
                case store:
                    std::cout << "Sd F" << it.rs.id << ", " << it.rt.value << "(R" << it.rd.id << ")\t";
                    break;
                case beq:
                    std::cout << "Beq R" << it.rs.id << ", R" << it.rt.id << ", " << it.rt.value;
                    break;
                case bne:
                    std::cout << "Bne R" << it.rs.id << ", R" << it.rt.id << ", " << it.rt.value;
                    break;
                default:
                    std::cout << "(Unknown opcode: " << it.name << ")\t";
                    break;
            }

            // TODO: 1, 2, 3, 4, and 5 are placeholders. These will eventually be ISSUE, EX, MEM, WB, and COMMIT
            // for the respective instruction that's being printed in each row.
            std::cout << "\t\t" << 1 << "\t\t" << 2 << "\t\t" << 3 << "\t\t" << 4 << "\t\t" << 5 << "\n";

        }
    }

    // Display the non-zero register and memory values

    // Print nonzero int registers
    std::cout << "\n";
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

    std::cout << "-----------------------------------------------\n" << std::endl;

}
*/