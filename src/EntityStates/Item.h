/// Emil Hedemalm
/// 2013-02-08

#ifndef ITEM_H
#define ITEM_H

namespace ItemType{
enum ItemTypes{
	NULL_TYPE,
	FOOD,
	DRINK,
	TOOL,
	VEHICLE,
	ITEM_TYPES,
};};

struct Item {
	/// Default constructor
	Item(int type);
	/// Sale-price
	int price;
	/// String-literal
	char name[240];
	char type;
};

struct Food : public Item {
	Food(const char * name, int price, float hungerReplenishedPerSec);
	float hungerReplenishedPerSecond;
};

struct Drink : public Item {
	Drink(const char * name, int price, float thirstReplenishedPerSec);
	float thirstReplenishedPerSecond;
};

struct Tool : public Item{
	Tool(const char * name, int price, float workBonus);
	float workBonus;
};

struct Vehicle : public Item{
	Vehicle(const char * name, int price, float movementSpeedBonus);
	float movementSpeedBonus;
};

#endif