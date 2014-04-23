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
	state = base.state;
	lineFinished = base.lineFinished;
	flags = base.flags;
	/// Copy loaded data too.
	lines = base.lines;
	parentScript = NULL;
}

Script::Script(String name, Script * parent /* = NULL */ )
: name(name), parent(parent)
{
	name = "Test";
	source = "Test.e";
	triggerCondition = NULL_TRIGGER_TYPE;
	loaded = false;
	// Assume child scripts are inherent C++ classes which do not require further loading.
	if (parent)
		loaded = true;
	executed = false;
	repeatable = false;
	currentLine = -1;
	state = NOT_BEGUN;
	lineFinished = true;
	flags = 0;
};

void Script::Reset(){
	currentLine = -1;
	state = NOT_BEGUN;
	lineFinished = true;
	executed = false;
}

/// Loads script using given name as reference to source-file.
bool Script::Load()
{
	std::cout<<"\nScript::Load called: "<<name;
	bool result = Load(name);
	assert(result && "Was unable to load target script.");
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
		assert(file.is_open() && "ERROR opening file stream in Script::Load(fromFile)!");
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
	List<String> sourceLines = fileContents.GetLines();
	for (int i = 0; i < sourceLines.Size(); ++i){
		String & line = sourceLines[i];
		// Try load the battler from the relative directory.
		if (line.Contains("//"))
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
void Script::OnBegin(){
	state = BEGUN;
}

void Script::Process(float time)
{
	String line = lines[currentLine];
	/// Check if the current line is finished? If not, wait until it is finished.
	if (!lineFinished)
		return;

	/// If it is finished, check for next line
	if (lineFinished){
		++currentLine;
		/// If we finished the last line, flag this event as ending...!
		if (currentLine >= lines.Size()){
			state = ENDING;
			return;
		}
		/// New line: specify it as not finished.!
		lineFinished = false;
	}

	/// Evaluate the current line, look if we have to check for line-finishing conditions or not.
	EvaluateLine(lines[currentLine]);
}

void Script::OnEnd()
{
	if (repeatable){
		state = NOT_BEGUN;
		currentLine = -1;
		return;
	}
	/// Flag the event as finished!
	state = ENDED;
	/// Notify parents if needed.
	if (parent)
		parent->OnScriptEnded(this);
}

/// o.o
void Script::OnScriptEnded(Script * childScript)
{
	// Mark line as finished executing.
	lineFinished = true;
}


void Script::EvaluateLine(String & line)
{
	line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	// "80Gray50Alpha.png"
#define DEFAULT_TEXTURE_SOURCE	"black50Alpha.png"
#define DEFAULT_TEXT_SIZE_RATIO	0.3f
	
	if (line.Contains("EnterGameState("))
	{
		String stateName = line.Tokenize("()")[1];		
		StateChanger * changer = new StateChanger(line, this);
		ScriptMan.PlayScript(changer);
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
			Graphics.QueueMessage(new GMPushUI("Dialogue"));
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
		Graphics.QueueMessage(new GMPushUI(dialogue));
	}
	else if (line.Contains("if(") || line.Contains("if (")){
		/// Evaluate if-clauses!
		List<String> tokens = line.Tokenize(" ()");
		String varName = tokens[1];
		String comparisonOperator = tokens[2];
		String comparisonValue = tokens[3];
		GameVariable * gv = GameVars.Get(varName);
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
			lineFinished = true;
		}
		else {
			// If the statement is not true, find an else or endif block..!
			for (int i = currentLine; i < lines.Size(); ++i){
				String l = lines[i];
				if (l.Contains("else")){
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
	else if (line.Contains("else") || line.Contains("endif")){
		for (int i = currentLine; i < lines.Size(); ++i){
			String l = lines[i];
			List<String> tokens = l.Tokenize(" \t");
			String token1 = tokens[0];
			if (l.Contains("endif")){
				currentLine = i;
				lineFinished = true;
				return;
			}
		}
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
		std::cout<<"\nERROR: Undefined event command: "<<line;
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

