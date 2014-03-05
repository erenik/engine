/// Emil Hedemalm
/// 2014-01-20
/// A structure for handling entity save/load/creation for the RuneRPG game.

#ifndef RUNE_ENTITY_H
#define RUNE_ENTITY_H

/// A structure for handling entity save/load/creation for the RuneRPG game. Subclass to add further adjustments/statistics. 
class RuneEntity {

private:
	/// List of all loaded/created types.
	List<RuneEntity*> entityTypes;
public:
	// Static functions handling all these entities!
	static RuneEntity * 
	static bool Save(String toFile);
	static bool Load(String fromFile);
#define RuneEntities RuneEntityTypes
	static List<RuneEntity> RuneEntityTypes();

	// Name of the entity, yow
	String name;
	// General type, can be Creature, Object, etc?
	String type;
	
	// General texture used to represent this entity. If no animation set is present, the texture will also be the primary rendering base.
	String texture;
	// Animation set to be used.
	String animSet;

	// Size on the grid.
	int sizeX, sizeY;
	// Centered, left, right, etc. Default is centered on X, bottom on Y.
	int alignment;
	
	// Further descriptions for this kind of entity. Can be anything extra?
	String description.

};


#endif