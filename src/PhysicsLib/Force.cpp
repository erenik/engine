// Emil Hedemalm
// 2013-09-09
#include "Force.h"

Force::Force(){
    amount = Vector3f(1,0,0);
    lifeTime = 1.0f;
    duration = 0.0f;
}

Force::~Force(){
}
