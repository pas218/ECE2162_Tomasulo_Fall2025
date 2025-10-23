// input_parser.h
#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

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



// Input parser class that fills other globals
class InputParser
{
public:
    static void parse(const std::string &filename);
};



typedef std::pair <int,float> mem_unit;
enum Operation
{
	addi,
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


struct InstBuf
{
    int free;
    std::string inst;
    InstBuf(): free(1), inst() {}
};


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


class ReOrderBuf_entry
{
public:
	int id;				//reorder buffer id is 201 ~ 400, 0 for available entry.
	int dst_id;			// destination register id.
	float value;
	int cmt_flag;		// 0 for not commit, 1 for commit;	
 
	ReOrderBuf_entry():id(0),dst_id(0),value(0.0),cmt_flag(0){}
};

class ReOrderBuf
{
public:
	ReOrderBuf_entry *table;
	int head;
	int tail;
	int n;
	int size;

	int get_size();
	bool empty();
	bool full();
	
	ReOrderBuf(int);
	~ReOrderBuf(){delete[]table;}
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

class ADDIER
{
public:
	RS_entry line;
	bool empty;
	int cycle;
	ADDIER();
};

class ADDFER
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
	ADDFER();
};

class MULFER
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
	MULFER();
};
#endif