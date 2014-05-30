// Emil Hedemalm
// 2013-09-05
// For rendering all queued simple objects.

#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"
#include "RenderRay.h"
#include "Graphics/Camera/Camera.h"
#include "Renderable.h"

void GraphicsManager::RenderShapes(){
    glBegin(GL_LINES);
    for (int i = 0; i < rays.Size(); ++i){
        RenderRay * rr = rays[i];
        if (rr->duration >= rr->lifeTime){
            rays.Remove(rr);
            delete rr;
            --i;
            continue;
        }
        rr->duration += graphicsState.frameTime;
        glColor4f(1.0f, 1.0f, 1.0f, 1 - rr->duration / rr->lifeTime);
        Vector3f start = rr->start, end = rr->start + rr->direction * 100000000.0f;
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
    }
	glEnd();

    // Render all queued shapes!
    for (int i = 0; i < renderShapes; ++i){
        Renderable * r = renderShapes[i];
        r->Render();
    }


    Camera * cam = graphicsState.camera;
    assert(cam);
    if (!cam)
        return;
	/*
    Frustum frustum = cam->GetFrustum();
    Vector3f topLeft = frustum.hitherTopLeft;
    Vector3f topLeft2 = frustum.fartherTopLeft * 0.9f;
    Vector3f topRight = frustum.hitherTopRight;
    Vector3f topRight2 = frustum.fartherTopRight * 0.9f;
    Vector3f bottomLeft = frustum.hitherBottomLeft;
    Vector3f bottomLeft2 = frustum.fartherBottomLeft * 0.9f;
 //   std::cout<<"\nTopLeft2: "<<topLeft2<<" topRight2: "<<topRight2;
    glBegin(GL_LINES);
        glColor4f(0,0,1,1);
        glVertex3f(topLeft.x, topLeft.y, topLeft.z);
        glColor4f(0,1,0,1);
        glVertex3f(topLeft2.x, topLeft2.y, topLeft2.z);
        glColor4f(0,0,1,1);
        glVertex3f(topRight.x, topRight.y, topRight.z);
        glColor4f(0,1,0,1);
        glVertex3f(topRight2.x, topRight2.y, topRight2.z);
        glColor4f(1,1,0,1);
        glVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);
        glVertex3f(bottomLeft2.x, bottomLeft2.y, bottomLeft2.z);


        glColor4f(1,0,1,1);
        glVertex3f(topRight2.x, topRight2.y, topRight2.z);
        glVertex3f(bottomLeft2.x, bottomLeft2.y, bottomLeft2.z);
	glEnd();
	*/
}
