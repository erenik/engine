// Emil Hedemalm
// 2013-07-18

#ifndef GRAPHICS_MESSAGE_H
#define GRAPHICS_MESSAGE_H

#include "Entity/Entities.h"
#include "../Lighting.h"

class Triangle;
class Mesh;
class UserInterface;
class Viewport;
class Texture;
class Model;
class Ray;
class Renderable;
class AppWindow;


/// Default message class
class GraphicsMessage {
public:
	GraphicsMessage(int type);
	/** Explicitly declared constructor to avoid memory leaks.
		No explicit constructor may skip subclassed variable deallocation!
	*/
	virtual ~GraphicsMessage();
	virtual void Process(GraphicsState* graphicsState);
	int GetType() const { return type; };

	/// The retry flag. Default false. If true, message is assumed to have failed processing and will be requeued using retryTimeout to be attempted again at a later time.
	bool retry;
	int maxRetryAttempts; // Default 3.
	/// If the processing is to be attempted again, this is the timeout when re-queueing the message for processing it again. Default 1 second.
	AETime retryTimeout;
	/// Actual time the message will be processed when re-queued.
	AETime timeToProcess;

	/// Default 3. Adjust if you are using it much for UI updates.
	static int defaultMaxRetryAttempts;
protected:
	/// Type of message, if relevant
	int type;
};

/// For translated key-codes of alphabetical nature.
class GMChar : public GraphicsMessage 
{
public:
	GMChar(AppWindow * window, char c);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	AppWindow * window;
	char c;
};
/// For keys of system nature (enter, backspace, arrow-keys, etc.)
class GMKey : public GraphicsMessage 
{
public:
	GMKey(AppWindow * window, int keyCode, bool down, bool downBefore);
	static GMKey * Down(AppWindow * window, int keyCode, bool downBefore);
	static GMKey * Up(AppWindow * window, int keyCode);
	virtual void Process(GraphicsState* graphicsState) override;
private:
	bool down;
	bool up;
	bool downBefore;
	int keyCode;
	AppWindow * window;
};

/// Query start to record video.
class GMRecordVideo : public GraphicsMessage
{
public:
	GMRecordVideo(AppWindow * fromWindow);
	virtual void Process(GraphicsState * graphicsState) override;
private:
	AppWindow * window;
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
    void Process(GraphicsState* graphicsState) override;
private:
    int type;
    void * renderObject;
};

/// Resizes screen, updating relevant matrices
class GMResize : public GraphicsMessage {
public:
	GMResize(AppWindow * window, short width, short height);
	void Process(GraphicsState* graphicsState) override;
private:
	AppWindow * window;
	short width;
	short height;
};

class GMBufferMesh : public GraphicsMessage {
public:
	GMBufferMesh(Mesh * mesh);
	void Process(GraphicsState* graphicsState) override;
private:
	Mesh * mesh;
};

class GMBufferTexture : public GraphicsMessage {
public:
	GMBufferTexture(int textureID);
	GMBufferTexture(Texture * t);
	void Process(GraphicsState* graphicsState) override;
private:
	int textureID;
	Texture * t;
};

#include "UI/UIElement.h"
class UIElement;

class GMBufferUI : public GraphicsMessage {
public:
	GMBufferUI(UIElement * element);
	void Process(GraphicsState* graphicsState) override;
private:
	UIElement * element;
};


#include "../Entity/Entity.h"

class GMRegisterEntity : public GraphicsMessage {
public:
	GMRegisterEntity(Entity* entity);
	void Process(GraphicsState* graphicsState) override;
private:
	Entity* entity;
};

class GMRegisterEntities : public GraphicsMessage {
public:
	GMRegisterEntities(Entities selection);
	void Process(GraphicsState* graphicsState) override;
private:
	Entities selection;
};

class GMUnregisterEntity : public GraphicsMessage {
public:
	GMUnregisterEntity(Entity* entity);
	void Process(GraphicsState* graphicsState) override;
private:
	Entity* entity;
};

class GMUnregisterEntities : public GraphicsMessage {
public:
	GMUnregisterEntities(List< Entity* > entities);
	void Process(GraphicsState* graphicsState) override;
private:
	List< Entity* > entities;
};

class ParticleSystem;

class GMRegister : public GraphicsMessage {
public:
	GMRegister(List<ParticleSystem*> particleSystems);
	void Process(GraphicsState* graphicsState) override;
private:
	int target;
	List<ParticleSystem*> particleSystems;
};

class GMClear : public GraphicsMessage {
public:
	GMClear(int target);
	void Process(GraphicsState* graphicsState) override;
private:
	int target;
};

class GMSetLighting : public GraphicsMessage {
public:
	/// Sets copy of the given lighting setup.
	GMSetLighting(Lighting & lighting);
	/// Sets copy of the given lighting setup. Should be removed since pointers imply setting a newly allocated object.
	GMSetLighting(Lighting * lighting);
	~GMSetLighting();
	void Process(GraphicsState* graphicsState) override;
private:
	Lighting lighting;
	Lighting * lightingPtr;
};

/// ALWAYS send at least 1 Viewport.
class GMSetViewports : public GraphicsMessage {
public:
	/// ALWAYS send at least 1 Viewport.
	GMSetViewports(List<Viewport *> viewports, AppWindow * inWindow);
	~GMSetViewports();
	void Process(GraphicsState* graphicsState) override;
private:
	List<Viewport*> viewports;
	AppWindow * window;
};

class GMDeleteVBOs : public GraphicsMessage {
public:
	GMDeleteVBOs(UserInterface * ui);
	void Process(GraphicsState* graphicsState) override;
private:
	UserInterface * ui;
};

// Unbuffers and deletes (i.e. all data) related to a UserInterface-object, including the object itself!
class GMDelete : public GraphicsMessage {
public:
	GMDelete(UserInterface * ui);
	void Process(GraphicsState* graphicsState) override;
private:
	UserInterface * ui;
};

class GMRecompileShaders : public GraphicsMessage {
public:
	GMRecompileShaders();
	void Process(GraphicsState* graphicsState) override;
};


#endif
