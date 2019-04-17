//============================================================================
// Name        : SharedCodeTest.cpp
// Description : Example unit test code that uses shared code from the src directory.
//============================================================================

#include <gtest/gtest.h>

// Use a header that comes from the main "src" directory (not the "test" directory).
// Support for includes from the "src" directory were added in "CMakeLists.txt".
#include "SampleData.h"

using namespace std;

namespace unit_test {

/**
 * Nothing is setup/torn down in this test fixture.
 * For a more detailed example test, see ExampleTest.cpp.
 */
class SharedCodeTest : public testing::Test
{
protected:
	SharedCodeTest() { }

	virtual ~SharedCodeTest() { }
};

TEST_F(SharedCodeTest, TestSampleDataClass)
{
	// Use a class that comes from the main "src" directory (not the "test" directory).
	// Support for *.cpp files from the "src" directory were added in "CMakeLists.txt".
	ExamplePlugin::SampleData data;

	EXPECT_EQ(456, data.Value);
}

}  // namespace
