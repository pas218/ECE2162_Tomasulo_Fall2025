// input_parser.cpp
#include "input_parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>


using namespace std;

// extern definitions
extern std::vector<mem_unit> memory;
extern std::vector<inst> instruction;

extern int num_ROB;
int num_CDB = 0;

extern int num_addiRS;
extern int num_addfRS;
extern int num_mulfRS;
extern int num_memRS;

extern int cycle_addi;
extern int cycle_addf;
extern int cycle_mulf;
extern int cycle_mem_exe;
extern int cycle_mem_mem;

extern int num_addi;
extern int num_addf;
extern int num_mulf;
extern int num_mem;

extern Operation opcode;
extern item dst,src,tgt;
extern InstBuf ib;
extern IntARF *IntArf;
extern FpARF *FpArf;
extern IntRAT *IntRat;
extern FpRAT *FpRat;
extern ReOrderBuf *ROB;
extern RS *addiRS,*addfRS,*mulfRS,*memRS;
extern ADDIER *addier, *memer, *memer2;
extern ADDFER *addfer;
extern MULFER *mulfer;

extern int clk;		


item::item() : value(0.0f), id(0), imme_flag(false), ready(false) {}


IntARF::IntARF(int n)
{
    pointer = n;
    table = new ARF<int>[n];

    for (int i = 0; i < n; i++)
    {
        table[i].id = i;
        table[i].ready = true;
        table[i].value = 0;
    }
}

FpARF::FpARF(int n)
{
    pointer = n;
    table = new ARF<float>[n];

    for (int i = 0; i < n; i++)
    {
        table[i].id = i;
        table[i].ready = true;
        table[i].value = 0.0f;
    }
}

IntRAT::IntRAT(int n)
{
    pointer = n;
    table = new RAT[n];

    for (int i = 0; i < n; i++)
    {
        table[i].alias = 0;
        table[i].value = 0.0f;
    }
}

FpRAT::FpRAT(int n)
{
    pointer = n;
    table = new RAT[n];

    for (int i = 0; i < n; i++)
    {
        table[i].alias = 0;
        table[i].value = 0.0f;
    }
}

ReOrderBuf::ReOrderBuf(int n)
{
    size = n;
    n = 0;
    head = 0;
    tail = 0;
    table = new ReOrderBuf_entry[n];
}

RS::RS(int capacity)
{
    n = capacity;
    table = new RS_entry[capacity];
    head = 0;
    tail = 0;
    size = 0;
}

int RS::get_size() 
{
    return size;
}
bool RS::empty()
{
    return size == 0;
}
bool RS::full()
{
    return size == n;
}

ADDIER::ADDIER()
{
    empty = true;
    cycle = 0;
}

ADDFER::ADDFER()
{
    n = 1;
    line = new RS_entry[n];
    head = 0;
    tail = 0;
    size = 0;
    cycle = 0;
}

int ADDFER::get_size()
{
    return size;
}
bool ADDFER::empty()
{
    return size == 0;
}
bool ADDFER::full()
{
    return size == n;
}

MULFER::MULFER()
{
    n = 1;
    line = new RS_entry[n];
    head = 0;
    tail = 0;
    size = 0;
    cycle = 0;
}

int MULFER::get_size()
{
    return size;
}
bool MULFER::empty()
{
    return size == 0;
}
bool MULFER::full()
{
    return size == n;
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
	addiRS = new RS(num_addiRS);
	addfRS = new RS(num_addfRS);
	mulfRS = new RS(num_mulfRS);
	memRS = new RS(num_memRS);
	addier = new ADDIER[num_addi];
	memer= new ADDIER;
	memer2= new ADDIER;
	addfer = new ADDFER[num_addf];
	mulfer = new MULFER[num_mulf];



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
			num_CDB = atoi(words[3]);
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
                if (index < IntArf->pointer)
                {
                    if (k + 1 < max)
                    {
                        IntArf->table[index].value = atoi(words[k + 1]);
                        IntArf->table[index].ready = true;
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
                    if (index < FpArf->pointer) {
                        FpArf->table[index].value = value;
                        FpArf->table[index].ready = true;
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
        if (op == "Add")
        {
            itmp.opcode = addi;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Sub")
        {
            itmp.opcode = subi;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Add.d")
        {
            itmp.opcode = addf;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Sub.d")
        {
            itmp.opcode = subf;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Mul.d")
        {
            itmp.opcode = mulf;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rs.id = stoi(words[2] + 1);
            itmp.rt.id = stoi(words[3] + 1);
        }
        else if (op == "Ld")
        {
            itmp.opcode = load;
            itmp.rt.imme_flag = true;
            itmp.rd.id = stoi(words[1] + 1);
            itmp.rt.value = atof(words[2]);
            itmp.rs.id = stoi(words[3] + 1);
        }
        else if (op == "Sd")
        {
            itmp.opcode = store;
            itmp.rs.id = stoi(words[1] + 1);
            itmp.rt.value = atof(words[2]);
            itmp.rd.id = stoi(words[3] + 1);
            itmp.rt.imme_flag = true;
        }
        else if (op == "Beq")
        {
            itmp.opcode = beq;
            itmp.rs.id = stoi(words[1] + 1);
            itmp.rt.id = stoi(words[2] + 1);
            itmp.rt.value = atof(words[3]);
        }
        else if (op == "Bne")
        {
            itmp.opcode = bne;
            itmp.rs.id = stoi(words[1] + 1);
            itmp.rt.id = stoi(words[2] + 1);
            itmp.rt.value = atof(words[3]);
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
}






