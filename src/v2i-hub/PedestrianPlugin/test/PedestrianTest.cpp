//============================================================================
// Name        : PedestrianTest.cpp
// Description : PedestrianTest unit test code using a test fixture.
//============================================================================

#include <vector>
#include <gtest/gtest.h>

using namespace std;
namespace unit_test {

// For a detailed tutorial of test fixtures, see official documentation:
//
//   Google Test Primer: https://github.com/google/googletest/blob/master/googletest/docs/Primer.md
//
// Also note that the mock framework can also be used if needed.  It is not used in this example.
//
//   Google Mock Docs: https://github.com/google/googletest/blob/master/googlemock/README.md

class PedestrianTest : public testing::Test
{
protected:
	// Set-up and clean-up work for the test is best done in the constructor/destructor
	// unless exceptions are thrown.

	PedestrianTest()
	{
	}

	virtual ~PedestrianTest()
	{
	}

	// SetUp is called immediately after the constructor (right before each test).
	// TearDown is called immediately after each test (right before the destructor).
	// Exceptions can be thrown in these methods.

	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
	}

	// Objects declared here can be used by the tests cases for this fixture.

	vector<int> _vector;
};

// A test case will continue running on failure when the "EXPECT" macros are used.
// A test case will abort on failure when the "ASSERT" macros are used.
TEST_F(PedestrianTest, CarInGeofenceTest)
{
	// Floats and Doubles comparisons have special ASSERT and EXPECT tests.

	EXPECT_TRUE(true);
}

}  // namespace
