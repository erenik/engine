// Emil Hedemalm
// 2013-09-05
// Base class for renderable items.

#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "MathLib.h"
#include "PhysicsLib/Shapes.h"

struct RenderOptions {
    RenderOptions();
    bool disableDepthTest;
    float duration;
    Vector4f color;
};

struct GraphicsState;

class Renderable {
public:
    Renderable();
    virtual ~Renderable();
    virtual void Render(GraphicsState & graphicsState);

    float lifeTime;
    Vector4f color;
    RenderOptions ro;
protected:
    float duration;
private:
};

#define IMPLEMENTS_RENDERABLE  public: void Render(GraphicsState & graphicsState);

class RenderTriangle : public Renderable {
    IMPLEMENTS_RENDERABLE
public:
    RenderTriangle(Triangle tri);
private:
    Triangle tri;
};

#endif

