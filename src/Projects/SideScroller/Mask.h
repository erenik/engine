/// Emil Hedemalm
/// 2015-04-29
/// Mask

#ifndef MASK_H
#define MASK_H

#include "Entity/Entity.h"

class Mask 
{
public:
	Mask();
	Mask(String name, String textureSource, int price, int speedBonus = 0, Vector2i jumpBonus = Vector2i());
	
	void Nullify();

	static Mask * GetByName(String name);
	/// Loads 'em.
	static bool LoadFromCSV(String fileName);

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

#endif
