/// Emil Hedemalm
/// 2015-04-20
/// Player property.

#ifndef LUCHADOR_H
#define LUCHADOR_H

#include "Entity/EntityProperty.h"

class Mask 
{
public:
	Mask()
		: name ("No name"), price(10000)
	{
		Nullify();
	}
	Mask(String name, String textureSource, int price, int speedBonus = 0, Vector2i jumpBonus = Vector2i())
		: name(name), textureSource(textureSource), price(price), speedBonus(speedBonus), jumpBonus(jumpBonus)
	{
		Nullify();
	}
	void Nullify()
	{
		purchased = false;	
	}
	String name;
	String textureSource;
	int price;
	bool purchased;

	// A bonus of sorts to passive running speed acceleration.
	int speedBonus;
	// A bonus in X- and Y-velocity on jumps?
	Vector2i jumpBonus;
//	int additionalJumps;
};
extern List<Mask> masks;

class Entity;
struct Collision;
class Message;
class CollisionCallback;

class LuchadorProperty : public EntityProperty 
{
public:
	LuchadorProperty(Entity * owner);
	virtual void OnCollision(Collision & data);	
	virtual void Process(int timeInMs);
	virtual void ProcessMessage(Message * message);
	virtual void OnCollisionCallback(CollisionCallback * cc);
protected:
	bool sleeping;
	Time lastJump;
};

#endif
