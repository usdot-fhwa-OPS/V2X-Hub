//============================================================================
// Name        : PreemptionTest.cpp
// Description : Preemption unit test code using a test fixture.
//============================================================================

#include <vector>
#include <gtest/gtest.h>

using namespace std;
namespace unit_test {

class PreemptionTest : public testing::Test
{
protected:
	// Set-up and clean-up work for the test is best done in the constructor/destructor
	// unless exceptions are thrown.

	PreemptionTest()
	{
	}

	virtual ~PreemptionTest()
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

bool CarInGeofence(double x, double y, double geox[], double geoy[], int GeoCorners) {
        int   i, j=GeoCorners-1 ;
        bool  oddNodes      ;

        for (i=0; i<GeoCorners; i++) {
            if ((geoy[i]< y && geoy[j]>=y
            ||   geoy[j]< y && geoy[i]>=y)
            &&  (geox[i]<=x || geox[j]<=x)) {
            oddNodes^=(geox[i]+(y-geoy[i])/(geoy[j]-geoy[i])*(geox[j]-geox[i])<x); }
            j=i; }

        return oddNodes; 
    } 

TEST_F(PreemptionTest, CarInGeofenceTest)
{
	// Floats and Doubles comparisons have special ASSERT and EXPECT tests.

	double geox [4] = { 0,4,4,0 }; 
	double geoy [4] = { 0,0,4,4 }; 

	EXPECT_TRUE(CarInGeofence(2, 2, geox, geoy, 4));
}

}  // namespace
