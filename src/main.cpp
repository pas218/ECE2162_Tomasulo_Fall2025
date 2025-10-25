#include <iostream>
#include <string>
#include "ARF.hpp"
#include "RAT.hpp"


int main()
{
	/*
	std::cout << "Hi!" << std::endl;
	ARF<int> myARF(10);;
	std::cout << "The value of ARF(0) " << myARF.getValue(0) << std::endl;
    myARF.changeValue(0, 5); 
	std::cout << "The new value of ARF(0) " << myARF.getValue(0) << std::endl;
	*/
	RAT<int> int_RAT;
	RAT<float> float_RAT;
	
	RAT_type float_RAT_instance;
	RAT_type int_RAT_instance;
	
	
	int_RAT_instance = int_RAT.getValue(3);
	std::cout << "The value of int_RAT(3): " << int_RAT_instance.locationType << std::to_string(int_RAT_instance.locationNumber) << std::endl;
	int_RAT.changeValue(3, 0, 6);
	int_RAT_instance = int_RAT.getValue(3);
	std::cout << "The new value of int_RAT(3): " << int_RAT_instance.locationType << int_RAT_instance.locationNumber << std::endl;
	
	float_RAT_instance = float_RAT.getValue(3);
	std::cout << "The value of float_RAT(3): " << float_RAT_instance.locationType << std::to_string(float_RAT_instance.locationNumber) << std::endl;
	float_RAT.changeValue(3, 0, 6);
	float_RAT_instance = float_RAT.getValue(3);
	std::cout << "The new value of float_RAT(3): " << float_RAT_instance.locationType << float_RAT_instance.locationNumber << std::endl;
	
	return 0;
}