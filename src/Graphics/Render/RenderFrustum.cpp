// Emil Hedemalm
// 2013-09-05

#include "RenderFrustum.h"
#include "Graphics/GraphicsManager.h"

RenderFrustum::RenderFrustum(Frustum frustum)
: Renderable(), frustum(frustum)
{
    lifeTime = 5.0f;
    duration = 0.0f;
    color = Vector4f(0,1,1,1);
}

RenderFrustum::~RenderFrustum(){
}

void RenderFrustum::Render(){
    glColor4f(color.x, color.y, color.z, 1.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#define RENDER(vert) glVertex3f(vert.x,vert.y,vert.z);

    glBegin(GL_LINE_STRIP);
        RENDER(frustum.hitherTopLeft);
        RENDER(frustum.hitherTopRight);
        RENDER(frustum.hitherBottomRight);
        RENDER(frustum.hitherBottomLeft);
        RENDER(frustum.hitherTopLeft);
        RENDER(frustum.fartherTopLeft);
        RENDER(frustum.fartherTopRight);
        RENDER(frustum.hitherTopRight);
        RENDER(frustum.hitherBottomRight);
        RENDER(frustum.fartherBottomRight);
        RENDER(frustum.fartherBottomLeft);
        RENDER(frustum.hitherBottomLeft);
    glEnd();
};
