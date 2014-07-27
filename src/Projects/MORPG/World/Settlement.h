/// Emil Hedemalm
/// 2014-07-27
/// A settlement in the world. Probably controlled or owned by a nation.

#ifndef SETTLEMENT_H
#define SETTLEMENT_H

/// Relationship between a settlement and nations (both the owner/controller and others)
struct SettlementAllegience 
{
	/// 0 to 1. 0 being fervent hate and 1 being loving loyality.
	float state;
	Nation * nation;
};

#include "Zone.h"

class Settlement
{
public:
	String name;
	int id;

	virtual bool WriteTo(std::fstream & file);
	virtual bool ReadFrom(std::fstream & file);

	/// o-o
	int population;

	/// List of zones which are related to or part of this settlement.
	List<Zone*> zones;

	/// Residents living here.
	List<Character*> residents;
	/// Residents overall view of the neighbouring nations.
	List<SettlementAllegience*> nationRelations;
};

#endif
