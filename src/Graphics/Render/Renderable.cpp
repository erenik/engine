// Emil Hedemalm
// 2013-09-05
// Base class for renderable items.

#include "Renderable.h"
#include <cassert>
#include "Graphics/OpenGL.h"

RenderOptions::RenderOptions(){
    disableDepthTest = false;
    duration = 5.0f;
    color = Vector4f(1.f,1.f,1.f,1.f);
}

Renderable::Renderable(){

}
Renderable::~Renderable(){

}
void Renderable::Render(GraphicsState & graphicsState){
    assert(false);
}



RenderTriangle::RenderTriangle(Triangle tri)
 : Renderable(), tri(tri)
{
}

void RenderTriangle::Render(GraphicsState & graphicsState){
    if (ro.disableDepthTest){
        glDisable(GL_DEPTH_TEST);
    }
    glColor4f(ro.color.x, ro.color.y, ro.color.z, ro.color.w);
    glBegin(GL_TRIANGLES);
#define RENDER(p) glVertex3f(p.x,p.y,p.z);
        RENDER(tri.point1);
        RENDER(tri.point2);
        RENDER(tri.point3);
    glEnd();
}
