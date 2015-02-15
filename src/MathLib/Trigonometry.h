// Emil Hedemalm
// 2013-07-28

#ifndef AE_TRIGONOMETRY_H
#define AE_TRIGONOMETRY_H

/** Returns the angle in radians, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
	Assumes normalized co-ordinates (-1 to +1).
*/
float GetAngler(float x, float y);
/** Returns the angle in degrees, given the coordinates in XY-space, relative to the unit-circle. (0 degrees being X+, increasing counter-clockwise).
	Assumes normalized co-ordinates (-1 to +1).
*/
float GetAngled(float x, float y);

#define FastSin sin
#define FastCos cos

double SinSampled360(double d);
double CosSampled360(double d);

void InitSampled360();
void DeallocSampled360();

#endif