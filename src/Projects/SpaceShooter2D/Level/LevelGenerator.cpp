//Skapad 22-02-2016 kl 10:33 på morgonen. Det var en måndag.
#include "Level.h"
#include "../SpaceShooter2D.h"
#include "SpawnGroup.h"

int difficulty;
AETime levelDuration(TimeType::MILLISECONDS_NO_CALENDER);


void GenerateLevel (String arguments)
{
	List<String> args = arguments.Tokenize(" ,");
	
	if(args.Size() > 1)
	{
		String timeString = args[1];
		String diffString = args[2];
		levelDuration.ParseFrom(timeString);
		difficulty = diffString.ParseInt();
	}
	
	std::cout<<"\nYo what up in da hooooood. Level generator on da waaaaay!!";
	std::cout<<"\nYou have given "<<levelDuration.Seconds()<<" as time total, and "<<difficulty<<" as difficulty! Woopdeedoo!";
	
	Level & level = spaceShooter -> level;
	level.Clear();
	
	//Filter wanted ships, difficulty and alien type mainlyyyyyy
	List<String> relevantShips;
	for(int q = 0; q<Ship::types.Size(); q++)
	{
		Ship * shipType = Ship::types[q];
		
		//Catch unwanted bosses and skip them
		if(shipType->boss) 
			continue;
		//Criteria, difficulty lator!
		if(shipType->difficulty > difficulty)
			continue;
		relevantShips.AddItem(shipType -> name);
	}
	

	Random selector;

	//Create spawns in order
	AETime spawnTime(TimeType::MILLISECONDS_NO_CALENDER);
	while(spawnTime.Seconds() <= levelDuration.Seconds())
	{

		SpawnGroup * sg = new SpawnGroup();
		spawnTime.AddMs(3000);
		sg->spawnTime = spawnTime;
		sg->shipType = relevantShips[selector.Randi(relevantShips.Size())];
		level.spawnGroups.AddItem(sg);
		sg->formation = Formation::LINE_Y;
		sg->number = selector.Randi(9)+1;
		sg->size = Vector2f(5, 5);

	}


};