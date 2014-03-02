// Emil Hedemalm
// 2013-09-09

#ifndef FORCE_H
#define FORCE_H

#include "MathLib.h"

class Force {
public:
    Force();
    /// Allow subclassing.
    virtual ~Force();

    Vector3f amount;
    float lifeTime;
    /// St
    float duration;

protected:

};

#endif
