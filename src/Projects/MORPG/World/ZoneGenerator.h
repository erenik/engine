/// Emil Hedemalm
/// 2015-01-20
/// Generator for zone contents and its internal parts.

#include "Zone.h"

class ZoneGenerator 
{
public:
	/// Generates contents for target zone. Neighbours are and type are taken into consideration to some extent?
	void GenerateZone(Zone * forZone);
private:

	void CreateGrid();
	void PlaceEntrances();
	void AddMainRooms();
	/// 1 big fat room, dynamically generated, all of it, probably.
	void AddMaximizedRoom();

	void ConnectRooms();
	void PlaceBuildingSlots();



	/// Active zone being generated, one at a time.
	Zone * zone;
};
