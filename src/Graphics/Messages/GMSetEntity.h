// Emil Hedemalm
// 2013-07-01

#include "GraphicsMessage.h"
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
	GMSetEntity(Entity * entity, int target, Model * model);
	GMSetEntity(Entity * entity, int target, String string);
	void Process();
private:
	String string;
	Entity * entity;
	int target;
	void * data;
	Model * model;
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