// Emil Hedemalm
// 2013-09-05

#ifndef RENDER_FRUSTUM_H
#define RENDER_FRUSTUM_H

#include "Renderable.h"
#include "PhysicsLib/Shapes/Frustum.h"

class RenderFrustum : public Renderable {
public:
    RenderFrustum(const Frustum & frustum);
    virtual ~RenderFrustum();
    virtual void Render();
private:
    Frustum frustum;
};

#endif
