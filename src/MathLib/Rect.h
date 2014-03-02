// Emil Hedemalm
// 2013-08-07

#ifndef AE_RECT_H
#define AE_RECT_H

struct Rect {
    Rect();
    Rect(int x0, int y0, int x1, int y1);
    int x0, x1;
    int y0, y1;
};

#endif
