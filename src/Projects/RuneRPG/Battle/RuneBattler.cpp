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
#include "File/File.h"
#include "RuneBattleActionLibrary.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"


RuneBattler::RuneBattler() : Battler()
{
	Nullify();
}

RuneBattler::~RuneBattler()
{
	this->actions.ClearAndDelete();
}

void RuneBattler::Nullify()
{
	unconscious = false;
	delay = 5000 + rand()%10000;
	hp = maxHP = maxMP = mp = 10;
	weaponDamage = spellDamage = 1;
	magicArmor = magicPower = magicSkill = spellDamage = 0;
	initiative = 1000;
	state = BattlerState::WAITING_FOR_INITIATIVE;

	entity = NULL;
	isEnemy = false;
}

/// from 0 to 4, 0 being player, 1-3 being enemies 1-3
RuneBattler::RuneBattler(int defaultTypes) 
: Battler()
{
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

/// If dead, for example, it is not.
bool RuneBattler::IsARelevantTarget()
{
	if (this->state == BattlerState::INCAPACITATED ||
		this->unconscious == true)
		return false;
	return true;
}


void RuneBattler::Process(BattleState &battleState)
{
	// Woo!
	if (state == BattlerState::INCAPACITATED ||
		unconscious == true)
		return;

    /// Check state, has it queued an action yet?
	if (state == BattlerState::WAITING_FOR_INITIATIVE)
	{
		initiative -= battleState.timeDiff * speed;
        if (initiative <= 0){
			if (!isAI){
				Message * msg = new Message(MessageType::STRING);
				msg->msg = "BattlerReady("+this->name+")";
				msg->data = (char*)this;
				MesMan.QueueMessage(msg);
			}
            initiative = 0;
			state = BattlerState::IDLE;
        }
    }

    /// Process status-effects.

    /// Return if no AI.
	if (!isAI || state != BattlerState::IDLE)
        return;

	/// Queue up an action, random ones work fine.
	if (actions.Size() == 0)
	{
		std::cout<<"\nBattler lacking any valid action to take";
		// Load 'em.
		if (!UpdateActions())
			return;
	}
	BattleAction * ba = actions[rand()%actions.Size()];
	RuneBattleAction * rba = new RuneBattleAction(*ba);
	rba->subjects = this;
	RuneBattler * b = (RuneBattler*)BattleMan.RandomTarget(rba->targetFilter, this);
	if (b)
		rba->targets.Add(b);
	/// What if no valid targets?
	if (!rba->HasValidTargets())
	{
		// Then abort it.
		delete rba;
		return;
	}
	BattleMan.QueueAction(rba);
	state = BattlerState::PREPARING_ACTION;

    /// Since no good AI atm, return.
    return;
}

void RuneBattler::OnActionFinished()
{
	state = BattlerState::WAITING_FOR_INITIATIVE;
}

/// Checks the initiative-parameter!
bool RuneBattler::IsIdle()
{
	if (state == BattlerState::IDLE)
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
bool RuneBattler::Damage(int dmg)
{
	hp -= dmg;
	if (hp <= 0)
	{
		OnKO();
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
bool RuneBattler::MagicDamage(int dmg)
{
	float magicArmorReduction = pow(0.98f, magicArmor);
	std::cout<<"\nMagical damage reduced by magicArmor from "<<dmg;
	dmg *= magicArmorReduction;
	std::cout<<" to "<<dmg<<".";
	/// Currently no deduction is made.
	return Damage(dmg);
}

/// Updates which actions it has available using the RuneBattleAction library...?
bool RuneBattler::UpdateActions()
{
	actions.Clear();
	for (int i = 0; i < actionNames.Size(); ++i)
	{
		String name = actionNames[i];
		const RuneBattleAction * action = RBALib.GetBattleAction(name);
		if (action)
			this->AddActions(new RuneBattleAction(*action));
	}
	if (actions.Size())
		return true;
	return false;
}

/// When getting knocked out/incapacitated, cancels queued actions, etc.
void RuneBattler::OnKO()
{
	// Reset some variables.
	hp = 0;
	unconscious = true;
	state = BattlerState::INCAPACITATED;

	/// Queue a message of the event to the game state.
	MesMan.QueueMessage(new Message("OnBattlerIncapacitated("+name+")"));
	
	// Notify all queued actions to be halted.
	for (int i = 0; i < this->actionsQueued.Size(); ++i)
	{
		BattleAction * ba = actionsQueued[i];
		ba->Cancel();
	}
	/// If we got a visible entity, animate it for death.
	if (entity)
	{
		// Make it invisible!
		Graphics.QueueMessage(new GMSetEntityb(entity, VISIBILITY, false));
	}
}


/// Bleh o-o
bool RuneBattler::Load(String source)
{
	this->source = source;
	enum {
		NULL_MODE,
		ACTIONS, SKILLS = ACTIONS,
	};
	int inputMode = NULL_MODE;
	List<String> lines = File::GetLines(source);
	if (!lines.Size())
		return false;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String & line = lines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
			continue;
		List<String> tokens = line.Tokenize(" \t");
		/// One or more word below
		if (tokens.Size() == 0)
			continue;
		String token = tokens[0];
		String key = tokens[0];
		key.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (key == "Actions" || key == "Skills")
		{
			inputMode = ACTIONS;
			continue;
		}
		else if (inputMode == ACTIONS)
		{
			this->actionNames.Add(key);
		}

	

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
		else if (line.Contains("animationSet"))
		{
			animationSet = token2;
		}
		else if (token == "hp"){
			maxHP = hp = token2.ParseFloat();
		}
		else if (token == "mp"){
			maxMP = mp = token2.ParseFloat();
		}
		else if (token == "attack"){
			attack = token2.ParseFloat();
		}
		else if (token == "agility"){
			agility = token2.ParseFloat();
		}
		else if (token == "defense"){
			defense = token2.ParseFloat();
		}
		else if (token == "magicPower"){
			magicPower = token2.ParseFloat();
		}
		else if (token == "magicSkill"){
			magicSkill = token2.ParseFloat();
		}
		else if (token == "speed"){
			speed = token2.ParseFloat();
		}
		else if (token == "weaponDamage"){
			weaponDamage = token2.ParseFloat();
		}
		else if (token == "armor"){
			armor = token2.ParseFloat();
		}
		else if (token == "magicArmor"){
			magicArmor = token2.ParseFloat();
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
