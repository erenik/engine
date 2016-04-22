// Emil Hedemalm
// 2013-08-07

#ifndef AE_RECT_H
#define AE_RECT_H

#include "MathLib.h"

struct Rect 
{
    Rect();
    Rect(int x0, int y0, int x1, int y1);
	/// Creates a rect which is an intersection of this and the other rect. See: http://en.wikipedia.org/wiki/Intersection_%28set_theory%29
	Rect Intersection(const Rect & otherRect);
	/// Size in x and y.
	Vector2i Size();

	Vector2i mini, maxi;
};

#endif
