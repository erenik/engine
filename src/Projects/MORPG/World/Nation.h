/// Emil Hedemalm
/// 2014-07-27
/// A nation in the world. May or may not be officiall recognized by the other nations.

#ifndef NATION_H
#define NATION_H

class Settlement;

/// Describes the relationship between one nation and another. 
struct InternationalRelationship 
{
	enum
	{
		NEUTRAL,
		FRIENDLY,
		ALLIED,
		AT_WAR,
	};
	int state;
	Nation * otherNation;
};

class Nation 
{
public:
	/// Official name.
	String name;
	/// Unique ID for this nation. Used during save/load if not the name is used..?
	int id;

	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);


	/// Within the nation.
	List<Settlement*> settlements;
	/// Diplomatic relationships..
	List<InternationalRelationship*> internationalRelationships;

	

};

#endif


