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

#include "Maps/MapManager.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSetEntity.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "ModelManager.h"
#include "TextureManager.h"

#include "Random/Random.h"

RuneBattler::RuneBattler() : Battler()
{
	Nullify();
}

RuneBattler::~RuneBattler()
{
	this->actions.ClearAndDelete();
}


/// Creates the Entity to animate and visualize this battler while in action.
void RuneBattler::CreateEntity()
{
	/// Skip if we already have an entity. No need to re-create it, yo?
	if (entity)
		return;
	// Place them and their entities on the grid.
	Model * model = NULL;
	Texture * tex;
	if (isEnemy)
	{
		tex = TexMan.GetTexture("White");
		model = ModelMan.GetModel("obj/Sprite.obj");
	}
	// Player! Face 'em leftward!
	else 
	{
		tex = TexMan.GetTexture("Red");
		model = ModelMan.GetModel("obj/SpriteMirroredUVs.obj");
	}
	/// o.o
	entity = MapMan.CreateEntity("RuneBattlerEntity: "+name, model, tex);
	
	/// Give it animation set if applicable.
	if (animationSet.Length())
		Graphics.QueueMessage(new GMSetEntity(entity, GT_ANIMATION_SET, animationSet));
	/// Give a default animation set to those lacking one.
	else 
	{
		Graphics.QueueMessage(new GMSetEntity(entity, GT_ANIMATION_SET, "anim/Battle/SwordSlasher"));		
	}
	/// Scale it.
	Physics.QueueMessage(new PMSetEntity(entity, PT_SET_SCALE, 2.f));
	/// Set relative position due to the animation's form.
}

void RuneBattler::Nullify()
{
	unconscious = false;
	delay = 5000 + rand()%10000;
	hp = maxHP = maxMP = mp = 10;
	weaponDamage = spellDamage = 1;
	magicArmor = magicPower = magicSkill = spellDamage = 0;
//	initiative = 1000;
	state = BattlerState::WAITING_FOR_INITIATIVE;

	/// Default stats start always at 5.
	attackPower = 
		defensePower =
		speed =
		magicSkill =
		magicPower = 5;

	entity = NULL;
	isEnemy = false;
	actionPoints = 0;
	actionPointsFillUpSpeed = 0;
	
	/// These require further investigation later on!
	battleGearWeight = 0;
	battleWeightLimit = 1;
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
			agility = 5;
			armor = 5;
			speed = 5;
			weaponDamage = 5;
			spellDamage = 10;
			break;
		case 1:
			name = "Moffsi";
			hp = 16;
			agility = 5;
			armor = 5;
			speed = 3;
			weaponDamage = 3;
			break;
		case 2:
			name = "Gnupp";
			hp = 35;
			agility = 5;
			armor = 5;
			speed = 4;
			weaponDamage = 6;
			break;
		case 3:
			name = "Raowrg";
			hp = 35;
			mp = 10;
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


/// Called if actionPointsFillUpSpeed is set to 0. If speed changes, set it to 0 and it will be automatically re-calculated upon next iteration.
void RuneBattler::RecalculateActionPointsFillUpSpeed()
{
	this->actionPointsFillUpSpeed = (100 + pow(Speed(), 1.2f)) / 12.f / 1000.f;
	std::cout<<"Action points fill up speed per millisecond recalculated to: "<<this->actionPointsFillUpSpeed;
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
		if (this->actionPointsFillUpSpeed == 0)
			this->RecalculateActionPointsFillUpSpeed();

		this->actionPoints += actionPointsFillUpSpeed * battleState.timeDiff;
        if (actionPoints >= 100){
			if (!isAI){
				Message * msg = new Message(MessageType::STRING);
				msg->msg = "BattlerReady("+this->name+")";
				msg->data = (char*)this;
				MesMan.QueueMessage(msg);
			}
			actionPoints = 100;
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
	this->actionPoints = 0;
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

/// Average armor rating of all equipment pieces, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
float RuneBattler::ArmorRating()
{
	return 5.f;
}
/// Average magic armor rating of all equipment pieces, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
float RuneBattler::MagicArmorRating()
{
	return 5.f;
}
/// Weapon damage, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
float RuneBattler::WeaponDamage()
{
	return this->weaponDamage;
}
/// Shield block rating, including buffs/debuffs. Will mostly vary within the 0.0 to 20.0 interval.
float RuneBattler::ShieldBlockRating()
{
	return 0.f;
}
/// Shield defense modifier, including buffs/debuffs. Will mostly vary within the 0.0 to 20.0 interval.
float RuneBattler::ShieldDefenseModifier()
{
	return 0.f;
}


/// Returns current HP, including buffs/debuffs.
int RuneBattler::HP()
{
	return hp;
}
/// Returns current Attack power, including buffs/debuffs.
int RuneBattler::AttackPower()
{
	return attackPower;
}
/// Returns current physical and magical defense power, including buffs/debuffs.
int RuneBattler::DefensePower()
{
	return defensePower;
}
/// Returns current offensive magical power, including buffs/debuffs applied to it.
int RuneBattler::MagicPower()
{
	return magicPower;
}
/// Returns current agility, including buffs/debuffs.
int RuneBattler::Agility()
{
	return agility;
}
/// Returns current speed, including buffs/debuffs.
int RuneBattler::Speed()
{
	return speed;
}

/// Returns current amount of action points, floored.
int RuneBattler::ActionPoints()
{
	return int(actionPoints);
}

Random battlerRandom;
#define Maximum(a,b) ((a) > (b)? (a) : (b))

/// Returns true if a dodge occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::Dodge(int preDodgeDamage, int & dodgeModifiedDamage)
{

	float baseDodgeRating = Agility() * 0.25f + 1;
	float dodgeWeightModifier = Maximum(battleGearWeight / battleWeightLimit - 0.25, 0);
	float dodgeRating = baseDodgeRating * (1 - dodgeWeightModifier);
	float dodgeRand = battlerRandom.Randf() * 100.f;
	if (dodgeRand > dodgeRating)
		return false;
	/// In percent.
	float dodgeAmount = 0.40 + battlerRandom.Randf() * (0.80 + Agility() * 0.005);
	dodgeModifiedDamage *= 1 - dodgeAmount;
	return true;
}

/// Returns true if a parry occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::Parry(int preParryDamage, int & parryModifiedDamage)
{
	float parryRating = Agility() * 0.5f + 1;
	float parryRand = battlerRandom.Randf() * 100.f;
	if (parryRand > parryRating)
		return false;
	/// In percent.
	float parryAmount = 0.40 + battlerRandom.Randf() * (0.60 + Agility() * 0.00125);
	parryModifiedDamage *= 1 - parryAmount;
	return true;
}
/// Returns true if a block occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::ShieldBlock(int preBlockDamage, int & blockModifiedDamage)
{
	float baseBlockRating = 10 + 2 * ShieldBlockRating() * (1 + Agility() * 0.01f);
	float blockRand = battlerRandom.Randf() * 100.f;
	if (blockRand > baseBlockRating)
		return false;
	/// In percent. Allow blocking even without a shield? Sure!
	float blockAmount = 0.10 + 0.02 * (1 + ShieldDefenseModifier()) * (1 +  DefensePower() * 0.01f);
	int diff = blockAmount * preBlockDamage;
	if (diff <= 0)
		diff = 1;
	blockModifiedDamage -= diff;
	return true;
}
/// Returns true if a critical occurred. The resulting damage is stored in the second parameter.
bool RuneBattler::Critical(int preCriticalDamage, int & postCriticalDamage)
{
//	Base critical hit rate (5 to 55%) = 5 + Max(Agility, Attack power) * 0.125 %
//	Critical damage increase (+100 to +250%) = 100 + Random(0 to Max(Agility, Attack power) * 0.25) %
	float baseCriticalHitRate = 5 + Maximum(Agility(), AttackPower()) * 0.125f;
	float critRand = battlerRandom.Randf() * 100.f;
	if (critRand > baseCriticalHitRate)
		return false;
	float criticalIncrease = 1.f + battlerRandom.Randf() * Maximum(Agility(), AttackPower()) * 0.0025f;
	postCriticalDamage *= 1 + criticalIncrease;
	return true;
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
		Graphics.QueueMessage(new GMSetEntityb(entity, GT_VISIBILITY, false));
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
			maxMP = mp = token2.ParseInt();
		}
		else if (token == "attack"){
			attackPower = token2.ParseInt();
		}
		else if (token == "agility"){
			agility = token2.ParseInt();
		}
		else if (token == "defense"){
			defensePower = token2.ParseInt();
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
