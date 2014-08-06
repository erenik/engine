// Emil Hedemalm
// 2013-09-05
// An extension to the ray class in order to render them nicely!

#ifndef RENDER_RAY_H
#define RENDER_RAY_H

#include "PhysicsLib/Shapes/Ray.h"

class RenderRay : public Ray {
public:
    RenderRay(){
        Initialize();
        direction = Vector3f(0,0,-1.f);
    }
    RenderRay(const Ray & base){
        Initialize();
        start = base.start;
        direction = base.direction;
    }
    void Initialize(){
        lifeTime = 5.0f;
        duration = 0.0f;
    };
    float lifeTime; /// Max lifetime
    float duration; /// Current duration
private:
};

#endif
