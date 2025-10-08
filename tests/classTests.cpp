#include "ARF.h"

namespace classTests
{
	TEST_CLASS(classTests)
	{
		public:
		
			TEST_METHOD(test_ARF)
			{
				ARF registers<int>(10);
				Assert::AreEqual(registers.getValue(0), 0);
			}
	};
}