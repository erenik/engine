/// Emil Hedemalm
/// 2014-06-13
/** Population used for generating random fights within the vicinity. 
	Usually linked to a specific zone.
	Each population must have a unique name, which may be auto-generated if needed.
	The same population may be used in multiple zones and time periods as long as you keep track of its ID or name.
*/

#include "String/AEString.h"
#include "MathLib.h"

class Population 
{
	Population();
	friend class PopulationManager;
	bool LoadFrom(String file);
	// File used when loading last time.
	String source;
public:
	~Population();	
	// Unique name (hopefully?)
	String name;
	// Unique ID
//	int id;
	/// Original map this population started in.
	String originZone;

	
	float GetEncounterRatio(Vector3i atPosition);

	/// If active in current map.
	bool active;

	/// If flagged, this population may generate its encounters in the entire zone.
	bool entireZone;

	/// Origin of the population. A square or circular shape is assumed to be used for each population when generating their random encounters when traversing the map.
	Vector3f origin;
	/// Radius of the area this population occupies.
	float radius;

	/// p=p
	void RecalculateDensity();
	/// Residents/people per tile (square meter for zoomed in maps, calculate in some other way if using for zoomed out maps?)
	float density;

	/// Defines how likely it is that an encounter is to happen. Not everyone wants to fight you, y'know?
	float aggresiveness;

	/** If a fight should be triggered?! Note that this function is intended to be called ONCE each time upon reaching a tile!
		Returns reference to battle or an empty string if no battle is to be initiated.
	*/
	String ShouldFight(Vector3f playerPosition);
	
	/// When entering it.
	int initialAmount;
	/// Current amount of inhabitants?
	int amount;
	/// If it grows, defines the maximum cap.
	int maxPopulation;
	/// Relative growth based on amount per.. month?
	float growth;

	/// 0 = equal distribution, 1.0 = Max at center, 0 at edge. 0.5 = 0.75 at center, 0.25 at edge
	float distanceDecay;
	float decayExponent;

	/// Battlers that live in this population. Can be used for generating hybrid encounters maybe?
	List<String> battlers;
	/// Common battles that might occur when in the vicinity of this population.
	List<String> battles;
};

#define PopMan (*PopulationManager::Instance())

class PopulationManager 
{
	PopulationManager();
	~PopulationManager();
	static PopulationManager * populationManager;
public:
	static void Allocate();
	static void Deallocate();
	static PopulationManager * Instance();

	void ReloadPopulations();

	List<Population*> ActivePopulations();
	List<Population*> LoadPopulations(String fromDir);
	void MakeActive(List<Population*> pops);
	/// Fetches by name
	Population * Get(String byName);

private:
	/// Creates population if needed?
	Population * GetPopulation(String fromFile);
	/// Parses initial state. Should be called but once onry?
	Population * NewPopulation(String fromFile);

	List<Population*> populations;
};









