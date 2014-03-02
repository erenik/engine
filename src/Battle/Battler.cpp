// Emil Hedemalm 
// 2013-07-09

// An RPG-based class/struct

#include "Battler.h"
#include "BattleAction.h"

Battler::Battler(){
	name = "Unnamed";
}
Battler::~Battler(){};
void Battler::Process(BattleState &battleState){
	std::cout<<"\nBattler::Process called. Forgot to overload it?";
}


/// Make actions available for this battler. Creates and links the necessary categories for managing them as needed.
void Battler::AddActions(List<BattleAction*> actionsToAdd)
{
	for (int i = 0; i < actions.Size(); ++i)
	{
		BattleAction * action = actionsToAdd[i];
		if (actions.Exists(action))
		{
			std::cout<<"\nAlready got target action, skipping.";
			continue;
		}
		
		if (action){
			this->actions.Add(action);		
			BattleActionCategory * category = GetCategory(action->categoryName);
			/// Create it if needed.
			if (!category){
				category = new BattleActionCategory();
				category->name = action->categoryName;
				actionCategories.Add(category);
			}
			category->Add(action);
		}
	}
}

BattleActionCategory * Battler::GetCategory(String byName)
{
	for (int i = 0; i < actionCategories.Size(); ++i){
		BattleActionCategory * bac = actionCategories[i];
		if (bac->name == byName)
			return bac;
	}
	return NULL;
}