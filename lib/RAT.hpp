// RAT.hpp
#ifndef RAT_H
#define RAT_H

struct RAT_type{
	char locationType;    // This should take on the following type: R (interger register), F (Floating-point register), and B (ROB location).
	int locationNumber;
};

template <typename T>
class RAT
{
    private:
		int numLocations;
        RAT_type  *locationsPtr;      // Our actual registers will be dynamically allocated to account for specified input.
		static_assert(std::is_same<T, int>::value || std::is_same<T, float>::value,
                "The RAT class can only be instantied with either type int or float.");
    public:
		RAT(); 
        RAT(int numLocations);
		// Copy constructor
		RAT(RAT<T>& input_class);
		~RAT();
		
		// Returns -1 if no location.
		int getNextARFLocation(int ROBSpot);
		int getSize();
        // Change the value of a location. Returns 1 if successful, otherwise return 0.
		// If you want to change it to an ARF locatioin, isARF should be 1.
		// If you want to change it to a ROB location, isARF should be 0 and make sure to enter the rob location.
        bool changeValue(int locationNumber, bool isARF, int robNumber);
		bool resetLocation(int locationNumber);
		std::vector<RAT_type> exportRAT();
		void recoverRAT(std::vector<RAT_type> &numbers);
        RAT_type* getValue(int locationNumber);
};


#include "RAT.tpp"

#endif