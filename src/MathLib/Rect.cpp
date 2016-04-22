// Emil Hedemalm
// 2013-08-07

#include "Rect.h"

Rect::Rect(){
	mini = Vector2i(0,0);
	maxi = Vector2i(1,1);
}

Rect::Rect(int x0, int y0, int x1, int y1)
: mini(Vector2i(x0,y0)), maxi(Vector2i(x1,y1))
{

}

Rect Rect::Intersection(const Rect & otherRect)
{
	Rect iRect(mini[0] > otherRect.mini[0] ? mini[0] : otherRect.mini[0],
		mini.y > otherRect.mini.y ? mini.y : otherRect.mini.y,
		maxi.x < otherRect.maxi.x ? maxi.x : otherRect.maxi.x,
		maxi.y < otherRect.maxi.y ? maxi.y : otherRect.maxi.y);
	return iRect;
}

/// Size in x and y.
Vector2i Rect::Size()
{
	return maxi - mini;// Vector2i(max.x - min.x, max.y - min.y);
}
