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
