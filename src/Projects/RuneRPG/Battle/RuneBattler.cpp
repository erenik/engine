// Emil Hedemalm
// 2013-07-09

#include "RuneBattler.h"
#include <cmath>
#include <fstream>
#include <cstring>
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "RuneBattleAction.h"
#include "Battle/BattleManager.h"

RuneBattler::RuneBattler() : Battler(){
	Nullify();
}

RuneBattler::~RuneBattler(){
}
void RuneBattler::Nullify(){
	unconscious = false;
	delay = 5000 + rand()%10000;
	hp = maxHP = maxMP = mp = 10;
	weaponDamage = spellDamage = 1;
	magicArmor = magicPower = magicSkill = spellDamage = 0;
	initiative = 1000;
	state = WAITING_FOR_INITIATIVE;
}

/// from 0 to 4, 0 being player, 1-3 being enemies 1-3
RuneBattler::RuneBattler(int defaultTypes) : Battler(){
	Nullify();
	switch(defaultTypes){
		case 0:
			name = "Erenik";
			hp = 35;
			mp = 10;
			attack = 10;
			defense = 5;
			agility = 5;
			armor = 5;
			speed = 5;
			weaponDamage = 5;
			spellDamage = 10;
			break;
		case 1:
			name = "Moffsi";
			hp = 16;
			attack = 6;
			defense = 7;
			agility = 5;
			armor = 5;
			speed = 3;
			weaponDamage = 3;
			break;
		case 2:
			name = "Gnupp";
			hp = 35;
			attack = 6;
			defense = 7;
			agility = 5;
			armor = 5;
			speed = 4;
			weaponDamage = 6;
			break;
		case 3:
			name = "Raowrg";
			hp = 35;
			mp = 10;
			attack = 8;
			defense = 7;
			agility = 5;
			armor = 5;
			speed = 6;
			weaponDamage = 4;
			break;
		default:
			assert(false && "Bad type in RuneBattler(defaultTypes) constructor!");
			break;
	}
}

void RuneBattler::Process(BattleState &battleState){

	// Woo!
	if (state == INCAPACITATED)
		return;

    /// Check state, has it queued an action yet?
    if (state == WAITING_FOR_INITIATIVE){
        initiative -= battleState.timeDiff * speed;
        if (initiative <= 0){
			if (!isAI){
				MesMan.QueueMessage(new Message("BattlerReady"));
			}
            initiative = 0;
            state = IDLE;
        }
    }

    /// Process status-effects.

    /// Return if no AI.
    if (!isAI || state != IDLE)
        return;

	state = ACTION_QUEUED;
	/// Queue up an action, random ones work fine.
	BattleAction * ba = actions[rand()%actions.Size()];
	RuneBattleAction * rba = new RuneBattleAction(*ba);
	rba->subjects = this;
	RuneBattler * b = (RuneBattler*)BattleMan.RandomTarget(rba->targetFilter, this);
	if (b)
		rba->targets.Add(b);
	BattleMan.QueueAction(rba);

    /// Since no good AI atm, return.
    return;

	// Decrement delay
	delay -= battleState.timeDiff * speed;
	// Wait until delay is over.
	if (delay > 0)
		return;
	delay = 0;

	// Find a target, there is only one opposing side.
	Side * opposingSide = NULL;
	for (int i = 0; i < battleState.sides.Size(); ++i){
		opposingSide = battleState.sides[i];
		if (i != side)
			break;
	}
	// If not opposing side, flag the battle as finished.
	if (opposingSide == NULL){
		std::cout<<"\nNo valid opposing side! Side "<<side<<" won!";
		battleState.state = BATTLE_ENDED;
		return;
	}
	// Grab target (random)
	RuneBattler * target = (RuneBattler*)opposingSide->battlers[rand()%opposingSide->battlers.Size()];
	while (target->unconscious)
		target = (RuneBattler*)opposingSide->battlers[rand()%opposingSide->battlers.Size()];

	bool hit = false;
	bool lucky = false;
	int a = rand() % 1000;
	if (a < 25){
		hit = true;
		lucky = true;
	}
	a = rand() % 1000;
	if (a < 26){
		hit = false;
		lucky = true;
	}
	if (!hit && !lucky){
		float accuracy = 95 + agility - target->agility;
		a = rand()%100;
		if (a < accuracy)
			hit = true;
	}
	if (!hit){
		if (lucky)
			std::cout<<"\n"<<name<<" tries to attack "<<target->name<<" but completely misses!!";
		else
			std::cout<<"\n"<<name<<" tries to attack "<<target->name<<" but misses!";
	}
	else {
		// Do attack
		std::cout<<"\n"<<name<<" tries to attack "<<target->name<<" and hits him ";
		if (lucky)
			std::cout<<"gracefully ";
		float baseDamage = weaponDamage * pow(1.025f, attack);
		float attackDefRatio = pow(attack / target->defense, 1.5f);
		float armorRatio = pow(0.99f, target->armor);
		float physicalDamage = baseDamage *attackDefRatio * armorRatio;
		int damage = floor(physicalDamage+0.5f);
		std::cout<<"for "<<damage<<" points of damage!";
		target->Damage(damage);
		if (target->unconscious){
			// Check if all are unconscious
			bool stillAlive = false;
			std::cout<<"\n"<<target->name<<" is KO'd!";
			for (int i = 0 ; i < opposingSide->battlers.Size(); ++i)
				if (!((RuneBattler*)opposingSide->battlers[i])->unconscious)
					stillAlive = true;
			if (!stillAlive)
				battleState.state = BATTLE_ENDED;
		}
		else {
			std::cout<<"\n"<<target->name<<" hp down to "<<target->hp;
		}
	}
	// Three seconds default delay
	delay = 20000;
}

void RuneBattler::OnActionFinished(){
    state = WAITING_FOR_INITIATIVE;
}

/// Checks the initiative-parameter!
bool RuneBattler::IsIdle(){
    if (initiative <= 0)
        return true;
    return false;
}

/// Sets MP/HP to max, etc.
void RuneBattler::ResetStats(){
	hp = maxHP;
	mp = maxMP;
	;
}

/// Take damage o-o, returns true if it killed/incapacitated the battler.
bool RuneBattler::Damage(int dmg){
	hp -= dmg;
	if (hp <= 0){
		hp = 0;
		unconscious = true;
		state = INCAPACITATED;
		MesMan.QueueMessage(new Message("OnBattlerIncapacitated("+name+")"));
		return true;
	}
	return false;
}

/// See Dropbox/Emka Design/Karls/Rune RPG/Equations and testing for some formulae
bool RuneBattler::PhysicalDamage(int dmg){
	float armorReduction = pow(0.99f, armor);
	std::cout<<"\nPhysical damage reduced by armor from "<<dmg;
	dmg *= armorReduction;
	std::cout<<" to "<<dmg<<".";
	return Damage(dmg);
}

/// Perform magic-damage-reduction before applying it.
bool RuneBattler::MagicDamage(int dmg){
	float magicArmorReduction = pow(0.98f, magicArmor);
	std::cout<<"\nMagical damage reduced by magicArmor from "<<dmg;
	dmg *= magicArmorReduction;
	std::cout<<" to "<<dmg<<".";
	/// Currently no deduction is made.
	return Damage(dmg);
}


/// Bleh o-o
bool RuneBattler::LoadFromFile(String source){
	std::fstream file;
	file.open(source.c_str(), std::ios_base::in);
	if (!file.is_open()){
		std::cout<<"\nERROR: Unable to open file stream to "<<source;
		file.close();
		return false;
	}
	this->source = source;
	int start  = (int) file.tellg();
	file.seekg( 0, std::ios::end );
	int fileSize = (int) file.tellg();
	char * data = new char [fileSize];
	memset(data, 0, fileSize);
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	file.close();
	String fileContents(data);
	delete[] data; data = NULL;
	List<String> lines = fileContents.GetLines();
	for (int i = 0; i < lines.Size(); ++i){
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		List<String> tokens = line.Tokenize(" \t");
		if (tokens.Size() == 0)
			continue;
		String token = tokens[0];
		String token2;
		if (tokens.Size() >= 2)
			token2 = tokens[1];
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		std::cout<<"\nLine "<<i<<": "<<line;
		if (line.Length() < 2)
			continue;
		else if (line.Contains("name")){
			if (line.Contains("\"")){
				name = line.Tokenize("\"")[1];
			}
			else
				name = token2;
		}
		else if (token == "hp"){
			maxHP = hp = token2.ParseInt();
		}
		else if (token == "mp"){
			maxMP = mp = token2.ParseInt();
		}
		else if (token == "attack"){
			attack = token2.ParseInt();
		}
		else if (token == "agility"){
			agility = token2.ParseInt();
		}
		else if (token == "defense"){
			defense = token2.ParseInt();
		}
		else if (token == "magicPower"){
			magicPower = token2.ParseInt();
		}
		else if (token == "magicSkill"){
			magicSkill = token2.ParseInt();
		}
		else if (token == "speed"){
			speed = token2.ParseInt();
		}
		else if (token == "weaponDamage"){
			weaponDamage = token2.ParseInt();
		}
		else if (token == "armor"){
			armor = token2.ParseInt();
		}
		else if (token == "magicArmor"){
			magicArmor = token2.ParseInt();
		}
		else if (token == "spellDamage"){
			spellDamage = token2.ParseInt();
		}
		else if (token == "actions"){
		    String s = token2;
			std::cout<<"\nAction names before parsing: "<<actionNames.Size();
			for (int j = 0; j < actionNames.Size(); ++j){
				
			}
		    actionNames = s.Tokenize("\",");
		}
		else {
			std::cout<<"\nUnknown token "<<tokens[0]<<" while parsing RuneBattler data";
		//	assert(false && "Unknown token while parsing RuneBattler data");
		}
	}
	return true;
}
