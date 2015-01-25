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
void Renderable::Render(){
    assert(false);
}



RenderTriangle::RenderTriangle(const Triangle & tri)
 : Renderable(), tri(tri)
{
}

void RenderTriangle::Render(){
    if (ro.disableDepthTest){
        glDisable(GL_DEPTH_TEST);
    }
    glColor4f(ro.color[0], ro.color[1], ro.color[2], ro.color[3]);
    glBegin(GL_TRIANGLES);
#define RENDER(p) glVertex3f(p[0],p[1],p[2]);
        RENDER(tri.point1);
        RENDER(tri.point2);
        RENDER(tri.point3);
    glEnd();
}
