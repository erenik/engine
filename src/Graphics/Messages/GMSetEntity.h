// Emil Hedemalm
// 2013-07-01

#include "GraphicsMessage.h"
#include "GraphicsMessages.h"
struct AnimationSet;

/** For setting textures, for applicable texture-targets, see Shader.h or the following
	DIFFUSE_MAP		0x0000001
	SPECULAR_MAP	0x0000002
	NORMAL_MAP		0x0000004
*/
class GMSetEntityTexture : public GraphicsMessage {
public:
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
	GMSetEntity(List<Entity*> entities, int target, Camera * camera);
	GMSetEntity(Entity * entity, int target, Model * model);
	GMSetEntity(List<Entity*> entities, int target, String string);
	void Process();
private:
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
	GMSetEntityb(Entity * entity, int target, bool value);
	virtual void Process();
private:
	Entity * entity;
	bool bValue;
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
	GMSetEntityf(Entity * entity, int target, float value);
	virtual void Process();
private:
	Entity * entity;
	float fValue;
	int target;
};

class GMSetEntityVec4f : public GraphicsMessage 
{
public:
	GMSetEntityVec4f(Entity * entity, int target, Vector4f value);
	virtual void Process();
private:
	Entity * entity;
	Vector4f vec4fValue;
	int target;
};




