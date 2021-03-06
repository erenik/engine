// Emil Hedemalm
// 2013-07-28

#include "AEMath.h"
#include "Trigonometry.h"
#include <cmath>
#include <cstdlib>
#include "Globals.h"
#include "List/List.h"


/// Returns the angle in radians, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
float GetAngler(float x, float y)
{
	if (x == 0 && y == 0)
		return 0;
	/// First get raw degrees.
	float radians = acos(x);
	if (y < 0)
		radians *= -1.0f;
	return radians;
}
/// Returns the angle in degrees, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
float GetAngled(float x, float y)
{
	/// First get raw degrees.
	float radians = GetAngler(x,y);
	float degrees = radians * 360/(2* PI);
	return degrees;
}

double * arr360 = NULL;

double SinSampled360(double d)
{
	int degree = (int)(RADIANS_TO_DEGREES(d));
	while (degree < 0)
		degree += 360;
	degree %= 360;
	return arr360[degree];
}
double CosSampled360(double d)
{
	int degree = (int)(RADIANS_TO_DEGREES(d));
	degree += 90;
	while (degree < 0)
		degree += 360;
	degree %= 360;
	return arr360[degree];
}

void InitSampled360()
{
	arr360 = new double[360];
	for (int i = 0; i < 360; ++i)
	{
		arr360[i] = sin((double)i);
	}
}
void DeallocSampled360()
{
	SAFE_DELETE_ARR(arr360);
}


void TestSinCos()
{
	List<float> cosValues, sinValues;
	int tests = 20000;
	cosValues.Allocate(tests);
	sinValues.Allocate(tests);
	/// Detect resolution of native.
	for (int i = 0; i < tests; ++i)
	{
		cosValues.Add(cos((i / (float)tests) * (2 * PI)));
		sinValues.Add(sin((i / (float)tests) * (2 * PI)));
	}
	std::cout<<"\nTests per 2PI: "<<tests;
	int duplicates = cosValues.Duplicates();
	std::cout<<"\nDuplicates: "<<duplicates;
	std::cout<<"\nSome samples: ";
#define PRINT_SAMPLES(x) \
	for (int i = x; i < x + amount; ++i) \
		std::cout<<"\nSample "<<i<<": "<<cosValues[i]; 

	int amount = 10;
	PRINT_SAMPLES(0);
	PRINT_SAMPLES(tests - amount);
	// Close to X = 0
	PRINT_SAMPLES(tests / 4.f - amount);
	PRINT_SAMPLES(tests / 3.f);
	PRINT_SAMPLES(tests / 2.f);
	PRINT_SAMPLES(tests * 2 / 3.f);

}


bool TrigonometryTests()
{
	return false;
	List<float> values;
	int tests = 20000;
	values.Allocate(tests);
	/// Detect resolution of native.
	for (int i = 0; i < tests; ++i)
	{
		values.Add(asin((i / (float)tests) * (1.f)));
	}
	std::cout<<"\nTests per [0,1]: "<<tests;
	int duplicates = values.Duplicates();
	std::cout<<"\nDuplicates: "<<duplicates;
	std::cout<<"\nSome samples: ";
#define PRINT_SAMPLES(x) \
	for (int i = x; i < x + amount; ++i) \
		std::cout<<"\nSample "<<i<<": "<<values[i]; 

	int amount = 10;
	PRINT_SAMPLES(0);
	PRINT_SAMPLES(tests - amount);
	// Close to X = 0
	PRINT_SAMPLES(tests / 4.f - amount);
	PRINT_SAMPLES(tests / 3.f);
	PRINT_SAMPLES(tests / 2.f);
	PRINT_SAMPLES(tests * 2 / 3.f);

	return true;
}

