#include <string>
#ifndef ROB_H
#define ROB_H
struct InstBuf
{
    int free;
    std::string inst;
    InstBuf(): free(1), inst() {}
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

#endif