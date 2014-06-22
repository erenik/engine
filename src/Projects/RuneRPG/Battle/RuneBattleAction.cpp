/// Emil Hedemalm
/// 2013-10-10

#include "RuneBattleAction.h"
#include "RuneBattler.h"
#include "Timer/Timer.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMAnimate.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"
#include <sstream>

RuneBattleAction::RuneBattleAction() 
: BattleAction()
{
    //ctor
    className = "RuneBattleAction";

	/// 2 seconds duration default.
	duration = 2000;
	Nullify();
}

RuneBattleAction::RuneBattleAction(const BattleAction & ref): BattleAction(ref)
{
    /// Initialize the rest.
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
}

RuneBattleAction::~RuneBattleAction()
{
    //dtor
}

/// Based on the one in BattleAction?
bool RuneBattleAction::Load(String fromFile)
{
	// Copy paste and edit from base one.. or just use base one for now..
	return BattleAction::Load(fromFile);
}


void RuneBattleAction::OnBegin(BattleState & battleState)
{
	/// Set start time
	startTime = Timer::GetCurrentTimeMs();
	/// See if we've got a duration saved somewhere.
	for (int i = 0; i < this->onBegin.Size(); ++i)
	{
		String line = this->onBegin[i];
		EvaluateLine(line);
	}
}

/// Should return true once the action (including animation, sound etc.) has been finished.
bool RuneBattleAction::Process(BattleState & battleState)
{
	int64 time = Timer::GetCurrentTimeMs();
	if (time - startTime > duration)
		return true;
	return false;
}

void RuneBattleAction::OnEnd(BattleState & battleState)
{
	// Remove it from it's subject's queue if not done so already. (some spells may continue after "casting" is done, and may not be present here then).
	for (int i = 0; i < subjects.Size(); ++i)
	{
		RuneBattler * subject = subjects[i];
		subject->actionsQueued.Remove(this);
		subject->OnActionFinished();
	}	
	narr = String();
	String str;
	primarySubject = subjects[0];
	if (!primarySubject)
		return;

	if (targets.Size())
	{
		primaryTarget = targets[0];
	}
	if (!primaryTarget)
		return;
	died = false;
    for (int i = 0; i < onEnd.Size(); ++i)
	{
        String s = onEnd[i];
        EvaluateLine(s);
    }
	if (died){
		Narrate(primaryTarget->name+" is KO'd!");
	}
	if (!primaryTarget->isAI)
		MesMan.QueueMessage(new Message("OnAttackPlayers()"));

	/// Send the stringstream's characters to the narrator!	
	if (narr.Length())
		Narrate(narr);
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
    else if (s.Contains("Initiative("))
	{
        String s2 = s.Tokenize("()")[1];
        int v = s2.ParseInt();

        for (int p = 0; p < subjects.Size(); ++p){
            subjects[p]->initiative += v;
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

	/// See if it hits at all.
	/// Re-seed the randomizer.
	srand(Timer::GetCurrentTimeMs());
	int hitProbability = (0.95f + (primarySubject->agility - primaryTarget->agility) * 0.01f) * 100.0f;

	/// Check for lucky hit/miss.
	int lucky = rand()%1000;
	bool luckyHit = false, luckyMiss = false;
	bool hit = false;
	if (lucky < 25){
		luckyHit = true;
		hit = true;
		narr = "Lucky hit! ";
	}
	else if (lucky < 50){
		luckyMiss = true;
		narr = "Lucky miss! ";
	}
	else {
		hit = (rand()%100 < hitProbability);
	}
	/// Calculate damage.
	if (hit){
		int criticalChance = 6 * relCritHitRatio;
		bool critical = (rand()%100 < criticalChance);
		int damage = primarySubject->weaponDamage * relWeaponDamage;
		if (damage < 1)
			damage = 1;
		
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
    int damage = args[damageIndex].ParseInt();
    if (damage < 1)
        damage = 1;
    for (int i = 0; i < targets.Size(); ++i){
        died = targets[i]->MagicDamage(damage);
    }
	if (subjects.Size())
		narr += subjects[0]->name + " casts "+name;
	narr +=  String(", dealing ") + damage + " points of damage to ";
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
