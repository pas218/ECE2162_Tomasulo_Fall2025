// input_parser.h
#ifndef INPUT_PARSER_V2_H
#define INPUT_PARSER_V2_H

#include <string>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <vector>



constexpr int MAX_BUFFER = 65536;
static constexpr char DELIM0[] = " \t";
static constexpr char DELIM1[] = " =,[]";
static constexpr char DELIM2[] = ", \t()";
static constexpr char DELIM3[] = " ,\t()";

typedef std::pair <int,float> mem_unit;

// 
// 0-1 integer adder
// 2-3 floating point adder
// 4 floating point multiplier
// 5-6 memory
//  7-8 branch
enum Operation
{
	addi = 0,
	subi,
	addf,
	subf,
	mulf,
	load,
	store,
	beq,
	bne
};

class item
{
public:
	float value;
	int id;
	bool imme_flag;		//0 for register, 1 for immediate
	bool ready;			//0 for not ready, 1 for ready
	item();
};

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

// Input parser class that fills other globals
class InputParser
{
private:

	
public:

	// extern definitions
	std::vector<mem_unit> memory;
	std::vector<inst> instruction;
	
	std::vector<int> intARFValues;
	std::vector<float> floatARFValues;

	int num_ROB;
	int num_CDB_buf;

	int num_addiRS;
	int num_addfRS;
	int num_mulfRS;
	int num_memRS;

	int cycle_addi;
	int cycle_addf;
	int cycle_mulf;
	int cycle_mem_exe;
	int cycle_mem_mem;

	int num_addi;
	int num_addf;
	int num_mulf;
	int num_mem;
	
	InputParser(int numARF);
    void parse(const std::string &filename);
	//void output();
};






/*
template<typename T>
class ARF
{
public:
	int id;
	bool ready;
	T value;
	ARF():id(0),ready(0){};
};

class IntARF
{
public:
	ARF<int> *table;
	int pointer;
	IntARF(int);
	~IntARF(){delete[]table;}
};

class FpARF
{
public:
	ARF<float> *table;
	int pointer;
	FpARF(int);
	~FpARF(){delete[]table;}
};
*/
/*
class RAT
{
public:
	int alias;
	float value;
	RAT():value(0.0){};
};

class IntRAT
{
public:
	RAT *table;
	int pointer;
	IntRAT(int);
	~IntRAT(){delete[]table;}
};

class FpRAT
{
public:
	RAT *table;
	int pointer;
	FpRAT(int);
	~FpRAT(){delete[]table;}
};


/*
void Addi();
void Addf();
void Mulf();
void Store();
void Load();
void Beq();

void initial();
void issue();
void execution();
void mem();
void writeback();
void commit();
*/

class RS_entry
{
public:
	item d_r;
	item s_r;
	item t_r;
	int time;
	int icount;
	bool empty;
	RS_entry():time(0),icount(0),empty(1){};
};

/*
class RS
{
public:
	RS_entry *table;
	int head;
	int tail;
	int n;
	int size;
	
	int get_size();
	bool empty();
	bool full();
	RS(int );
	~RS(){delete[]table;}
};
*/
class AddIUnit
{
public:
	RS_entry line;
	bool empty;
	int cycle;
	AddIUnit();
};

class AddFUnit
{
public:
	RS_entry *line;
	int head;
	int tail;
	int cycle;
	int n;
	int size;

	int get_size();
	bool empty();
	bool full();
	AddFUnit();
};

class MulFUnit
{
public:
	RS_entry *line;
	int head;
	int tail;
	int cycle;
	int n;
	int size;

	int get_size();
	bool empty();
	bool full();
	MulFUnit();
};

#endif