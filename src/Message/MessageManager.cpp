#include "Message/MessageManager.h"

//#include "IO.h"

#include "Message/Message.h"
#include "Message/VectorMessage.h"
#include "FileEvent.h"
#include "Network/Packet/Packet.h"
#include <Mutex/Mutex.h>

#include "StateManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Input/InputManager.h"
#include "Script/Script.h"
#include "Maps/MapManager.h"
#include "Script/ScriptManager.h"

#include "UI/UITypes.h"
#include "UI/UIFileBrowser.h"
#include "UI/UIQueryDialogue.h"
#include "UI/UIVideo.h"
#include "Network/NetworkManager.h"

MessageManager * MessageManager::messageManager = NULL;

void MessageManager::Allocate(){
	assert(messageManager == NULL);
	messageManager = new MessageManager();
}
MessageManager * MessageManager::Instance(){
	assert(messageManager);
	return messageManager;
}
void MessageManager::Deallocate(){
	assert(messageManager);
	delete(messageManager);
	messageManager = NULL;
}


MessageManager::MessageManager(){
	// Create two mutexes
	msgQueueMutex.Create("msgQueueMutex");
	packetQueueMutex.Create("packetQueueMutex");
}
MessageManager::~MessageManager(){
	while(!messageQueue.isOff())
		delete messageQueue.Pop();
#ifdef USE_NETWORK
	while(!packetQueue.isOff())
		delete packetQueue.Pop();
#endif
    msgQueueMutex.Destroy();
    packetQueueMutex.Destroy();
}

void MessageManager::ProcessPackets(){
#ifdef USE_NETWORK
    packetQueueMutex.Claim(-1);
	// Process
	while (packetQueue.Length())
	{
		Packet * packet = packetQueue.Pop();
		ProcessPacket(packet);
		delete packet;
	}
	packetQueueMutex.Release();
#endif
}

void MessageManager::ProcessMessages()
{
	/// Fetch available messages, then process them later, or lese mutexes will deadlock if trying to process the messages while locked with e.g. the graphics thread.
    msgQueueMutex.Claim(-1);
	Queue<Message*> messageQueueCopy;
	// Clear it.
	while(messageQueue.Length()){
		messageQueueCopy.Push(messageQueue.Pop());
	}
	/// Fetch all delayed messages too.
	long long cTime = Timer::GetCurrentTimeMs();
	for (int i = 0; i < delayedMessages.Size(); ++i){
		Message * mes = delayedMessages[i];
		if (mes->timeToProcess < cTime){
			delayedMessages.Remove(mes);
			messageQueueCopy.Push(mes);
			--i;
			continue;
		}
	}

	msgQueueMutex.Release();

	// And then process them.
	while (messageQueueCopy.Length()){
		Message * message = messageQueueCopy.Pop();
		ProcessMessage(message);
	//	assert(message->data == NULL && "OY!");
		delete message;
	}
}

/// For them delayed messages that require special treatment.. :P
bool MessageManager::QueueDelayedMessage(Message * message)
{
	msgQueueMutex.Claim(-1);
	delayedMessages.Add(message);
	msgQueueMutex.Release();
	return true;
}

/// Queues a bunch of string-based messages in the form "Message1&Message2&Message3&..."
bool MessageManager::QueueMessages(String messages, UIElement * elementThatTriggeredIt){
	msgQueueMutex.Claim(-1);	
	try{
		List<String> messageList = messages.Tokenize("&");
		for (int i = 0; i < messageList.Size(); ++i){
			String message = messageList[i];
			Message * msg = new Message(message);
			msg->element = elementThatTriggeredIt;
			messageQueue.Push(msg);
		}
	}
	catch(...){
		std::cout<<"\nFailed to push message!";
		assert(false && "MessageManager::QueueMessage");
		return false;
	}
	msgQueueMutex.Release();
	return true;
}

bool MessageManager::QueueMessage(Message* msg){
	msgQueueMutex.Claim(-1);
    try{
		messageQueue.Push(msg);
	}
	catch(...){
		std::cout<<"\nFailed to push message!";
		assert(false && "MessageManager::QueueMessage");
		return false;
	}
	msgQueueMutex.Release();
	return true;
}
bool MessageManager::QueueMessage(int msg){
    Message * m = new Message(msg);
	return QueueMessage(m);
}
bool MessageManager::QueuePacket(Packet *packet){
	try{
		packetQueue.Push(packet);
	}
	catch(...){
		std::cout<<"\nFailed to push packet!";
		return false;
	}
	return true;
}

/// Queues a list of packets for future processing by relevant game and entity states.
bool MessageManager::QueuePackets(List<Packet*> packets){
	try {
		for (int i = 0; i < packets.Size(); ++i){
			packetQueue.Push(packets[i]);
		}
	} catch (...){
		std::cout<<"\nFailed to push packet!";
		return false;
	}
	return true;
}

void MessageManager::ProcessPacket(Packet * packet)
{
	// First send it to global state
	if (StateMan.GlobalState())
		StateMan.GlobalState()->ProcessPacket(packet);
	// Send it firstly to the state
	if (StateMan.ActiveState())
		StateMan.ActiveState()->ProcessPacket(packet);
}
void MessageManager::ProcessMessage(Message * message){
	// Check for UI-messages first.
	String msg = message->msg;
	// Do note that not all messages uses the string-argument...
//	if (!msg.Length())
//		return;
	msg.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (msg.Contains("NavigateUI(")){
		bool toggle = msg.Tokenize("()")[1].ParseBool();
		Input.NavigateUI(toggle);
		return;
	}
	else if (msg.Contains("INTERPRET_CONSOLE_COMMAND(this)"))
	{
		String command = message->element->text;
		Message * newMes = new Message(MessageType::CONSOLE_COMMAND);
		newMes->msg = command;
		MesMan.QueueMessage(newMes);	
		return;
	}
	else if (msg == "TogglePause(this)"){
		UIElement * e = message->element;
		if (e->type != UIType::VIDEO)
			return;
		UIVideo * video = (UIVideo*) e;
		video->TogglePause();

	}
	else if (msg.Contains("UIProceed("))
	{
		String name = msg.Tokenize("()")[1];
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * e = ui->GetElementByName(name);
		if (!e)
			return;
		e->Proceed();
		return;
	}
	else if (msg.Contains("UITextureInput("))
	{
		String name = msg.Tokenize("()")[1];
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * e = ui->GetElementByName(name);
		if (e->type != UIType::TEXTURE_INPUT)
			return;
		UITextureInput * ti = (UITextureInput*) e;
		TextureMessage * m = new TextureMessage(ti->action, ti->GetTextureSource());
		MesMan.QueueMessage(m);
		return;	
	}
	else if (msg.Contains("UIStringInput("))
	{
		String name = msg.Tokenize("()")[1];
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * e = ui->GetElementByName(name);
		if (e->type != UIType::STRING_INPUT)
			return;
		UIStringInput * si = (UIStringInput*)e;
		SetStringMessage * m = new SetStringMessage(si->action, si->GetValue());
		MesMan.QueueMessage(m);
		return;
	}
	else if (msg.Contains("UIFloatInput("))
	{
		String name = msg.Tokenize("()")[1];
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * e = ui->GetElementByName(name);
		if (e->type != UIType::FLOAT_INPUT)
			return;
		UIFloatInput * fi = (UIFloatInput*)e;
		FloatMessage * m = new FloatMessage(fi->action, fi->GetValue());
		MesMan.QueueMessage(m);
		return;
	}
	else if (msg.Contains("UIIntegerInput("))
	{
		String name = msg.Tokenize("()")[1];
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * e = ui->GetElementByName(name);
		if (e->type != UIType::INTEGER_INPUT)
			return;
		UIIntegerInput * ii = (UIIntegerInput*)e;
		IntegerMessage * m = new IntegerMessage(ii->action, ii->GetValue());
		MesMan.QueueMessage(m);
		return;
	}
	else if (msg.Contains("UIVectorInput("))
	{
		String name = msg.Tokenize("()")[1];
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * e = ui->GetElementByName(name);
		if (e->type != UIType::VECTOR_INPUT)
			return;
		UIVectorInput * vi = (UIVectorInput*)e;
		/// Fetch vector data from the input first.
		VectorMessage * m = NULL;
		switch(vi->numInputs)
		{
			case 2:
				m = new VectorMessage(vi->action, vi->GetValue2i());
				break;
			case 3:
				m = new VectorMessage(vi->action, vi->GetValue3f());
				break;
			case 4:
				m = new VectorMessage(vi->action, vi->GetValue4f());
				break;
			default:
				assert(false && "implement");
				break;
		}
		if (m)
			MesMan.QueueMessage(m);
		return;
	}
	else if (msg.Contains("setVisibility")){
		List<String> params = msg.Tokenize("(),");
		assert(params.Size() >= 2 && "Invalid amount of arguments to SetVisibility UI command!");

		String uiName = params[1];
		/// Check if the name-parameter is 'this'
		if (uiName == "this"){
			// std::cout<<"\nElement with 'this' lacking name.. fix yo.";
			uiName = message->element->name;
		}
		bool visibility = params[2].ParseBool();
		Graphics.QueueMessage(new GMSetUIb(uiName, GMUI::VISIBILITY, visibility));
		return;
	}
	else if (msg.Contains("SetText(")){
		List<String> params = msg.Tokenize("(),");
		assert(params.Size() >= 2 && "Invalid amount of arguments to SetVisibility UI command!");

		String uiName = params[1];
		/// Check if the name-parameter is 'this'
		if (uiName == "this"){
			// std::cout<<"\nElement with 'this' lacking name.. fix yo.";
			uiName = message->element->name;
		}
		String text = params[2];
		text.Remove("\"", true);
		Graphics.QueueMessage(new GMSetUIs(uiName, GMUI::TEXT, text));
		return;
	}
	else if (msg.Contains("CyclicUIY(")){
		Input.cyclicY = msg.Tokenize("()")[1].ParseBool();
		return;
	}
	else if (msg.Contains("Query(")){
		// Create a new UI to place on top of it all!
		String uiName = msg;
		List<String> params = msg.Tokenize("(),");
		assert(params.Size() >= 2);
		String action = params[1];
		/// Create the dialogue
		UIQueryDialogue * dialog = new UIQueryDialogue(action, action);
		dialog->navigateUIOnPush = true;
		dialog->exitable = true;
		if (params.Size() >= 4){
			dialog->headerText = params[2];
			dialog->textToPresent = params[3];
		}
		dialog->CreateChildren();
		/// Add the dialogue to the global UI
		Graphics.QueueMessage(new GMAddGlobalUI(dialog, "root"));
		/// Push it to the top... should not be needed with the global ui.
		Graphics.QueueMessage(new GMPushUI(dialog, Graphics.GetGlobalUI(true)));
		return;
	}
	else if (msg.Contains("SetFileBrowserDirectory(")){
		List<String> params = msg.Tokenize(",()");
		String uiName = params[1];
		UIElement * ui = StateMan.ActiveState()->GetUI()->GetElementByName(uiName);
		if (!ui)
			return;
		assert(ui->type == UIType::FILE_BROWSER);
		UIFileBrowser * fb = (UIFileBrowser*)ui;
		String path = params[2];
		if (path == "this")
			path = message->element->text;
		fb->SetPath(path);
		fb->LoadDirectory(true);
		return;
	}
	else if (msg.Contains("UpdateFileBrowserDirectory(")){
		List<String> params = msg.Tokenize(",()");
		String uiName = params[1];
		UIElement * ui = StateMan.ActiveState()->GetUI()->GetElementByName(uiName);
		if (!ui)
			return;
		assert(ui->type == UIType::FILE_BROWSER);
		UIFileBrowser * fb = (UIFileBrowser*)ui;
		String path = params[2];
		if (path == "this")
			path = message->element->text;
		fb->UpdatePath(path);
		fb->LoadDirectory(true);
		return;
	}
	else if (msg.Contains("EvaluateFileBrowserSelection("))
	{
		List<String> params = msg.Tokenize("()");
		String uiName = params[1];
		UIElement * ui = StateMan.ActiveState()->GetUI()->GetElementByName(uiName);
		if (!ui)
			return;
		FileEvent * message = new FileEvent();
		UIFileBrowser * fb = (UIFileBrowser*)ui;
		message->msg = fb->action;
		message->files = fb->GetFileSelection();
		// Queue the new message.
		QueueMessage(message);
		return;
	}
	else if (msg.Contains("SetFileBrowserFile("))
	{
		List<String> params = msg.Tokenize(",()");
		String uiName = params[1];
		UIElement * ui = StateMan.ActiveState()->GetUI()->GetElementByName(uiName);
		if (!ui)
			return;
		assert(ui->type == UIType::FILE_BROWSER);
		UIFileBrowser * fb = (UIFileBrowser*)ui;
		String file = params[2];
		if (file == "this"){
			file = message->element->text;
		}
		fb->SetActiveFile(file);
		return;
	}
	else if (msg.Contains("OpenFileBrowser(")){
		/// Parse stuff
		List<String> params = msg.Tokenize("(),");
		assert(params.Size() >= 3);
		String title = params[1];
		title.Remove("\"", true);
		String action = params[2];
		String filter;
		if (params.Size() >= 4){
			filter = params[3];
			filter.Remove("\"", true);
		}
		/// Create the browser.
		UIFileBrowser * fileBrowser = new UIFileBrowser(title, action, filter);
		fileBrowser->CreateChildren();
		fileBrowser->LoadDirectory(false);
		/// Push it to the UI.
		Graphics.QueueMessage(new GMAddUI(fileBrowser, "root"));
		Graphics.QueueMessage(new GMPushUI(fileBrowser, Input.GetRelevantUI()));
		return;
	}
	else if (msg.Contains("QuitApplication"))
	{
		StateMan.QueueState(StateMan.GetStateByID(GameStateID::GAME_STATE_EXIT));
		return;
	}
	else if (msg.Contains("PushToStack(") || msg.Contains("PushUI(")){
		// Fetch target.
		List<String> params = msg.Tokenize("(),");
		assert(params.Size() >= 2 && "Invalid amount of arguments to PushToStack UI command!");
		if (params.Size() < 2)
			return;
		/// The user might want to enter either an element or a whole new UI here, so investigate it!
		String uiName = params[1];
		String uiSrc = uiName;
		/// Check if the element exists...!
		UserInterface * ui = Graphics.GetUI();
		if (!ui)
			return;
		UIElement * element = NULL;
		/// Check if it's a source file, if so try and load that first.
		if (uiName.Contains(".gui"))
		{
			element = ui->GetElementBySource(uiName);
			if (!element){
				// Load it.
				element = UserInterface::LoadUIAsElement(uiSrc);
				/// Return if we fail to load.
				if (!element){
					std::cout<<"\nUnable to load ui: "<<uiName;
					return;
				}
				/// If we load here, reset elementName since uiName since it probably changed.
				uiName = element->name;
				/// Make sure it is exitable.
				element->exitable = true;
				Graphics.QueueMessage(new GMAddUI(element));
			}
		}
		/// Regular name
		else {
			element = ui->GetElementByName(uiName);		
		}
		/// Push it to stack if not.
		if (element)
			Graphics.QueueMessage(new GMPushUI(element->name, ui));
		return;
	}
	else if (msg.Contains("PopFromStack(") || msg.Contains("PopUI("))
	{
		std::cout<<"\nPopFromStack/PopUI received.";
		// Fetch target.
		List<String> params = msg.Tokenize("(),");
		assert(params.Size() >= 2 && "Invalid amount of arguments to PopFromStack UI command!");
		if (params.Size() < 2){
			std::cout<<"\nToo few parameters.";
			return;
		}
		String uiName = params[1];
		/// Force pop it?
		Graphics.QueueMessage(new GMPopUI(uiName, Input.GetRelevantUI(), true));
		return;
	}
	else if (msg == "Back")
	{
		UserInterface * ui = Input.GetRelevantUI();
		UIElement * stackTop = ui->GetStackTop();
		Graphics.QueueMessage(new GMPopUI(stackTop->name, ui));
		return;
	}
	else if (msg.Contains("begin_input(") ||
		msg.Contains("BeginInput("))
	{
		String elementName = msg.Tokenize("()")[1];
		UIElement * element;
		if (elementName == "this")
			element = message->element;
		else
			element = StateMan.ActiveState()->GetUI()->GetElementByName(elementName);
		if (!element)
			return;
		assert(element->demandInputFocus);
		((UIInput*)element)->BeginInput();
		return;
		/*
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * element = message->element;
		if (!element){
			std::cout<<"\nNo active element, fetching hover element.";
			element = ui->GetHoverElement();
		}
		if (element != NULL){
			// assert(element->onTrigger);
			if (!element->onTrigger)
				std::cout<<"\nBegnning input for element without onTrigger specified!";
			Input.SetActiveUIInputElement(element);
			Input.EnterTextInputMode(element->onTrigger);
		}
		else
			assert(false && "NULL-element :<");
		return;
		*/
	}
	else if (msg.Contains("Remove(") || msg.Contains("DeleteUI("))
	{
		UserInterface * ui = StateMan.ActiveState()->GetUI();
		UIElement * element;
		String uiName = msg.Tokenize("()")[1];
		/// this-deletion
		if (uiName == "this"){
			if (message->element)
				element = message->element;
			else
				element = ui->GetActiveElement();
		}
		/// Named deletion
		else {
			element = ui->GetElementByName(uiName);
		}
		/// For all usual AI, the state is not active after activation, so just grab the one with the hover-state!
		if (element == NULL)
			element = ui->GetHoverElement();
		assert(element);
		if (element == NULL){
			std::cout<<"\nERRORRRR: Invalid hellelemend? No active hover element, yo..";
			return;
		}
		Graphics.QueueMessage(new GMRemoveUI(element));
		return;
	}
	else if (msg.Contains("ContinueEvent(")){
		List<Script*> events, mapEvents;
		Map * map = MapMan.ActiveMap();
		if (map)
			mapEvents = map->GetEvents();
		List<Script*> moreEvents = ScriptMan.GetActiveEvents();
		events += mapEvents + moreEvents;
		String targetEvent = msg.Tokenize("()")[1];
		for (int i = 0; i < events.Size(); ++i){
			Script * event = events[i];
			if (event->name == targetEvent)
				event->lineFinished = true;
		}
		return;
	}
	else if (msg.Contains("ActivateDialogueAlternative(")){
		List<Script*> events = MapMan.ActiveMap()->GetEvents();
		List<Script*> moreEvents = ScriptMan.GetActiveEvents();
		events += moreEvents;
		String argsString = msg.Tokenize("()")[1];
		List<String> args = argsString.Tokenize(" ,");
		String targetEvent = args[0];
		String alternative = args[1];
		assert(alternative);
		for (int i = 0; i < events.Size(); ++i){
			Script * e = events[i];
			if (e->name == targetEvent){
				e->ContinueToAlternative(alternative);
			}
		}
		return;
	}

	// First send it to global state
	if (StateManager::Instance()){
		if (StateMan.GlobalState())
			StateMan.GlobalState()->ProcessMessage(message);
		// Send it to the state for processing
		if (StateMan.ActiveState())
			StateMan.ActiveState()->ProcessMessage(message);
	}
}
