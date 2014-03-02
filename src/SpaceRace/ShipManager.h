// Emil Hedemalm
// 2013-07-11
#ifndef SHIP_MANAGER_H
#define SHIP_MANAGER_H

#include "Ship.h"

#define ShipMan (*ShipManager::Instance())

class ShipManager {
	ShipManager();
	~ShipManager();
	static ShipManager * shipManager;
public:
	static bool IsAllocated();
	static void Allocate();
	static void Deallocate();
	static ShipManager * Instance();
	Ship * GetShip(String byName);
	Ship * GetShipBySource(String source);
	List<String> GetShipNames();
	List<Ship*> GetShips() { return ships; };

	Ship * CreateShipType(String shipTypeName);

	bool LoadFromDirectory(String dir);

	Ship * LoadShip(String fromSource);
	bool SaveShip(Ship * ship, String toPath);	
private:
	List<Ship*> ships;
};

#endif
