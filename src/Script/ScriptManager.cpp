// Emil Hedemalm
// 2013-07-17

#include "Script.h"
#include "ScriptManager.h"
#include "Input/InputManager.h"

ScriptManager::ScriptManager(){
	// Wosh.
}
ScriptManager::~ScriptManager(){
	// Do nothing with the lists by default!
	for (int i = 0; i < activeScripts.Size(); ++i){
		Script * e = activeScripts[i];
		if (e->flags & DELETE_WHEN_ENDED)
			delete e;
	}
}
ScriptManager * ScriptManager::eventManager = NULL;
ScriptManager * ScriptManager::Instance(){
	assert(eventManager);
	return eventManager;
}
void ScriptManager::Allocate(){
	assert(eventManager == NULL);
	eventManager = new ScriptManager();
}
void ScriptManager::Deallocate(){
	assert(eventManager);
	delete eventManager;
	eventManager = NULL;
}
/** Triggers OnEnter of the event as registers it for processing which is called once per frame via the StateProcessor!
	DO NOTE: that the event manager is NOT in any way responsible for disposing of events, as these will probably best be controlled 
	via the entities, maps or state machines that use them!
	- Alternatively a boolean could be set which toggles this behaviour, but plan ahead and make sure that whatever you do doesn't leak memory!
*/
void ScriptManager::PlayScript(Script * newScriptToPlay)
{
	/// Check for multiple instances first!
	if (!newScriptToPlay->allowMultipleInstances)
	{
		for (int i = 0; i < activeScripts.Size(); ++i)
		{
			Script * script = activeScripts[i];
			if (newScriptToPlay->name == script->name)
			{
				std::cout<<"\nScript "<<script->name<<" already running! Skipping new instance.";
				/// Delete if specified.
				if (newScriptToPlay->flags & DELETE_WHEN_ENDED)
					delete newScriptToPlay;
				return;
			}	
		} 
	}
	// If needed, load it first.
	if (!newScriptToPlay->loaded)
		newScriptToPlay->Load();
	newScriptToPlay->OnBegin();
	activeScripts.Add(newScriptToPlay);
}
void ScriptManager::Process(int timeInMs)
{
//	std::cout<<"\nEventManager::Process";
	for (int i = 0; i < activeScripts.Size(); ++i)
	{
	//	std::cout<<"\nActive events: "<<activeEvents.Size();
		Script * script = activeScripts[i];
		if (script->IsPaused())
			continue;
		// Delete if ended! o.o
		if (script->scriptState == Script::ENDED)
		{
			activeScripts.Remove(script);
			/// Delete if specified.
			if (script->flags & DELETE_WHEN_ENDED)
				delete script;
			/// Step down i since we will remove this element, or one event will be skipped!
			--i;
			continue;
		}
		// Process if alive
		script->Process(timeInMs);
		if (script->scriptState == Script::ENDING)
		{
			script->OnEnd();
			this->finishedEvents.Add(script);
			activeScripts.Remove(script);
			/// Delete if specified.
			if (script->flags & DELETE_WHEN_ENDED)
				delete script;
			--i;
			continue;
		}
		if (activeScripts.Size() == 0){
			Input.NavigateUI(false);
		}
		else
			/// Most events assume UI-interaction, yes?
			Input.NavigateUI(true);
	}
}

/// Notifies all cutscene-flagged scripts to end the cinematics.
void ScriptManager::SkipCutscene()
{
	for (int i = 0; i < activeScripts.Size(); ++i)
	{
		Script * script = activeScripts[i];
		script->EndCutscene(true);
	}
}

/// Pauses all scripts, returning a list of all that now became paused.
List<Script*> ScriptManager::PauseAll()
{
	List<Script*> scriptsPaused;
	for (int i = 0; i < activeScripts.Size(); ++i)
	{
		Script * script = activeScripts[i];
		if (script->Pause())
		{
			scriptsPaused.Add(script);
		}
	}
	return scriptsPaused;
}

void ScriptManager::ResumeScripts(List<Script *> scripts)
{
	for (int i = 0; i < scripts.Size(); ++i)
	{
		Script * script = scripts[i];
		script->Resume();
	}
}
