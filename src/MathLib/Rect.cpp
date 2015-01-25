// Emil Hedemalm
// 2013-08-07

#include "Rect.h"

Rect::Rect(){
	min = Vector2i(0,0);
	max = Vector2i(1,1);
}

Rect::Rect(int x0, int y0, int x1, int y1)
: min(Vector2i(x0,y0)), max(Vector2i(x1,y1))
{

}

Rect Rect::Intersection(const Rect & otherRect)
{
	Rect iRect(min[0] > otherRect.min[0] ? min[0] : otherRect.min[0],
		min.y > otherRect.min.y ? min.y : otherRect.min.y,
		max.x < otherRect.max.x ? max.x : otherRect.max.x,
		max.y < otherRect.max.y ? max.y : otherRect.max.y);
	return iRect;
}

/// Size in x and y.
Vector2i Rect::Size()
{
	return max - min;// Vector2i(max.x - min.x, max.y - min.y);
}
