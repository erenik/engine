/// Emil Hedemalm
/// 2013-10-10

#include "RuneBattleAction.h"
#include "RuneBattler.h"
#include "BattleStats.h"

#include "Timer/Timer.h"

#include "Elements.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMAnimate.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"
#include <sstream>

#include "File/File.h"
#include "Random/Random.h"

Random random;

RuneBattleActionCategory::RuneBattleActionCategory()
{
	isAction = NULL;
};

RuneBattleAction::RuneBattleAction() 
{
	/// 2 seconds duration default.
	Nullify();
}

/// Will depend on the filter.
bool RuneBattleAction::HasValidTargets()
{
	switch(targetFilter)
	{
		case TargetFilter::ALLY:
		case TargetFilter::ALLIES:
		case TargetFilter::ENEMIES:
		case TargetFilter::ENEMY:
			if (this->targets.Size() == 0)
				return false;
			break;
		default:
			assert(false && "Implement");
	}
	return true;
}

void RuneBattleAction::Nullify()
{
	primaryTarget = NULL;
	primarySubject = NULL;

	spellPower = 0;
	
	/// Default is 1 enemy, okay?
	targetFilter = TargetFilter::ENEMY;
}

RuneBattleAction::~RuneBattleAction()
{
    //dtor
}

/// Based on the one in BattleAction?
bool RuneBattleAction::Load(String fromFile)
{
	assert(false);
	/*
	/// First load basic stuff.
	bool ok = BattleAction::Load(fromFile);
	/// Then load specific stuff for runeRPG in it.
	List<String> lines = File::GetLines(fromFile);
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.Contains("//"))
			continue;
		List<String> tokens = line.Tokenize(" \t");
		if (tokens.Size() < 2)
			continue;
		String key = tokens[0], value = tokens[1];
		key.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (key.Contains("SpellPower"))
			this->spellPower = value.ParseFloat();
	}
	*/
	bool ok = false;
	return ok;
}

void RuneBattleAction::EvaluateLine(String line)
{
	line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (line.Contains("//"))
		return;

	String s = line;
	if (s.Contains("QueueAnimation("))
	{
		String animation = s.Tokenize("()")[1];
		for (int i = 0; i < subjects.Size(); ++i)
		{
			RuneBattler * subject = subjects[i];
			Entity * entity = subject->entity;
			if (!entity)
				return;
			Graphics.QueueMessage(new GMQueueAnimation(animation, entity));
		}
	}
	else if (s.Contains("PhysicalDamage("))
	{
		PhysicalDamage(line);
	}
	else if (s.Contains("MagicDamage("))
	{
		MagicDamage(line);
	}
    else if (s.Contains("Damage("))
	{
         }
    else if (s.Contains("Initiative(") ||
		s.Contains("ActionPoints("))
	{
        String s2 = s.Tokenize("()")[1];
        int v = s2.ParseInt();

        for (int p = 0; p < subjects.Size(); ++p){
//            subjects[p]->initiative += v;
			subjects[p]->actionPoints -= v;
        }
    }
}


void RuneBattleAction::PhysicalDamage(String line)
{
	String s = line;
	List<String> args = s.Tokenize("(,\")");
	float relAcc, relAtt, relWeaponDamage, relCritHitRatio, relCritMultiplier;
	relAcc = relAtt = relWeaponDamage = relCritHitRatio = relCritMultiplier = 1.0f;
	/// Parse the arguments, setting defaults to 1.0f if none were given.
	switch(args.Size()){
		/// Critical damage multiplier
		case 5:
			relCritMultiplier = args[5].ParseFloat();
		/// Critical hit ratio
		case 4:
			relCritHitRatio = args[4].ParseFloat();
		/// Weapon damage
		case 3:
			relWeaponDamage = args[3].ParseFloat();
		/// Attack power
		case 2:
			relAtt = args[2].ParseFloat();
		/// Accuracy
		case 1:
			relAcc = args[1].ParseFloat();
	}

	/**
		Base damage = floor ( (Weapon damage * 5 + attack power * 2)^1.7 
			/ (Armor rating * 2 + defense power)^1.7 
			* (1.0 + (attack power / defense power)^0.5 ) )
	*/
	RuneBattler * subject = primarySubject;
	RuneBattler * target = primaryTarget;
	float rawAttack = pow(subject->WeaponDamage() * 5.f + subject->AttackPower() * 2.f, 1.7f);
	float rawArmor = pow(target->ArmorRating() * 2.f + target->DefensePower(), 1.7f);
	float attackDefenseMultiplier = 1.0 + pow(subject->AttackPower() / target->DefensePower(), 0.5f);
	float baseDamagef = rawAttack / rawArmor * attackDefenseMultiplier;
	int baseDamage = int(baseDamagef);

	int damage = baseDamage;
	if (target->Dodge(damage, damage))
		narr += "Dodge! ";
	if (damage > 0)
	{
		if (target->Parry(damage, damage))
			narr += "Parry! ";
		if (damage > 0)
		{
			if (target->ShieldBlock(damage, damage))
				narr += "Block! ";
			if (subject->Critical(damage,damage))
				narr += "Critical! ";
		}
	}
	if (damage < 0)
		damage = 0;
	// Stuff.
	for (int i = 0; i < targets.Size(); ++i)
	{
		died = targets[i]->PhysicalDamage(damage);
	}
	// Narrate accordingly.
	if (name == "Attack")
		narr += primarySubject->name + " attacks " + primaryTarget->name + " for " + damage + " points of damage!";
	else 
	{
		narr += primarySubject->name + " uses " + name;
		narr += String(", dealing ") + damage + " points of damage to ";
		if (targets.Size() == 1)
			narr += targets[0]->name + ". ";
	}
/*
		if (critical){
			float criticalMultiplier = 1.75f * relCritMultiplier;
			damage *= criticalMultiplier;
			narr += "Critical! ";
		}
		int variance = 2 + damage * 0.1f + damage * damage * 0.001f;
		int doubleVariance = variance * 2;
		int random = rand()%doubleVariance+1 - variance;
		damage += random;

		for (int i = 0; i < targets.Size(); ++i)
		{
			died = targets[i]->PhysicalDamage(damage);
		}
		if (name == "Attack")
			narr += primarySubject->name + " attacks " + primaryTarget->name + " for " + damage + " points of damage!";
		else {
			narr += primarySubject->name + " uses " + name;
			narr += String(", dealing ") + damage + " points of damage to ";
			if (targets.Size() == 1)
				narr += targets[0]->name + ". ";
		}
	}
	else {
		if (subjects.Size())
			narr += subjects[0]->name + " misses! ";
	}
	*/
}



void RuneBattleAction::MagicDamage(String line)
{
	String narr;
	String s = line;
	List<String> args = s.Tokenize("(,\")");
	for (int j = 0; j < args.Size(); ++j){
		String arg = args[j];
		std::cout<<"\nArg "<<j<<": "<<arg;
	}
    int damageIndex = -1;
    if (args.Size() > 2){
        /// Parse target first.
        /// Assume enemy for now..
        damageIndex = 2;
    }
    else if (args.Size() > 1){
        damageIndex = 1;
    }

	RuneBattler * subject = primarySubject;
	int baseMagicDamage = this->spellPower * 1.f + pow((1 + spellPower * 0.1f) * subject->MagicPower(), 1.2f);
	RuneBattler * target = primaryTarget;
	float magicDamageReductionMultiplier = pow(0.99f, pow(target->MagicArmorRating() * 3 + target->DefensePower(), 0.99f));
	int magicDamage = baseMagicDamage * magicDamageReductionMultiplier;
	int damageReduced = magicDamage - baseMagicDamage;
	target->Damage(magicDamage);

	if (subjects.Size())
		narr += subjects[0]->name + " casts "+name;
	narr +=  String(", dealing ") + magicDamage + " points of damage to ";
	if (targets.Size() == 1)
		narr += targets[0]->name + ". ";
	Narrate(narr);
}

void RuneBattleAction::Damage(String line)
{
	String s = line;
	List<String> args = s.Tokenize("()");
    int damageIndex = -1;
    if (args.Size() > 2){
        /// Parse target first.
        /// Assume enemy for now..
        damageIndex = 2;
    }
    else if (args.Size() > 1){
        damageIndex = 1;
    }
    int damage = args[damageIndex].ParseInt();
    if (damage < 1)
        damage = 1;
    for (int i = 0; i < targets.Size(); ++i){
        died = targets[i]->Damage(damage);
    }
	if (subjects.Size())
		narr += subjects[0]->name + "'s "+name;
	narr += String(" deals ") + damage + " points of damage to ";
	if (targets.Size() == 1)
		narr += targets[0]->name + ". ";
}


/// Send to battle narrator.
void RuneBattleAction::Narrate(String line)
{
	Graphics.QueueMessage(new GMSetUIs("Narrator", GMUI::TEXT, line));
}

/// Checks for a name and stuff.
bool RuneBattleAction::IsValid()
{
	if (name.Length() <= 0)
		return false;
	return true;
}

/// Parses a string with targets as defined in a CSV file. E.g. "Ally Select 1"
void RuneBattleAction::ParseTargets(String fromTargetString)
{
	/// If-else should do.
	if (fromTargetString.Contains("Select 1"))
		targetFilter = TargetFilter::ENEMY;
	else if (fromTargetString.Contains("Self"))
		targetFilter = TargetFilter::SELF;
	else if (fromTargetString.Contains("World"))
		targetFilter = TargetFilter::POINT;
	/*
	List<String> toks = fromString.Tokenize(" ");
	for (int i = 0; i < toks.Size(); ++i)
	{

		// spell->SetTargetFilterByString(word);
	}
	*/
}	

void RuneBattleAction::ParseElements(String fromString)
{
	List<String> toks = fromString.Tokenize(" ");
	for (int i = 0; i < toks.Size(); ++i)
	{
		// spell->SetTargetFilterByString(word);
	}	
}

void RuneBattleAction::ParseDurations(String fromString)
{
	// Get latest effect and set duration for it.
	if (effects.Size() == 0)
	{
	//	std::cout<<"\nNo effect to attach duration to!";					
		return;
	}

	List<String> tokens = fromString.Tokenize(" ");
	/// Poke Karl about this... easier parsing would be nice.

//	BattleEffect & effect = spell->effects.Last();
//	effect.durationInMs = BattleEffect::INSTANTANEOUS; // GetDurationByString(word);

}

void RuneBattleAction::ParseEffects(String fromString)
{
	List<String> tokens = fromString.Tokenize(" ");
	List<String> effectTokens = fromString.Tokenize("()");
	// Add them one at a time.
	for (int i = 0; i < effectTokens.Size(); i += 2)
	{
		String effectStr = effectTokens[i];
		String argument;
		if (effectTokens.Size() > i + 1)
			argument = effectTokens[i+1];
		effectStr.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		BattleEffect effect;
		if (effectStr.Contains("Increase"))
		{
			effect.type = BattleEffect::INCREASE;
		}
		else if (effectStr.Contains("Add Damage"))
		{
			effect.type = BattleEffect::ADD_DAMAGE;
			/// Parse element here too then.
			effect.element = GetElementByString(effectStr);
		}
		else if (effectStr.Contains("Damage"))
		{
			effect.type = BattleEffect::DAMAGE;
			effect.element = GetElementByString(effectStr);
		}
		else 
		{
			/// Skip if no good effect was determined.
			continue;
		}
		switch(effect.type)
		{
			case BattleEffect::DECREASE:
			case BattleEffect::INCREASE:
			{
				// Check next token.
				effect.statType = GetStatByString(effectStr);
				if (effect.statType == Stat::INVALID)
					continue;
				effect.argument = argument;
				effects.Add(effect);
			}
			break;
		}
	}

	// Parse it one character at a time, since we have parenthesis and stuff? or..
	/*
	// Create a new effect!
	Effect * effect = new Effect();
	effect->SetType(value);
	spell->effects.Add(effect);
	*/
}

void RuneBattleAction::ParseOtherCosts(String fromString)
{
	// o.o
}

void RuneBattleAction::SetElements(String toParse)
{
	elements.Clear();
	toParse.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (toParse.Contains("Fire"))
		elements.Add(Element::FIRE);
	if (toParse.Contains("Water"))
		elements.Add(Element::WATER);
	if (toParse.Contains("EARTH"))
		elements.Add(Element::EARTH);
	if (toParse.Contains("AIR"))
		elements.Add(Element::AIR);
	
	if (toParse.Contains("CHAOS"))
		elements.Add(Element::CHAOS);
	if (toParse.Contains("BALANCE"))
		elements.Add(Element::BALANCE);
	if (toParse.Contains("LIFE"))
		elements.Add(Element::LIFE);
	if (toParse.Contains("DEATH"))
		elements.Add(Element::DEATH);
}



