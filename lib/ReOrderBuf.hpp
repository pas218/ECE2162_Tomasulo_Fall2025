#include <string>
#ifndef ROB_H
#define ROB_H
#include <cstdint>
#include <iostream>

struct InstBuf
{
    int free;
    std::string inst;
    InstBuf(): free(1), inst() {}
};

class ReOrderBuf_entry
{
public:
	int id;				//reorder buffer id is 0 or 1. 0 indicates integer, 1 indicates float. -1 indicates it is free.
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
	
	// Returns the ROB address of the dependency. If no dependency, return 1;
	// If looking for an Fp dependency, depType = 1. If looking for an integer dependency, depType  0;
	// regID should be the register number.
	int findDependency(int depType, int regID);
	// If no full, return -1. If not full, return the location of the free spot.
	int freeSpot();
	int get_size();
	bool empty();
	bool full();
	
	ReOrderBuf(int n);
	~ReOrderBuf(){delete[]table;}
};

#endif