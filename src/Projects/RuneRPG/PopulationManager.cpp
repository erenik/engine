/// Emil Hedemalm 
/// 2014-06-14
/** Population used for generating random fights within the vicinity. 
	Usually linked to a specific zone.
	Each population must have a unique name, which may be auto-generated if needed.
	The same population may be used in multiple zones and time periods as long as you keep track of its ID or name.
*/


#include "PopulationManager.h"
#include "File/File.h"
#include "File/FileUtil.h"
#include "Random/Random.h"

Population::Population()
{
	distanceDecay = 0.f;
	decayExponent = 1.f;
}

Population::~Population(){}

float Population::GetEncounterRatio(Vector3i atPosition)
{
	// First check if the position is wihin the population boundary.
	bool inside = false;
	float distance;
	if (entireZone)
		inside = true;
	else 
	{
		Vector3f toPlayer = origin - atPosition;
		distance = toPlayer.Length();
		if (distance < radius)
			inside = true;
	}
	if (!inside)
		return 0;

	// Inside, check probability.
	float area = radius * radius * PI;
	// Density per tile.
	density = amount / area;
	// Multiply by aggresiveness
	float encounterRatio = density * aggresiveness;

	// Calculate distance decay multiplier.
	ClampFloat(distanceDecay, -1.f, 1.f);

	/// Value between 0 and 1.
	float relativeDistance = distance / radius;
	float distanceToCenter = relativeDistance;
	float distanceToEdge = 1.f - distanceToCenter;
	float closenessToCenter = 1.f - relativeDistance;
	float closenessToEdge = relativeDistance;

	float centerFactor = 1.f + distanceDecay;
	float edgeFactor = 1.f - distanceDecay;

	float centerPart = closenessToCenter * centerFactor;
	float edgePart = closenessToEdge * edgeFactor;

	float distanceDecayMultiplier = centerPart + edgePart;
	distanceDecayMultiplier = pow(distanceDecayMultiplier, decayExponent);
	encounterRatio *= distanceDecayMultiplier;

	return encounterRatio;
}


bool Population::LoadFrom(String file)
{
	source = file;
	List<String> lines = File::GetLines(file);
	bool specifyingBattles = false;
	if (!lines.Size())
		return false;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		List<String> tokens = line.Tokenize(" \t");
		/// At least one word on the line.
		if (!tokens.Size())
			continue;
		String key = tokens[0];
		/// Comments
		if (key.StartsWith("//"))
			continue;
		/// Setting future input mode.
		if (key == "battles")
		{
			specifyingBattles = true;
			continue;
		}
		/// For when specifying battles, add contents to the list directly.
		else if (specifyingBattles)
		{
			this->battles.Add(key);
			continue;
		}


		/// Lines with at least 1 argument below
		if (tokens.Size() < 2)
			continue;
		String value = tokens[1];
		if (key == "entireZone")
		{
			this->entireZone = value.ParseBool();
		}
		else if (key == "radius")
		{
			this->radius = value.ParseFloat();
		}
		else if (key == "distanceDecay")
			this->distanceDecay = value.ParseFloat();
		else if (key == "decayExponent")
			this->decayExponent = value.ParseFloat();
		else if (key == "aggresiveness")
			this->aggresiveness = value.ParseFloat();
		else if (key == "initialAmount")
			this->initialAmount = amount = value.ParseFloat();
		else if (key == "maxPopulation")
			maxPopulation = value.ParseFloat();
		else if (key == "growth")
			growth = value.ParseFloat();
		
		/// Lines with at least 2 arguments to the key
		if (tokens <= 3)
			continue;
		String value2 = tokens[2];
		if (key == "origin")
		{
			origin = Vector3f(value.ParseFloat(), value.ParseFloat(), 0);
		}
	}
	return true;
}

String Population::ShouldFight(Vector3f playerPosition)
{
	float encounterRatio = GetEncounterRatio(playerPosition);
	if (encounterRatio <= 0)
		return String();

	/// Randomizer!
	static Random encounterRand;

	// Random from 0 to 1
	float random = encounterRand.Randf();

	/// Optionally scale the random to match the max/min encounter ratio, depending on some options?
	if (random > encounterRatio)
		return String();

	// Ookay, encounter is warranted. Randomize battle from available ones.	
	random = encounterRand.Randf();
	// Scale it.
	random *= battles.Size();
	// Round it
	int battleIndex = floor(random + 0.5f);
	battleIndex = battleIndex % battles.Size();
	return battles[battleIndex];
}

// Singleton shit
PopulationManager * PopulationManager::populationManager = NULL;

void PopulationManager::Allocate()
{
	assert(!populationManager);
	populationManager = new PopulationManager();
}
void PopulationManager::Deallocate()
{
	assert(populationManager);
	delete populationManager;
	populationManager = NULL;
}
PopulationManager * PopulationManager::Instance()
{
	assert(populationManager);
	return populationManager;
}

/// Constructors
PopulationManager::PopulationManager()
{
}
PopulationManager::~PopulationManager()
{
	populations.ClearAndDelete();
}

void PopulationManager::ReloadPopulations()
{
	for (int i = 0; i < populations.Size(); ++i)
	{
		Population * pop = populations[i];
		if (pop->active)
		{
			pop->LoadFrom(pop->source);
		}
	}
}

List<Population*> PopulationManager::ActivePopulations()
{
	List<Population*> activePops;
	for (int i = 0; i < populations.Size(); ++i)
	{
		Population * pop = populations[i];
		if (pop->active)
			activePops.Add(pop);
	}
	return activePops;
}


List<Population*> PopulationManager::LoadPopulations(String fromDir)
{	
	List<Population*> createdOrPreexistingPops;
	List<String> files;
	int result = GetFilesInDirectory(fromDir, files);
	if (!result)
	{
		std::cout<<"\nUnable to find any populations in directory: "<<fromDir;
		return createdOrPreexistingPops;
	}
	for (int i = 0; i < files.Size(); ++i)
	{
		String file = files[i];		
		String name = file.Tokenize(".")[0];
		
		Population * preexisting = Get(name);
		if (preexisting)
		{
			createdOrPreexistingPops.Add(preexisting);
			continue;
		}
		Population * pop = new Population();
		pop->name = name;
		bool ok = pop->LoadFrom(fromDir+"/"+file);
		if (ok)
		{
			populations.Add(pop);
			createdOrPreexistingPops.Add(pop);
		}
		else 
			delete pop;
	}
	return createdOrPreexistingPops;
}

void PopulationManager::MakeActive(List<Population*> pops)
{
	List<Population*> activePops = ActivePopulations();
	for (int i = 0; i < activePops.Size(); ++i)
	{
		Population * pop = activePops[i];
		pop->active = false;
	}
	for (int i = 0; i < pops.Size(); ++i)
	{
		Population * pop = pops[i];
		pop->active = true;
	}
}

/// Fetches by name
Population * PopulationManager::Get(String byName)
{
	for (int i = 0; i < populations.Size(); ++i)
	{
		Population * pop = populations[i];
		if (pop->name == byName)
			return pop;
	}
	return NULL;
}
//	static Population * Get(int byID);
/// Parses initial state. Should be called but once onry?
Population * PopulationManager::NewPopulation(String fromFile)
{
	return NULL;
}



