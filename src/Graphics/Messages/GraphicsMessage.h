// Emil Hedemalm
// 2013-07-18

#ifndef GRAPHICS_MESSAGE_H
#define GRAPHICS_MESSAGE_H

#include "../Selection.h"
#include "../Lighting.h"

class Triangle;
class Mesh;
class UserInterface;
class Viewport;
class Texture;
class Model;
class Ray;
class Renderable;
class Window;


/// Default message class
class GraphicsMessage {
public:
	GraphicsMessage(int type);
	/** Explicitly declared constructor to avoid memory leaks.
		No explicit constructor may skip subclassed variable deallocation!
	*/
	virtual ~GraphicsMessage();
	virtual void Process();
	int GetType() const { return type; };
protected:
	/// Type of message, if relevant
	int type;
};

/// Query start to record video.
class GMRecordVideo : public GraphicsMessage
{
public:
	GMRecordVideo(Window * fromWindow);
	virtual void Process();
private:
	Window * window;
};

struct RenderOptions;

/// Message to queue simple shapes to be rendered more or less temporarily.
class GMRender : public GraphicsMessage {
public:
	/// Query to render if renderOnQuery is set to true.
	GMRender();
    GMRender(Triangle & tri, RenderOptions * ro = NULL);
    GMRender(Ray & ray, float time);
    GMRender(Renderable * renderable);
    void Process();
private:
    int type;
    void * renderObject;
};

/// Resizes screen, updating relevant matrices
class GMResize : public GraphicsMessage {
public:
	GMResize(Window * window, short width, short height);
	void Process();
private:
	Window * window;
	short width;
	short height;
};

class GMBufferMesh : public GraphicsMessage {
public:
	GMBufferMesh(Mesh * mesh);
	void Process();
private:
	Mesh * mesh;
};

class GMBufferTexture : public GraphicsMessage {
public:
	GMBufferTexture(int textureID);
	GMBufferTexture(Texture * t);
	void Process();
private:
	int textureID;
	Texture * t;
};

#include "UI/UIElement.h"
class UIElement;

class GMBufferUI : public GraphicsMessage {
public:
	GMBufferUI(UIElement * element);
	void Process();
private:
	UIElement * element;
};


#include "../Entity/Entity.h"

class GMRegisterEntity : public GraphicsMessage {
public:
	GMRegisterEntity(Entity * entity);
	void Process();
private:
	Entity * entity;
};

class GMRegisterEntities : public GraphicsMessage {
public:
	GMRegisterEntities(Selection selection);
	void Process();
private:
	Selection selection;
};

class GMUnregisterEntity : public GraphicsMessage {
public:
	GMUnregisterEntity(Entity * entity);
	void Process();
private:
	Entity * entity;
};

class GMUnregisterEntities : public GraphicsMessage {
public:
	GMUnregisterEntities(List<Entity*> entities);
	void Process();
private:
	List<Entity*> entities;
};

class ParticleSystem;

class GMRegister : public GraphicsMessage {
public:
	GMRegister(List<ParticleSystem*> particleSystems);
	void Process();
private:
	int target;
	List<ParticleSystem*> particleSystems;
};

class GMClear : public GraphicsMessage {
public:
	GMClear(int target);
	void Process();
private:
	int target;
};

class GMSetLighting : public GraphicsMessage {
public:
	GMSetLighting(Lighting * lighting);
	~GMSetLighting();
	void Process();
private:
	Lighting * lighting;
};

/// ALWAYS send at least 1 Viewport.
class GMSetViewports : public GraphicsMessage {
public:
	/// ALWAYS send at least 1 Viewport.
	GMSetViewports(List<Viewport *> viewports, Window * inWindow);
	~GMSetViewports();
	void Process();
private:
	List<Viewport*> viewports;
	Window * window;
};

class GMDeleteVBOs : public GraphicsMessage {
public:
	GMDeleteVBOs(UserInterface * ui);
	void Process();
private:
	UserInterface * ui;
};

// Unbuffers and deletes (i.e. all data) related to a UserInterface-object, including the object itself!
class GMDelete : public GraphicsMessage {
public:
	GMDelete(UserInterface * ui);
	void Process();
private:
	UserInterface * ui;
};

class GMRecompileShaders : public GraphicsMessage {
public:
	GMRecompileShaders();
	void Process();
};


#endif
