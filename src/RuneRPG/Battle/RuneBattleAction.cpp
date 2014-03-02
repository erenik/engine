/// Emil Hedemalm
/// 2013-10-10

#include "RuneBattleAction.h"
#include "RuneBattler.h"
#include "Timer/Timer.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include <sstream>
#include "Message/MessageManager.h"
#include "Message/Message.h"

RuneBattleAction::RuneBattleAction() : BattleAction()
{
    //ctor
    className = "RuneBattleAction";

	/// 2 seconds duration default.
	duration = 2000;
}

RuneBattleAction::RuneBattleAction(const BattleAction & ref): BattleAction(ref)
{
    /// Initialize the rest.
    std::cout<<"\nYOOOOOOOOOOOOOOOOO";
}

RuneBattleAction::~RuneBattleAction()
{
    //dtor
}

void RuneBattleAction::OnBegin(BattleState & battleState){
	/// Set start time
	startTime = Timer::GetCurrentTimeMs();
	/// See if we've got a duration saved somewhere.

}
/// Should return true once the action (including animation, sound etc.) has been finished.
bool RuneBattleAction::Process(BattleState & battleState){
	int time = Timer::GetCurrentTimeMs();
	if (time - startTime > duration)
		return true;
	return false;
}
void RuneBattleAction::OnEnd(BattleState & battleState){
	std::stringstream ss;
	RuneBattler * primarySubject = subjects[0];
	if (!primarySubject)
		return;
	RuneBattler * primaryTarget = targets[0];		
	if (!primaryTarget)
		return;
	bool died = false;
    for (int i = 0; i < onEnd.Size(); ++i){
        String s = onEnd[i];
        s.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		if (s.Contains("//"))
			continue;
		if (s.Contains("PhysicalDamage(")){
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
				ss<<"Lucky hit! ";
			}
			else if (lucky < 50){
				luckyMiss = true;
				ss<<"Lucky miss! ";
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
					ss<<"Critical! ";
				}
				int variance = 2 + damage * 0.1f + damage * damage * 0.001f;
				int doubleVariance = variance * 2;
				int random = rand()%doubleVariance+1 - variance;
				damage += random;

				for (int i = 0; i < targets.Size(); ++i){
					died = targets[i]->PhysicalDamage(damage);
				}
				if (name == "Attack")
					ss<<primarySubject->name<<" attacks "<<primaryTarget->name<<" for "<<damage<<" points of damage!";
				else {
					ss<<primarySubject->name<<" uses "<<name;
					ss<<", dealing "<<damage<<" points of damage to ";
					if (targets.Size() == 1)
						ss<<targets[0]->name<<". ";
				}
			}
			else {
				if (subjects.Size())
					ss<<subjects[0]->name<<" misses! ";
			}
		}
		else if (s.Contains("MagicDamage(")){
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
				ss<<subjects[0]->name<<" casts "<<name;
			ss<<", dealing "<<damage<<" points of damage to ";
			if (targets.Size() == 1)
				ss<<targets[0]->name<<". ";
		}
        else if (s.Contains("Damage(")){
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
				ss<<subjects[0]->name<<"'s "<<name;
			ss<<" deals "<<damage<<" points of damage to ";
			if (targets.Size() == 1)
				ss<<targets[0]->name<<". ";
			
			
        }
        else if (s.Contains("Initiative(")){
            String s2 = s.Tokenize("()")[1];
            int v = s2.ParseInt();

            for (int p = 0; p < subjects.Size(); ++p){
                subjects[p]->initiative += v;
            }
        }
    }
    for (int p = 0; p < subjects.Size(); ++p){
        subjects[p]->OnActionFinished();
    }
	if (died){
		ss<<primaryTarget->name<<" is KO'd!";
	}
	if (!primaryTarget->isAI)
		MesMan.QueueMessage(new Message("OnAttackPlayers()"));
	/// Send the stringstream's characters to the narrator!
	String str = ss.str().c_str();
	Graphics.QueueMessage(new GMSetUIs("Narrator", GMUI::TEXT, str));
}
