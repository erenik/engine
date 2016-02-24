//Skapad 22-02-2016 kl 10:33 på morgonen. Det var en måndag.
#include "Level.h"
#include "../SpaceShooter2D.h"
#include "SpawnGroup.h"

int difficulty = 1;
AETime levelDuration(TimeType::MILLISECONDS_NO_CALENDER);


void GenerateLevel (String arguments)
{
	Level & level = spaceShooter -> level;
	level.Clear();
	
	List<String> args = arguments.Tokenize(" ,");
	/// Reset level-time?
	levelTime = AETime(TimeType::MILLISECONDS_NO_CALENDER, 0);
	
	if(args.Size() > 1)
	{
		String timeString = args[1];
		String diffString = args[2];
		levelDuration.ParseFrom(timeString);
		difficulty = diffString.ParseInt();
	}
	if (levelDuration.Seconds() == 0)
		levelDuration.AddMs(1000 * 30);
	
	std::cout<<"\nYo what up in da hooooood. Level generator on da waaaaay!!";
	std::cout<<"\nYou have given "<<levelDuration.Seconds()<<" as time total, and "<<difficulty<<" as difficulty! Woopdeedoo!";
	
	
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
	
	CreateFolder("GeneratedLevels");

	File::ClearFile("Generatedlevel.srl");
	// Create spawns in order
	AETime spawnTime(TimeType::MILLISECONDS_NO_CALENDER);
	while(spawnTime.Seconds() <= levelDuration.Seconds())
	{
		
		SpawnGroup * sg = new SpawnGroup();
		// Cooldown between spawns
		spawnTime.AddMs(3000);
		// When it spawns
		sg->spawnTime = spawnTime;
		// Pick a ship
		sg->shipType = relevantShips[selector.Randi(relevantShips.Size())];
		level.spawnGroups.AddItem(sg);
		// Pick a formation
		sg->formation = Formation::LINE_Y;
		// Pick a number of ships
		sg->number = selector.Randi(9)+1;
		// Pick a formation size
		sg->size = Vector2f(5, 5);
		/// Default spawn on right side of field.
		sg->position = Vector2f(playingFieldHalfSize.x+5.f, 0);
		
		/// Time between spawns for moar flexibility
		sg->spawnIntervalMsBetweenEachShipInFormation = 200;

		String str = sg->GetLevelCreationString(sg->spawnTime);
		File::AppendToFile("Generatedlevel.srl", str);
		
	}
	AETime time = AETime::Now(); 
	String timestr = time.ToString("Y-M-D-H-m");
	String contents = File::GetContents("Generatedlevel.srl");
	contents.PrintData();
	String outputFile = "./GeneratedLevels/Generatedlevel"+timestr+".srl";
	File::AppendToFile(outputFile, contents);

	/// Set current level's source to be the generated file's, so that it is reloaded upon death automatically.
	level.source = outputFile;

};
