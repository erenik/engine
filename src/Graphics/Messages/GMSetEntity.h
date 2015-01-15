// Emil Hedemalm
// 2013-07-01

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"

struct AnimationSet;
class EstimatorFloat;

/** For setting textures, for applicable texture-targets, see Shader.h or the following
	DIFFUSE_MAP		0x0000001
	SPECULAR_MAP	0x0000002
	NORMAL_MAP		0x0000004
*/
class GMSetEntityTexture : public GraphicsMessage {
public:
	// Defualt, sets both diffuse and specular map.
	GMSetEntityTexture(Entity * entity, Texture * texture);
	GMSetEntityTexture(Entity * entity, int target, Texture * texture);
	GMSetEntityTexture(Entity * entity, int target, String textureSource);
	void Process();
private:
	Entity * entity;
	Texture * t;
	String textureSource;
	int target;
};

class GMSetEntity : public GraphicsMessage {
public:
	/// For general procedures that do stuff..
	GMSetEntity(Entity * entity, int target);
	GMSetEntity(List<Entity*> entities, int target, Entity * otherEntity);
	GMSetEntity(List<Entity*> entities, int target, Camera * camera);
	GMSetEntity(Entity * entity, int target, Model * model);
	GMSetEntity(List<Entity*> entities, int target, String string);
	void Process();
private:
	Entity * otherEntity;
	String string;
	List<Entity*> entities;
	int target;
	void * data;
	Model * model;
	Camera * camera;
};

class GMSetEntityb : public GraphicsMessage 
{
public:
	GMSetEntityb(List<Entity*> entities, int target, bool value, bool recursive = false);
	virtual void Process();
private:
	List<Entity*> entities;
	bool bValue;
	bool recurse;
	int target;
};

// For setting strings
class GMSetEntitys : public GraphicsMessage 
{
public:
	GMSetEntitys(Entity * entity, int target, String value);
	virtual void Process();
private:
	Entity * entity;
	String sValue;
	int target;
};

class GMSetEntityf : public GraphicsMessage 
{
public:
	GMSetEntityf(List<Entity*> entities, int target, float value);
	virtual void Process();
private:
	List<Entity*> entities;
	float fValue;
	int target;
};

class GMSetEntityi : public GraphicsMessage 
{
public:
	GMSetEntityi(List<Entity*> entities, int target, int value);
	virtual void Process();
private:
	List<Entity*> entities;
	int iValue;
	int target;
};

class GMSetEntityVec4f : public GraphicsMessage 
{
public:
	GMSetEntityVec4f(List<Entity*> entities, int target, Vector4f value);
	virtual void Process();
private:
	List<Entity*> entities;
	Vector4f vec4fValue;
	int target;
};


class GMSlideEntityf : public GraphicsMessage 
{
public:
	GMSlideEntityf(Entities entities, int target, EstimatorFloat * usingPrefilledEstimator);
	GMSlideEntityf(Entities entities, int target, float targetValue, int timeInMs);
	virtual void Process();
private:
	Entities entities;
	EstimatorFloat * estimatorFloat;
	float targetValue;
	int target;
	int timeInMs;
};

class GMClearEstimators : public GraphicsMessage 
{
public:
	GMClearEstimators(Entities entities);
	virtual void Process();
private:
	Entities entities;
};

