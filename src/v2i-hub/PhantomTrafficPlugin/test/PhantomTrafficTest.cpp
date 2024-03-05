//============================================================================
// Name        : PhantomTrafficTest.cpp
// Description : Example unit test code using a test fixture.
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

class VectorTest : public testing::Test
{
protected:
	// Set-up and clean-up work for the test is best done in the constructor/destructor
	// unless exceptions are thrown.

	VectorTest()
	{
		_vector.push_back(1);
		_vector.push_back(2);
	}

	virtual ~VectorTest()
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

TEST_F(VectorTest, TestElementZeroIsOne)
{
	EXPECT_EQ(1, _vector[0]);
}

TEST_F(VectorTest, TestElementOneIsTwo)
{
	ASSERT_EQ(2, _vector[1]);
}

TEST_F(VectorTest, TestSizeIsTwo)
{
	EXPECT_EQ((unsigned int)2, _vector.size());
}

TEST_F(VectorTest, BonusTest)
{
	// Floats and Doubles comparisons have special ASSERT and EXPECT tests.
	ASSERT_FLOAT_EQ (2.00001, 2.000011);
	ASSERT_NEAR (2.00001, 2.000011, 0.00001);
	//ASSERT_NEAR (2.00001, 2.000011, 0.000001); // Fails.
}

}  // namespace
