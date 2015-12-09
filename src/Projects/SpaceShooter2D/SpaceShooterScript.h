/// Emil Hedemalm
/// 2015-12-07
/// Space-Shooter specific scripting, containing bindings between classes here and functions in the script.

#ifndef SPACE_SHOOTER_SCRIPT_H
#define SPACE_SHOOTER_SCRIPT_H

#include "Script/Script.h"

class SpaceShooterScript : public Script 
{
public:
	SpaceShooterScript();
	virtual void EvaluateLine(String & line);
	virtual void EvaluateFunction(String function, List<String> arguments);

private:
	int sssState; // space shooter script state. - whatever that means :D
};

#include "MathLib/FunctionEvaluator.h"
class SpaceShooterEvaluator : public FunctionEvaluator 
{
public:
	virtual bool EvaluateFunction(String byName, List<String> arguments, ExpressionResult & result);
	virtual bool IsFunction(String name);
};

extern SpaceShooterEvaluator spaceShooterEvaluator;


#endif