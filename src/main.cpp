#include <iostream>
#include <string>
#include "ARF.hpp"
#include "RAT.hpp"
#include "RS.hpp"


int main()
{
	/*
	std::cout << "Hi!" << std::endl;
	ARF<int> myARF(10);;
	std::cout << "The value of ARF(0) " << myARF.getValue(0) << std::endl;
    myARF.changeValue(0, 5); 
	std::cout << "The new value of ARF(0) " << myARF.getValue(0) << std::endl;
	*/
	
	// Quick RAT Testing
	RAT<int> int_RAT;
	RAT<float> float_RAT;
	
	RAT_type float_RAT_instance;
	RAT_type int_RAT_instance;
	
	
	int_RAT_instance = int_RAT.getValue(3);
	std::cout << "The value of int_RAT(3): " << int_RAT_instance.locationType << std::to_string(int_RAT_instance.locationNumber) << std::endl;
	int_RAT.changeValue(3, 0, 6);
	int_RAT_instance = int_RAT.getValue(3);
	std::cout << "The new value of int_RAT(3): " << int_RAT_instance.locationType << int_RAT_instance.locationNumber << std::endl;
	
	std::cout << std::endl;
	
	float_RAT_instance = float_RAT.getValue(3);
	std::cout << "The value of float_RAT(3): " << float_RAT_instance.locationType << std::to_string(float_RAT_instance.locationNumber) << std::endl;
	float_RAT.changeValue(3, 0, 6);
	float_RAT_instance = float_RAT.getValue(3);
	std::cout << "The new value of float_RAT(3): " << float_RAT_instance.locationType << float_RAT_instance.locationNumber << std::endl;
	
	std::cout << std::endl;
	
	/*
	
	template <typename T, typename Op>
	struct RS_type{
		Op operation;
		int robLocation;       // Valid value for location is >0. If this is -1, then that means there is no entry.
		int robDependency0;    // Valid value for dependency is >0. If this is -1, then that means there is no dependency.
		int robDependency1;
		T value0;
		T value1;
	};
*/
	
	// Quick RS Testing.
	RS<int, Ops> integerRS;
	RS_type<int, Ops>* intRSStation = integerRS.getValue(3);
	std::cout << "The initial value of integer RS (3):\n";
	std::cout << "Operation: " << std::to_string(intRSStation->operation) << std::endl;
	std::cout << "RobLocation: " << std::to_string(intRSStation->robLocation) << std::endl;
	std::cout << "RobDependency0: " << std::to_string(intRSStation->robDependency0) << std::endl;
	std::cout << "RobDependency1: " << std::to_string(intRSStation->robDependency1) << std::endl;
	std::cout << "Value0: " << std::to_string(intRSStation->value0) << std::endl;
	std::cout << "Value1: " << std::to_string(intRSStation->value1) << std::endl;
	
	std::cout << std::endl;
	// Change RS values.
	
	integerRS.changeROBLocation(3, 5);
	integerRS.changeOperation(3, ADD);
	integerRS.changeROBDependency(3, 4, 3);
	integerRS.changeRSVal0(3, 18);
	integerRS.changeRSVal1(3, 72);
	intRSStation = integerRS.getValue(3);
	std::cout << "The changed value of integer RS (3):\n";
	std::cout << "Operation: " << std::to_string(intRSStation->operation) << std::endl;
	std::cout << "RobLocation: " << std::to_string(intRSStation->robLocation) << std::endl;
	std::cout << "RobDependency0: " << std::to_string(intRSStation->robDependency0) << std::endl;
	std::cout << "RobDependency1: " << std::to_string(intRSStation->robDependency1) << std::endl;
	std::cout << "Value0: " << std::to_string(intRSStation->value0) << std::endl;
	std::cout << "Value1: " << std::to_string(intRSStation->value1) << std::endl;
	
	std::cout << std::endl;
	
	integerRS.clearLocation(3);
	intRSStation = integerRS.getValue(3);
	std::cout << "The cleared value of integer RS (3):\n";
	std::cout << "Operation: " << std::to_string(intRSStation->operation) << std::endl;
	std::cout << "RobLocation: " << std::to_string(intRSStation->robLocation) << std::endl;
	std::cout << "RobDependency0: " << std::to_string(intRSStation->robDependency0) << std::endl;
	std::cout << "RobDependency1: " << std::to_string(intRSStation->robDependency1) << std::endl;
	std::cout << "Value0: " << std::to_string(intRSStation->value0) << std::endl;
	std::cout << "Value1: " << std::to_string(intRSStation->value1) << std::endl;
	return 0;
}