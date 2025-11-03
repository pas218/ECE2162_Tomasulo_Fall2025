// RS.hpp
// This can be used for the following: Interger, FP Add, and FP mult.
#ifndef RS_H
#define RS_H
#include <unordered_set>

// Define enum for integet operations.
enum Ops
{
	EMPTY = 0,
	ADD = 1,
	SUB = 2,
	MULT = 3,
	DIV = 4
};


// Defining enum for adder operations.
enum addOps
{
	FP_ADD_EMPTY = 0,
	FP_ADD = 1,
	FP_SUB = 2
};

// Define enum for multiplier operations.
enum multOps
{
	FP_MULT_EMPTY = 0,
	FP_MULT = 3,
	FP_DIV = 4
};


template <typename T, typename Op>
struct RS_type{
	Op operation;
	int robLocation;       // Valid value for location is >0. If this is -1, then that means there is no entry.
	int robDependency0;    // Valid value for dependency is >0. If this is -1, then that means there is no dependency.
	int robDependency1;
	T value0;
	T value1;
	T computation;
	bool computationDone;
};


template <typename T, typename Op>
class RS
{
    private:
		int numStations;
		static std::unordered_set<int> takenRobSpots;
        RS_type<T, Op>  *stationsPtr;      // Our actual stations will be dynamically allocated to account for specified input.
		static_assert(std::is_same<T, int>::value || std::is_same<T, float>::value,
                "The RS class can only be instantied with either type int or float.");
				
		inline bool checkBounds(int stationNumber);
		//inline bool checkRobFree(int robLocation);
    public:
		RS();
        RS(int numLocations);
		~RS();
		
		bool replaceROBDependency(int robLocation, T value);
		
		// If not rob spot is used here, return -1.
		// Otherwise, return the location in the RS that correspondes with the input ROB location.
		int findRSFromROB(int robLocation);
		
		// Returns next free spot in RS.
		// If no free spot, then returns -1.
		int freeSpot();
		
		int getSize();
        // Change the value of a location. Returns 1 if successful, otherwise return 0.
		// The value -1 indicates an emplty spot.
        bool changeROBLocation(int stationNumber, int robNumber);
		
		// Change the value of an operation. Returns 1 if successful, otherwise return 0.
		// The value EMPTY indicates an emplty spot.
		bool changeOperation(int stationNumber, Op operation);
		
		// Change the value of a dependency. Returns 1 if successful, otherwise return 0.
		// The value -1 indicates an emplty spot.
		bool changeROBDependency(int stationNumber, int dep0, int dep1);
		bool changeROBDependency0(int stationNumber, int dep0);
		bool changeROBDependency1(int stationNumber, int dep1);
		
		// Change the value of an operation slot. Returns 1 if successful, otherwise return 0.
		bool changeRSVal0(int stationNumber, T val0);
		bool changeRSVal1(int stationNumber, T val1);
		bool compute(int stationNumber);
		// Clears the location of an RS station.
		// The location and dependencies entries will be changes to -1, and the values to 0 (either integer or FP).
		bool clearLocation(int stationNumber);
        RS_type<T, Op>* getValue(int stationNumber);
};

template <typename T, typename Op>
std::unordered_set<int> RS<T, Op>::takenRobSpots;

#include "RS.tpp"

#endif