
#include "BattleState.h"

BattleState::BattleState(){
	state = BATTLE_NOT_STARTED;
};

BattleState::~BattleState(){
	for (int i = 0; i < sides.Size(); ++i){
		delete sides[i];
	}
	sides.Clear();
}