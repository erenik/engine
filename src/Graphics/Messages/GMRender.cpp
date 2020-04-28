// Emil Hedemalm
// 2013-09-05
// Message to queue simple shapes to be rendered more or less temporarily.

#include "Graphics/GraphicsManager.h"
#include "GraphicsMessage.h"
#include "GraphicsMessages.h"
#include "Graphics/Render/RenderRay.h"
#include "Graphics/Render/Renderable.h"

enum {
    NULL_TARGET,
	FRAME,
    RAY,
    TRIANGLE,
    RENDERABLE,
};


/// Query to render if renderOnQuery is set to true.
GMRender::GMRender()
: GraphicsMessage(GM_RENDER)
{
	type = FRAME;
}

GMRender::GMRender(Triangle & tri, RenderOptions * ro/* = NULL*/)
: GraphicsMessage(GM_RENDER)
{
    type = TRIANGLE;
    RenderTriangle * rt = new RenderTriangle(tri);
    if (ro)
        rt->ro = *ro;
    renderObject = (void*)rt;
}

GMRender::GMRender(Ray & ray, float time)
: GraphicsMessage(GM_RENDER)
{
    type = RAY;
    RenderRay * rr = new RenderRay(ray);
    rr->lifeTime = time;
    renderObject = (void*)rr;
};


GMRender::GMRender(Renderable * renderable)
: GraphicsMessage(GM_RENDER){
    type = RENDERABLE;
    renderObject = (void*) renderable;
}

void GMRender::Process(GraphicsState* graphicsState){
    switch(type){
		case FRAME: {
			Graphics.renderQueried = true;
			break;
		}
        case TRIANGLE: {
            Graphics.renderShapes.Add((Renderable*) (RenderTriangle*)renderObject);
            break;
        }
        case RAY: {
            RenderRay * rr = (RenderRay*) renderObject;
            Graphics.rays.Add(rr);
            break;
        }
        case RENDERABLE:
            Graphics.renderShapes.Add((Renderable*) renderObject);
            break;
        default:
            assert(false && "Bad type in GMRender::Process");
    }
}

