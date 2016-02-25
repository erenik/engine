/// Emil Hedemalm & Karl Wängberg
/// 2016-02-25
/// Movement pattern class batching up movement, rotation, etc. for easing level generation.

#include "MovementPattern.h"
#include "File/File.h"
#include "String/StringUtil.h"

List<MovementPattern> MovementPattern::movementPatterns;

void MovementPattern::Load()
{
	List<String> lines = File::GetLines("Ship Data/Alien/Movement pattern.csv");
	char delimiter = FindCSVDelimiter(lines[0]);
	List<String> columns = TokenizeCSV(lines[0], delimiter);
	
	// Den kommer göra det mesta
	for(int i = 1; i<lines.Size(); i++)
	{

		List<String> tokens = TokenizeCSV(lines[i], delimiter);
		MovementPattern mp;
		for( int o = 0; o<tokens.Size(); o++)
		{
			String column = columns[o];
			if(column == "Name")
			{
				mp.name = tokens[o];
			}
			if(column == "ID")
			{
				mp.ID = tokens[o].ParseInt();
			}
			if(column == "Movement")
			{
				mp.movements = Movement::ParseFrom(tokens[o]);
			}
			if(column == "Rotation")
			{
				mp.rotations = Rotation::ParseFrom(tokens[o]);
			}
			if (column == "Spawning Offset")
				mp.spawnOffset.ParseFrom(tokens[o]);
		} 
		MovementPattern::movementPatterns.AddItem(mp);
	}
}