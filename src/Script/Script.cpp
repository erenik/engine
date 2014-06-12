// Emil Hedemalm
// 2013-07-16

#include "Script.h"
#include <fstream>
#include "UI/UIButtons.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "UI/UIList.h"
#include "Game/GameVariables.h"
#include <cstring>
#include "ScriptManager.h"
#include "GeneralScripts.h"
#include "Audio/TrackManager.h"
#include "Input/InputManager.h"
#include "File/FileUtil.h"
#include "TextureManager.h"

/// Compact saveable version of the event
//struct CompactEvent{};

const char * Script::rootEventDir = "data/scripts/";

Script::Script(const Script & base)
{
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
	parentScript = NULL;
	timePassed = 0; // Should be added each process-frame.
	pausesExecution = base.pausesExecution;
}

Script::Script(String name, Script * parent /* = NULL */ )
: name(name), parent(parent)
{
	name = "Test";
	source = "Test.e";
	triggerCondition = NULL_TRIGGER_TYPE;
	loaded = false;
	// Assume child scripts are inherent C++ classes which do not require further loading.
	if (parent){
		loaded = true;
	}
	executed = false;
	repeatable = false;
	currentLine = 0;
	scriptState = NOT_BEGUN;
	flags = 0;
	timePassed = 0;
	// Set this to false for those scripts that do not require waiting for completion! o.o
	pausesExecution = true;
	inCutscene = false;
	lineFinished = false;
	lineProcessed = false;
	uiDisabled = false;
	paused = false;

	if (parent)
	{
		parent->childScripts.Add(this);
	}
};

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
	std::cout<<"\nScript::Load called: "<<name;
	bool result = Load(name);
	if (!result)
	{
		std::cout<<"Was unable to load target script.";
		loaded = false;
		/// Quit the script ASAP since it failed to load anyway.
		scriptState = FINISHING;
	}
	else
	{
		loaded = true;
	}
	return result;
}

/// Wosh o.o, NOTE that the root dir will be appended at the start automatically!
bool Script::Load(String fromFile){
	std::cout<<"\nEvent::Load fromFile: "<<fromFile;
	/// Already loaded, reset and re-load!
	if (loaded){
		std::cout<<"\nAlready loaded, resetting.";
		loaded = false;
		Reset();
		lines.Clear();
	}
	std::cout<<"\nSaving source path...";
	String source = fromFile;
	/// Parsley parse, yes?
	/// Add root event dir if not already included (could be)
	/// Assure that at least the data/ dir is included. Any path with it can be assumed to be completely relative!
	if (!source.Contains(rootEventDir) && !source.Contains("data/"))
		source = rootEventDir + source;

	std::cout<<"\nTrying to open file...";
	std::fstream file;
	file.open(source.c_str(), std::ios_base::in);
	if (!file.is_open()){
//		assert(file.is_open() && "ERROR opening file stream in Script::Load(fromFile)!");
		std::cout<<"\nERROR: Unable to open file stream to "<<source;
		file.close();
		return NULL;
	}

	int start  = (int) file.tellg();
	std::cout<<"\nStart: "<<start;
	file.seekg( 0, std::ios::end );
	int fileSize = (int) file.tellg();
	std::cout<<"\nEvent::Load: fileSize: "<<fileSize;
	if (fileSize <= 0){
		std::cout<<"\nInvalid file size, returning.";
		return false;
	}
	char * data = new char [fileSize];
	memset(data, 0, fileSize);
	file.seekg( 0, std::ios::beg);
	file.read((char*) data, fileSize);
	file.close();
	String fileContents(data);
	delete[] data; data = NULL;
	int loadingType = 0;
	bool midComment = false;
	List<String> sourceLines = fileContents.GetLines();
	for (int i = 0; i < sourceLines.Size(); ++i){
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
		else if (line.Contains("TriggerCondition")){
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
	return true;
}

/// Regular state-machine mechanics for the events, since there might be several parralell events?
void Script::OnBegin()
{
	scriptState = BEGUN;
	currentLine = 0;
	lineProcessed = false;
}

void Script::Process(long long timeInMs)
{
	if (paused)
		return;
	if (currentLine < 0 || currentLine >= lines.Size())
	{
		std::cout<<"\nScript::Process line out of scope. Aborting";
		scriptState = ENDING;
		return;
	}
	String line = lines[currentLine];
	timePassed += (int)timeInMs;

	if (!lineProcessed)
	{
		/// Evaluate the current line, look if we have to check for line-finishing conditions or not.
		lineFinished = false;
		EvaluateLine(lines[currentLine]);
		lineProcessed = true;
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
		Input.EnableActiveUI();

	if (repeatable){
		scriptState = NOT_BEGUN;
		currentLine = -1;
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
	line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	// "80Gray50Alpha.png"
#define DEFAULT_TEXTURE_SOURCE	"black50Alpha.png"
#define DEFAULT_TEXT_SIZE_RATIO	0.3f
	
	/// Some state began, take not of it?
	if (line == "DisableActiveUI")
	{
		Input.DisableActiveUI();
		lineFinished = true;
		uiDisabled = true;
	}
	else if (line == "EnableActiveUI")
	{
		Input.EnableActiveUI();
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
	else if (line.Contains("Wait"))
	{
		// Wait..! ..
		ScriptMan.PlayScript(new WaitScript(line, this));
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
			Graphics.QueueMessage(new GMPushUI("Dialogue", Graphics.GetUI()));
		}
		/// If no quotes, load the specified dialogue-file and begin processing that instead, waiting until it is finished.!
		else {
			/// Give the npc a dialogue?
		//	assert(false);
			// Send it tot he state too, to attach to the appropriate thingymajig.
			Message * message = new Message(line);
			/// Set this event as
			message->event = this;
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
		Graphics.QueueMessage(new GMPushUI(dialogue, Graphics.GetUI()));
	}
	else if (line.Contains("elsif") || line.Contains("elseif") || line.Contains("else if"))
	{
		HandleConditional(line);
	}
	else if (line.Contains("if(") || line.Contains("if ("))
	{
		ifProcessed = false;
		HandleConditional(line);
	}
	else if (line.Contains("else"))
	{
		if (ifProcessed)
			JumpToEndif();
		lineFinished = true;
		return;
	}
	else if (line.Contains("endif")){
		lineFinished = true;
		ifProcessed = false;
	}
	else if (line.Contains("CreateInt")){
		List<String> tokens = line.Tokenize(" \t");
		String varName = tokens[1];
		int initialValue = 0;
		if (tokens.Size() >= 3)
			initialValue = tokens[2].ParseInt();
		if (!GameVars.Get(varName)){
			GameVars.CreateInt(varName,initialValue);
		}
		lineFinished = true;
	}
	else if (line.Contains("SetInt")){
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
		message->event = this;
		MesMan.QueueMessage(message);
		/// Instant thingies.
		lineFinished = true;
	}
	else {
		std::cout<<"\nUndefined event command: "<<line;
		std::cout<<"\nPassing it as a custom command to the game states for further processing.";
		Message * message = new Message(line);
		/// Set this event as
		message->event = this;
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
	if (ifProcessed)
	{
		// Continue to the Endif
		JumpToEndif();
		return;
	}

	/// Evaluate if-clauses!
	List<String> tokens = line.Tokenize(" ()");
	String varName = tokens[1];
	String comparisonOperator = tokens[2];
	String comparisonValue = tokens[3];
	GameVariable * gv = GameVars.Get(varName);
	/// If not defined, assume it auto-fails the check
	if (!gv)
	{
		JumpToNextConditional();
		return;
	}
	assert(gv && "No game variable with given name?");
	int intValue, intComparisonValue;
	bool statementTrue = false;;
	switch(gv->Type()){
		case GameVariable::INTEGER:
			intValue = ((GameVariablei*)gv)->Get();
			intComparisonValue = comparisonValue.ParseInt();
			if (comparisonOperator == ">"){
				if (intValue > intComparisonValue)
					statementTrue = true;
			}
			else if (comparisonOperator == "<"){
				if (intValue < intComparisonValue)
					statementTrue = true;
			}
			else if (comparisonOperator == "=="){
				if (intValue == intComparisonValue)
					statementTrue = true;
			}
			else {
				assert(false && "Invalid comparison operator! D:");
				lineFinished = true;
				return;
			}
			break;
		default:
			assert(false && "Implement other GameVariable checks, prugrumur!");
			break;
	}
	/// If statement is true, sign this row as finished.
	if (statementTrue){
		// Set line finished to true so the actual content will be processed.
		lineFinished = true;
		// Set if-Processed to true so that no elsif- or else- clause will be handled.
		ifProcessed = true;
	}
	else {
		// If the statement is not true, find an else or endif block..!
		for (int i = currentLine+1; i < lines.Size(); ++i){
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



