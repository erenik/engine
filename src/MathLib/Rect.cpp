// Emil Hedemalm
// 2013-08-07

#include "Rect.h"

Rect::Rect(){
    x0 = y0 = 0;
    x1 = y1 = 1;
}

Rect::Rect(int x0, int y0, int x1, int y1)
: x0(x0), y0(y0), x1(x1), y1(y1)
{
}

Rect Rect::Intersection(const Rect & otherRect)
{
	Rect iRect(x0 > otherRect.x0 ? x0 : otherRect.x0,
		y0 > otherRect.y0 ? y0 : otherRect.y0,
		x1 < otherRect.x1 ? x1 : otherRect.x1,
		y1 < otherRect.y1 ? y1 : otherRect.y1);
	return iRect;
}

/// Size in x and y.
Vector2i Rect::Size()
{
	return Vector2i(x1 - x0, y1 - y0);
}
