// Emil Hedemalm
// 2013-07-16

#ifndef AE_EVENT_H
#define AE_EVENT_H

#include <String/AEString.h>
#include "MathLib/Vector3f.h"

/// Compact saveable version of the event
struct CompactEvent{};
class GameState;

// Script flags
#define DELETE_WHEN_ENDED	0x00000001

#define Script Script

/// An arbitrary script-sequence.
class Script {
public:
	Script(const Script & base);
	Script(String name = "Script", Script * parentScript = NULL);

	// Playback functions.
	// Returns true if it was paused. Some scripts may not be pausable, as defined when loading them or when a flag is set during processing.
	bool Pause();
	void Resume();
	bool IsPaused() {return paused;};

	/// Queries this script to end if it hasn't already.
	void QueueEnd();

	/// Loads script using given name as reference to source-file.
	bool Load();
	/// Wosh o.o, NOTE that the root dir will be appended at the start automatically! <- Wat?
	bool Load(String fromFile);

	/// Regular state-machine mechanics for the events, since there might be several parralell events?
	virtual void OnBegin();
	virtual void Process(long long time);
	virtual void OnEnd();
	void EvaluateLine(String & line);
	/// Resets the event so that it can be re-played again!
	void Reset();

	/// o.o
	virtual void OnScriptEnded(Script * childScript);

	/// File I/O. Reading will reset data.
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

	/// Sets the flag to delete the event once it's finished.
	void SetDeleteOnEnd(bool value);

	/// Continue to dat alternative (for branching with Alternatives/Questions)
	bool ContinueToAlternative(String alternative);

	/// To avoid long names in the editor, et al.
	static const char * rootEventDir;

	enum scriptStates {
		NOT_BEGUN,
		BEGUN,
		PAUSED,
		ENDING, FINISHING = ENDING,
		ENDED,
	};

	/// Script name. May also be the contents of the script for custom one-liner basic functions.
	String name;
	/// Source-script file to parse and load/use when activathed.
	String source;
	Vector3f position;
	// See enum scriptStates.
	int scriptState;
	
	/// To keep track of things.
	Script * parent;
	List<Script*> childScripts;

	/// Wosh?
	bool loaded;
	bool executed;
	bool repeatable;

	// Set this to false for those scripts that do not require waiting for completion! o.o
	bool pausesExecution;

	// For keeping track of time that has passed for a current script. Needed in order to avoid debug- and lag-times affecting outcome.
	int timePassed;

	enum triggerType {
		NULL_TRIGGER_TYPE,
		ON_ENTER, // OnEnter = On Entering map
		ON_TOUCH,	// OnTouch = When stepping on the tile
		ON_APPROACH, // OnApproach = When one tile away and approaching the specified tile.
	};

	/** If this was called from another script. Mark it here. 
		Callback method OnScriptFinished() will then be called on the parent upon completion.
	*/
	Script * parentScript;


	void BeginCutscene();
	void EndCutscene(bool endingPrematurely = false);

	/// good flag
	bool inCutscene;

	/// Main trigger-condition type, which can be one of the previous enums.
	int triggerCondition;
	/// When executing, keeps track of which line we were on. Starts at.. 0?
	int currentLine;

	/// All that should happen when the event triggers..!
	List<String> lines;
	/// For checking that whatever the line wanted to do got finished.
	bool lineFinished;
	/// If the line has been processed.
	bool lineProcessed;

	/// Whetehr this script disabled any ui...
	bool uiDisabled;

	/// Like DELETE_WHEN_ENDED,
	int flags;
private:

	/// For if- and elsif- statements.
	void HandleConditional(String line);
	void JumpToNextConditional();
	void JumpToEndif();

	/// Flag to true once it processes any if-case-thingy?
	bool ifProcessed;

	bool paused;
	/// For handling stuff...
	bool isInAlternativeDialogue;
};


#endif

