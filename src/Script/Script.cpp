// Emil Hedemalm
// 2013-07-16

#include "Script.h"
#include <fstream>
#include "UI/UIButtons.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "UI/UILists.h"
#include "Game/GameVariableManager.h"
#include <cstring>
#include "ScriptManager.h"
#include "GeneralScripts.h"
#include "Audio/TrackManager.h"
#include "Input/InputManager.h"
#include "File/FileUtil.h"
#include "TextureManager.h"
#include "MathLib/Expression.h"

/// Compact saveable version of the event
//struct CompactEvent{};

const char * Script::rootEventDir = "data/scripts/";

Script::Script(const Script & base)
{
	Nullify();
	name = base.name;
	source = base.source;
	triggerCondition = base.triggerCondition;
	loaded = base.loaded;
	executed = base.executed;
	repeatable = base.repeatable;
	currentLine = base.currentLine;
	scriptState = base.scriptState;
	lineFinished = base.lineFinished;
	flags = base.flags;
	/// Copy loaded data too.
	lines = base.lines;
	pausesExecution = base.pausesExecution;
}

Script::Script(String name, Script * parent /* = NULL */ )
{
	Nullify();
	this->name = name;
	this->parent = parent;
//	source = "Test.e";
	// Assume child scripts are inherent C++ classes which do not require further loading.
	if (parent){
		loaded = true;
	}
	if (parent)
	{
		parent->childScripts.Add(this);
	}
};

void Script::Nullify()
{
	entity = 0;
	allowMultipleInstances = true;
	parentScript = NULL;
	timePassed = 0; // Should be added each process-frame.
	// Set this to false for those scripts that do not require waiting for completion! o.o
	pausesExecution = true;
	inCutscene = false;
	lineFinished = false;
	lineProcessed = false;
	uiDisabled = false;
	paused = false;
	executed = false;
	repeatable = false;
	currentLine = 0;
	scriptState = NOT_BEGUN;
	flags = 0;
	timePassed = 0;
	loaded = false;
	triggerCondition = NULL_TRIGGER_TYPE;
}

// Playback functions.
bool Script::Pause()
{
	paused = true;
	return true;
}
void Script::Resume()
{
	paused = false;
}

/// Queries this script to end if it hasn't already.
void Script::QueueEnd()
{
	if (scriptState == ENDED)
		return;
	scriptState = ENDING;
}


void Script::Reset()
{
	currentLine = -1;
	scriptState = NOT_BEGUN;
	lineFinished = lineProcessed = false;
	executed = false;
}

/// Loads script using given name as reference to source-file.
bool Script::Load()
{
	std::cout<<"\nScript::Load called: "<<source;
	bool result = Load(source);
	assert(result && "Was unable to load target script.");
	loaded = result;
	return result;
}

/// Wosh o.o, NOTE that the root dir will be appended at the start automatically!
bool Script::Load(String fromFile)
{
	std::cout<<"\nScript::Load fromFile: "<<fromFile;
//	SleepThread(1000);
	/// Already loaded, reset and re-load!
	if (loaded){
		std::cout<<"\nAlready loaded, resetting.";
		loaded = false;
		Reset();
		lines.Clear();
	}
//	std::cout<<"\nSaving source path...";
	name = source = fromFile;
	/// Parsley parse, yes?
	/// Add root event dir if not already included (could be)
	/// Assure that at least the data/ dir is included. Any path with it can be assumed to be completely relative!
//	if (!source.Contains(rootEventDir) && !source.Contains("data/"))
//		source = rootEventDir + source;

//	std::cout<<"\nTrying to open file...";
	int loadingType = 0;
	bool midComment = false;
	std::cout<<"\nFetching lines.";
//	SleepThread(1000);
	List<String> sourceLines = 	File::GetLines(source);
	std::cout<<"\nLines: "<<sourceLines.Size();
//	SleepThread(1000);
	for (int i = 0; i < sourceLines.Size(); ++i)
	{
		String & line = sourceLines[i];
		// Try load the battler from the relative directory.
		if (line.StartsWith("//"))
			continue;
		if (line.Contains("/*"))
			midComment = true;
		else if (line.Contains("*/"))
			midComment = false;
		if (midComment)
			continue;
		List<String > tokens = line.Tokenize(" \t");
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
#define ASSERT_TWO_TOKENS {if(tokens.Size() < 2) continue;}
		if (line.Length() < 3)
			continue;
	//	ASSERT_TWO_TOKENS;
		if (line.Contains("name")){
			line.Remove("name");
			line.RemoveInitialWhitespaces();
			name = line;
		}
		else if (line.Contains("TriggerCondition"))
		{
			String tok = tokens[1];
			if (tok == "OnEnter")
				triggerCondition = ON_ENTER;
			else if (tok == "OnTouch")
				triggerCondition = ON_TOUCH;
			else if (tok == "OnApproach")
				triggerCondition = ON_APPROACH;
		}
		else {
			/// Dump the rest to the lines of execution.
			lines.Add(line);
		}
	}
	/// Save source, without the dir dir please!
	this->source = fromFile;
	loaded = true;
	std::cout<<"\nScript loaded.";
//	SleepThread(100);
	return true;
}

/// Regular state-machine mechanics for the events, since there might be several parralell events?
void Script::OnBegin()
{
	scriptState = BEGUN;
	currentLine = 0;
	lineProcessed = false;
}

void Script::Process(int timeInMs)
{
	if (paused)
		return;
	if (currentLine < 0 || currentLine >= lines.Size())
	{
		std::cout<<"\nScript::Process: Line "<<currentLine<<" out of scope. Aborting";
		scriptState = ENDED;
		return;
	}
	String line = lines[currentLine];
	int p = 2;
	if (p == 3)
		for (int i = 0; i < lines.Size(); ++i){std::cout<<"\nLines "<<i<<": "<<lines[i];}
	line.RemoveSurroundingWhitespaces();
	// Skip one-line comments if any remain.
	if (line.StartsWith("//"))
	{
		++currentLine;
		return;
	}
	timePassed += (int)timeInMs;

	// Line not processed? Evaluate it.
	if (!lineProcessed)
	{
		/// Evaluate the current line, look if we have to check for line-finishing conditions or not.
		lineFinished = false;
		EvaluateLine(lines[currentLine]);
	}

	/// Check if the current line is finished? If not, wait until it is finished.
	if (!lineFinished)
		return;

	/// If it is finished, check for next line
	++currentLine;
	/// If we finished the last line, flag this event as ending...!
	if (currentLine >= lines.Size()){
		scriptState = ENDING;
		return;
	}
	/// New line: specify it as not finished.!
	lineFinished = false;
	lineProcessed = false;
}

void Script::OnEnd()
{
	// In-case somebody forgot to, like.. re-enable stuff again, do it now.
	if (inCutscene)
		EndCutscene();
	if (uiDisabled)
		InputMan.EnableActiveUI();

	if (repeatable)
	{
		scriptState = NOT_BEGUN;		
		currentLine = -1;
		OnBegin();
		return;
	}
	/// Flag the event as finished!
	scriptState = ENDED;
	/// Notify parents if needed.
	if (parent)
		parent->OnScriptEnded(this);
}

/// o.o
void Script::OnScriptEnded(Script * childScript)
{
	// Mark line as finished executing.
	if (childScript->pausesExecution)
		lineFinished = true;
}

void Script::BeginCutscene()
{
	inCutscene = true;
}
void Script::EndCutscene(bool endingPrematurely /*= false*/)
{
	// Make sure we're in a cutscene first..
	if (!inCutscene)
		return;
	if (endingPrematurely)
	{
		// Jump to the end of the cutscene.
		for (int i = currentLine; i < lines.Size(); ++i)
		{
			// Look for the end.
			String line = lines[i];
			if (line.Contains("End(Cutscene)"))
			{
				// Jump to it.
				currentLine = i;
				// Stop doing whatever we were doing too.
				lineFinished = true;
				break;
			}
		} 
	}
	// End all sub-scripts too!
	for (int i = 0; i < childScripts.Size(); ++i)
	{
		Script * script = childScripts[i];
		script->QueueEnd();
	}

	inCutscene = false;
}


void Script::EvaluateLine(String & line)
{
	/// Default line processed once?
	lineProcessed = true;

	line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	// "80Gray50Alpha.png"
#define DEFAULT_TEXTURE_SOURCE	"black50Alpha.png"
#define DEFAULT_TEXT_SIZE_RATIO	0.3f
	
	/// Some state began, take not of it?
	if (line.Contains("Wait("))
	{
		WaitScript * wait = new WaitScript(line, this);
		wait->SetDeleteOnEnd(true);
		ScriptMan.PlayScript(wait);
	}
	else if (line.StartsWith("Key:"))
	{
		String keyStr = line.Tokenize(":")[1];
		keyStr.RemoveSurroundingWhitespaces();
		int keyCode = GetKeyForString(keyStr);
		assert(keyCode != 0);
		InputMan.KeyDown(keyCode, false);
		InputMan.KeyUp(keyCode);
		lineFinished = true;
	}
	else if (line.Contains("PlayScript("))
	{
		List<String> tokens = line.Tokenize("(),");
		// Source of script within the parenthesis.
		String source = tokens[1];
		bool wait = true;
		Script * scriptParent = this;
		if (tokens.Size() >= 3)
		{
			wait = tokens[2].ParseBool();
			if (!wait)
			{
				scriptParent = NULL;
				this->lineFinished = true;
			}
		}
		Script * script = new Script(source, scriptParent);
		script->source = source;
		bool loaded = script->Load();
		assert(loaded);
		ScriptMan.PlayScript(script);
	}
	else if (line == "DisableActiveUI")
	{
		InputMan.DisableActiveUI();
		lineFinished = true;
		uiDisabled = true;
	}
	else if (line == "EnableActiveUI")
	{
		InputMan.EnableActiveUI();
		lineFinished = true;
	}
	else if (line.Contains("PreloadTexturesInDirectory("))
	{
		// Fetch the stuff, do the buff
		String dir = line.Tokenize("()")[1];
		List<String> files;
		int num = GetFilesInDirectory(dir, files);
		for (int i = 0; i < files.Size(); ++i)
		{
			String path = dir + "/" + files[i];
			Texture * tex = TexMan.LoadTexture(path);
			Graphics.QueueMessage(new GMBufferTexture(tex));
		}
		lineFinished = true;
	}
	else if (line.Contains("Begin("))
	{
		String stateBeginning = line.Tokenize("()")[1];
		if (stateBeginning == "Cutscene")
		{
			BeginCutscene();
		}
		lineFinished = true;
	}
	else if (line.Contains("End("))
	{
		String stateEnding = line.Tokenize("()")[1];
		if (stateEnding == "Cutscene")
		{
			EndCutscene();
		}
		lineFinished = true;
	}
	else if (line.Contains("EndScript"))
	{
		// End it.
		scriptState = ENDING;
	}
	else if (line.Contains("EnterGameState("))
	{
		String name = line.Tokenize("()")[1];		
		StateChanger * changer = new StateChanger(line, this);
		ScriptMan.PlayScript(changer);
	}
	else if (line.Contains("FadeTo(") || line.Contains("FadeIn("))
	{
		FadeInEffect * fade = new FadeInEffect(line, this);
		ScriptMan.PlayScript(fade);
	}
	else if (line.Contains("FadeInBackground("))
	{
		FadeInBackground * fade = new FadeInBackground(line, this);
		ScriptMan.PlayScript(fade);
	}
	else if (line.Contains("FadeOutBackground("))
	{
		FadeOutBackground * fade = new FadeOutBackground(line, this);
		ScriptMan.PlayScript(fade);
	}
	else if (line.Contains("FadeOut"))
	{
		FadeOutEffect * fade = new FadeOutEffect(line, this);
		ScriptMan.PlayScript(fade);
	}
	else if (line.Contains("FadeText("))
	{
		FadeTextEffect * text = new FadeTextEffect(line, this);
		ScriptMan.PlayScript(text);
		lineFinished = true;
	}
	else if (line.Contains("PlaySong("))
	{
		// Just play it.
		String song = line.Tokenize("()")[1];
		TrackMan.PlayTrack(song);
		// Line finished straight away.
		lineFinished = true;
	}
	else if (line.Contains("Dialogue")){
		/// If raw string, output it straight away! (should later be queued to some kind of dialogue-manager?)
		if (line.Contains("\"")){
			/// Create dialogue UI and append it to the current UI!
			String text = line.Tokenize("\"")[1];
			std::cout<<"\n"<<text;
			UIButton * dialogue = new UIButton("Dialogue");
			dialogue->exitable = false;
			dialogue->text = text;
			dialogue->activationMessage = "PopFromStack(this)&Remove(this)&ContinueEvent("+this->name+")";
			dialogue->textureSource = DEFAULT_TEXTURE_SOURCE;
			dialogue->textSizeRatio = DEFAULT_TEXT_SIZE_RATIO;
			dialogue->sizeRatioY = 0.3f;
			dialogue->alignmentY = 0.15f;
			dialogue->state |= UIState::DIALOGUE;  // Flag the dialogue-state flag to signify importance!
			Graphics.QueueMessage(new GMAddUI(dialogue, "root"));
			Graphics.QueueMessage(new GMPushUI("Dialogue", ActiveUI()));
		}
		/// If no quotes, load the specified dialogue-file and begin processing that instead, waiting until it is finished.!
		else {
			/// Give the npc a dialogue?
		//	assert(false);
			// Send it tot he state too, to attach to the appropriate thingymajig.
			Message * message = new Message(line);
			/// Set this event as
			message->scriptOrigin = this;
			MesMan.QueueMessage(message);
			/// Instant thingies.
			lineFinished = true;
		}
	}
	else if (line.Contains("Answer")){
		///  Go to EndAnswers..!
		lineFinished = true;
		for (int i = currentLine; i < lines.Size(); ++i){
			String line = lines[i];
			if (line.Contains("EndAnswers")){
				currentLine = i;
				lineFinished = true;
				return;
			}
		}
		assert(false && "No EndAnswers found? No good, jaow ;___;");
	}
	else if (line.Contains("BeginAlternatives") || line.Contains("BeginQuestion")){
		/// Create dialogue UI and append it to the current UI!
		String text = line.Tokenize("\"")[1];
		std::cout<<"\n"<<text;
		UIElement * dialogue = new UIElement();
		dialogue->exitable = false;
		dialogue->name = "AlternativesDialogue";
	//	dialogue->activationMessage = "Remove(this)&ContinueEvent("+this->name+")";
		dialogue->textureSource = DEFAULT_TEXTURE_SOURCE;
		dialogue->sizeRatioY = 0.3f;
		dialogue->alignmentY = 0.15f;
		dialogue->state |= UIState::DIALOGUE;  // Flag the dialogue-state flag to signify importance!

		UILabel * dialogueText = new UILabel();
		dialogueText->text = text;
		dialogueText->textSizeRatio = DEFAULT_TEXT_SIZE_RATIO;
		dialogueText->sizeRatioX = 0.5f;
		dialogueText->alignmentX = 0.25f;
		dialogue->AddChild(dialogueText);

		UIList * dialogueAnswerList = new UIList();
		dialogueAnswerList->sizeRatioX = 0.5f;
		dialogueAnswerList->alignmentX = 0.75f;
		dialogue->AddChild(dialogueAnswerList);

		int answers = 0;
		List<UIElement*> answerList;
		// Parse and add answers
		for (int i = currentLine+1; i < lines.Size(); ++i){
			String l = lines[i];
			l.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			List<String> tokens = l.Tokenize(" ");
			String token1 = tokens[0];
			token1.SetComparisonMode(String::NOT_CASE_SENSITIVE);

			if (token1 == "text"){
				l.Remove(token1);
				dialogueText->text = l;
				dialogueText->text.RemoveInitialWhitespaces();
				dialogueText->text.Remove("\"");
				dialogueText->text.Remove("\"");
			}
			else if (l.Contains("Answer")){
				++answers;
				UIButton * answerButton = new UIButton();
				answerButton->name = token1;
				l.Remove("Answer");
				l.RemoveInitialWhitespaces();
				l.Remove("\"");
				l.Remove("\"");
				answerButton->textureSource = DEFAULT_TEXTURE_SOURCE;
				answerButton->text = l;
				answerButton->sizeRatioY = 0.2f;
				answerButton->activationMessage = "ActivateDialogueAlternative("+name+","+answerButton->name+")&PopFromStack("+dialogue->name+")&Remove("+dialogue->name+")";
				answerList.Add(answerButton);
			}
			else if (l.Contains("EndAlternatives")){
				// Donelir. o-o
				break;
			}
			else {
				assert(false && "Bad line! Should only be Answer before EndAlternatives!");
			}
		}
		assert(answers);
		float sizeRatioY = 0.95f / answers;
		for (int i = 0; i < answers; ++i){
			UIElement * ans = answerList[i];
		//	ans->sizeRatioY = sizeRatioY; // Stupid to set the sizeRatioY to be this dynamic, yo.
			dialogueAnswerList->AddChild(ans);
		}
		isInAlternativeDialogue = true;
		Graphics.QueueMessage(new GMAddUI(dialogue, "root"));
		Graphics.QueueMessage(new GMPushUI(dialogue, ActiveUI()));
	}
	else if (line.Contains("elsif") || line.Contains("elseif") || line.Contains("else if"))
	{
		HandleConditional(line);
	}
	else if (line.Contains("if(") || line.Contains("if ("))
	{
		// Add to stack.
		stack.AddItem(ScriptLevel(ScriptLevel::IF_CLAUSE, currentLine));
		HandleConditional(line);
	}
	else if (line.Contains("else"))
	{
//		if (ifProcessed)
			JumpToEndif();
		lineFinished = true;
		return;
	}
	else if (line.Contains("endif"))
	{
		ScriptLevel sl = stack.Last();
		assert(sl.type == ScriptLevel::IF_CLAUSE);
		stack.RemoveLast();
		lineFinished = true;
	}
	else if (line.Contains("endwhile"))
	{
		// Go to start!
		ScriptLevel sl = stack.Last();
		assert(sl.type == ScriptLevel::WHILE_LOOP);
		currentLine = sl.evaluatedAtLine;
		String startLine = lines[currentLine];
		HandleConditional(startLine);
//		lineFinished = true;
		// Evaluate?
//		stack.RemoveLast();
	}
	else if (line.Contains("while"))
	{
		stack.AddItem(ScriptLevel(ScriptLevel::WHILE_LOOP, currentLine));
		HandleConditional(line);
	}
	else if (line.Contains("CreateInt")){
		List<String> tokens = line.Tokenize(" \t");
		String varName = tokens[1];
		int initialValue = 0;
		if (tokens.Size() >= 3)
			initialValue = tokens[2].ParseInt();
		if (!GameVars.Get(varName)){
			GameVars.CreateInt(varName, initialValue);
		}
		lineFinished = true;
	}
	else if (line.Contains("SetInt ")){
		List<String> tokens = line.Tokenize(" \t");
		String varName = tokens[1];
		int value = tokens[2].ParseInt();
		GameVars.SetInt(varName, value);
		lineFinished = true;
	}
	else if (line.Contains("Repeatable")){
		/// Flag the event as repeatable.
		repeatable = true;
		lineFinished = true;
	}
	// Consider just making an else-clause for all remaining events to be processed by the specific game instead?
	else if (
		line.Contains("SpawnEntity") ||
		line.Contains("OnApproach") ||
		line.Contains("OnInteract") ||
		line.Contains("DisableMovement") ||
		line.Contains("EnableMovement") ||
		line.Contains("Zone(") ||
		line.Contains("PlacePlayer(") ||
		line.Contains("TrackPlayer")
		)
	{
		Message * message = new Message(line);
		/// Set this event as
		message->scriptOrigin = this;
		MesMan.QueueMessage(message);
		/// Instant thingies.
		lineFinished = true;
	}
	else {
//		std::cout<<"\nUndefined event command: "<<line;
//		std::cout<<"\nPassing it as a custom command to the game states for further processing.";
		Message * message = new Message(line);
		/// Set this event as source of it.
		message->scriptOrigin = this;
		MesMan.QueueMessage(message);
		lineFinished = true;
	//	assert(false && "Undefined event command!");
	};
}

/// Continue to dat alternative (for branching with Alternatives/Questions)
bool Script::ContinueToAlternative(String alternative){
	/// Check that we're currently in an alternativeDialogue?
	if (!isInAlternativeDialogue){
		std::cout<<"\nNot in any alternative dialogue anymore. Ignoring command.";
		return false;
	}
	bool endAlternativesFound = false;
	for (int i = currentLine; i < lines.Size(); ++i){
		String line = lines[i];
		if (line.Contains("EndAlternatives"))
			endAlternativesFound = true;
		if (!endAlternativesFound)
			continue;
		if (line.Contains(alternative)){
			currentLine = i;
			lineFinished = true;
			isInAlternativeDialogue = false;
			return true;
		}
	}
	return false;
}


#define EVENT_DEFAULT	0x00000001

/// File I/O. Reading will reset data.
bool Script::WriteTo(std::fstream & file){
	/// Default
	int fileIOFlags = EVENT_DEFAULT;
	file.write((char*)&fileIOFlags, sizeof(int));
	/// Write name, source and position.
	name.WriteTo(file);
	source.WriteTo(file);
	file.write((char*)&position, sizeof(Vector3f));
	return true;
}

bool Script::ReadFrom(std::fstream & file){
	/// Default
	int fileIOFlags;
	file.read((char*)&fileIOFlags, sizeof(int));
	assert(fileIOFlags == EVENT_DEFAULT);
	/// Read name, source and position.
	name.ReadFrom(file);
	source.ReadFrom(file);
	file.read((char*)&position, sizeof(Vector3f));
	loaded = true;

	/// Clear previous lines and reset variables.
	lines.Clear();
	Reset();
	return true;
}


/// Sets the flag to delete the event once it's finished.
void Script::SetDeleteOnEnd(bool value){
	if (value)
		flags |= DELETE_WHEN_ENDED;
	else
		flags &= ~DELETE_WHEN_ENDED;
}


/// For if- and elsif- statements.
void Script::HandleConditional(String line)
{
	// Remove first part until (
	int index = line.Find('(');
	line = line.Part(index);
	/// Use expressions from the MathLib. First parse for functions to provide their values? Or...
	Expression exp;
	List<Variable> allVars = GameVars.GetAllExpressionVariables() + variables;
	exp.functionEvaluators = functionEvaluators; // Set evaluators.
	bool parseOK = exp.ParseExpression(line);
	if (!parseOK)
	{
		std::cout<<"\nParse error in expression "<<line;
		return;
	}
	ExpressionResult res = exp.Evaluate(allVars);
	bool statementTrue = res.GetBool();
	/// If statement is true, sign this row as finished.
	if (statementTrue)
	{
		// Set line finished to true so the actual content will be processed.
		lineFinished = true;
		// Set if-Processed to true so that no elsif- or else- clause will be handled.
		ScriptLevel & latest = stack.Last();
		latest.evaluatedAtLine = currentLine;
	}
	else 
	{
		// Check stuff.
		ScriptLevel & sl = stack.Last();
		int newRow = -1;
		// If the statement is not true, find an else or endif block..!
		for (int i = currentLine+1; i < lines.Size(); ++i)
		{
			String l = lines[i];
			if (sl.type == ScriptLevel::WHILE_LOOP)
			{
				if (l.Contains("endwhile"))
				{
					// Jump to next after, as regular stopping on endwhile will reboot the loop
					newRow = i + 1;
					stack.RemoveLast();
					break; 
				}
			}
			if (sl.type == ScriptLevel::IF_CLAUSE)
			{
				if (l.Contains("elsif") || l.Contains("else") || l.Contains("endif"))
				{
					newRow = i; break;
				}
			}
		}
		assert(newRow > 0);
		// Process it next iteration.
		currentLine = newRow;
		lineProcessed = false;
		lineFinished = false;
		return;

	}
	if (lineFinished == false)
		assert(false && "Line not finished? Something is missing in the if/else/endif block!");	
}

void Script::JumpToNextConditional()
{
	// If the statement is not true, find an else or endif block..!
	for (int i = currentLine; i < lines.Size(); ++i){
		String l = lines[i];
		if (l.Contains("elsif"))
		{
			currentLine = i;
			// Call this function again?
			EvaluateLine(lines[i]);
			return;
		}
		else if (l.Contains("else"))
		{
			/// Jump to this row ^^
			currentLine = i;
			lineFinished = true;
			return;
		}
		else if (l.Contains("endif")){
			currentLine = i;
			lineFinished = true;
			return;
		}
	}
}


void Script::JumpToEndif()
{
	// If the statement is not true, find an else or endif block..!
	for (int i = currentLine; i < lines.Size(); ++i){
		String l = lines[i];
		if (l.Contains("endif")){
			currentLine = i;
			lineFinished = true;
			return;
		}
	}
	std::cout<<"\nERROR: No Endif found D:";
}



