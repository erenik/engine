/// Emil Hedemalm
/// 2014-08-31 (although much older)
/// Message manager which takes care of glueing together most of the engine.

#include "Message/MessageManager.h"

#include "InputState.h"

//#include "IO.h"
#include "MathLib/Expression.h"
#include "Physics/Messages/CollisionCallback.h"
#include "Message/Message.h"
#include "Message/MathMessage.h"
#include "FileEvent.h"
#include "Network/Packet/Packet.h"
#include <Mutex/Mutex.h>

#include "Physics/PhysicsManager.h"
#include "StateManager.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Input/InputManager.h"
#include "Script/Script.h"
#include "Maps/MapManager.h"
#include "Script/ScriptManager.h"

#include "Network/Http.h"
#include "Network/CURLManager.h"

#include "UI/UIUtil.h"
#include "UI/UIInputs.h"
#include "UI/UITypes.h"
#include "UI/UIFileBrowser.h"
#include "UI/UIQueryDialogue.h"
#include "UI/UIVideo.h"

#include "Network/NetworkManager.h"
#include "Window/AppWindowManager.h"
#include "Audio/AudioManager.h"
#include "Multimedia/MultimediaManager.h"

#include "Graphics/Camera/Camera.h"
#include "Graphics/Messages/GMSet.h"

#include "File/LogFile.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"

extern Camera * CreateEditorCamera(Entity ** e = NULL);

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
MessageManager::~MessageManager()
{
    msgQueueMutex.Destroy();
    packetQueueMutex.Destroy();
	messageQueue.ClearAndDelete();
#ifdef USE_NETWORK
	while(!packetQueue.isOff())
		delete packetQueue.Pop();
#endif
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
	List<Message*> messagesToProcess;
	// Clear it.
	while(messageQueue.Size())
	{
		messagesToProcess = messageQueue;
		messageQueue.Clear();
	}
	/// Fetch all delayed messages too.
	Time now = Time::Now();
	for (int i = 0; i < delayedMessages.Size(); ++i)
	{
		Message * mes = delayedMessages[i];
		if (mes->timeToProcess < now)
		{
			delayedMessages.Remove(mes);
			messagesToProcess.Add(mes);
			--i;
			continue;
		}
	}
	msgQueueMutex.Release();

	// And then process them.
	for (int i = 0; i < messagesToProcess.Size(); ++i)
	{
		Message * message = messagesToProcess[i];
		ProcessMessage(message);	
	}
	messagesToProcess.ClearAndDelete();
}

/// Processes string-based message.
void MessageManager::ProcessMessage(String message)
{
	Message msg(message);
	ProcessMessage(&msg);
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
bool MessageManager::QueueMessages(String messages, UIElement * elementThatTriggeredIt)
{
	msgQueueMutex.Claim(-1);	
	try{
		List<String> messageList = messages.Tokenize("&");
		for (int i = 0; i < messageList.Size(); ++i){
			String message = messageList[i];
			Message * msg = new Message(message);
			msg->element = elementThatTriggeredIt;
			messageQueue.Add(msg);
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


bool MessageManager::QueueMessages(const List<Message*> & messages, UIElement * elementThatTriggeredIt)
{
	msgQueueMutex.Claim(-1);	
    try{
		messageQueue.Add(messages);
	}
	catch(...){
		std::cout<<"\nFailed to push message!";
		assert(false && "MessageManager::QueueMessage");
		return false;
	}
	msgQueueMutex.Release();
}	

/// Queues a bunch of string-based messages in the form "Message1&Message2&Message3&..."
bool MessageManager::QueueMessages(List<String> messages, UIElement * elementThatTriggeredIt)
{
	msgQueueMutex.Claim(-1);	
	try{
		for (int i = 0; i < messages.Size(); ++i)
		{
			String messageInOriginalList = messages[i];
			List<String> messageList = messageInOriginalList.Tokenize("&");
			for (int i = 0; i < messageList.Size(); ++i){
				String message = messageList[i];
				Message * msg = new Message(message);
				msg->element = elementThatTriggeredIt;
				messageQueue.Add(msg);
			}
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


bool MessageManager::QueueMessage(Message* msg)
{
	msgQueueMutex.Claim(-1);
    try{
		messageQueue.Add(msg);
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
void MessageManager::ProcessMessage(Message * message)
{
	// Check for UI-messages first.
	String msg = message->msg;
//	UserInterface * globalUI = GlobalUI();
	// Do note that not all messages uses the string-argument...
//	if (!msg.Length())
//		return;

	// Let active lighting process messages if wanted.
	Lighting * activeLighting = Graphics.ActiveLighting();
	if (activeLighting)
	{
		if (activeLighting->ProcessMessage(message))
			return;
	}
			
	WindowMan.ProcessMessage(message);

	switch(message->type)
	{
		case MessageType::BOOL_MESSAGE:
		{
			BoolMessage * bm = (BoolMessage*) message;
			if (msg == "BGMEnabled")
			{
				QueueAudio(new AMSetb(AT_BGM_ENABLED, bm->value));
			}
			break;
		}
		case MessageType::COLLISSION_CALLBACK:
		{
			CollisionCallback * cc = (CollisionCallback*) message;
			/// o.o
			Entity * one = cc->one;
			Entity * two = cc->two;
			one->ProcessMessage(cc);
			two->ProcessMessage(cc);
			break;	
		}
		case MessageType::RAYCAST:
		{
			Raycast * raycast = (Raycast*) message;
			if (raycast->relevantEntity)
			{
				raycast->relevantEntity->ProcessMessage(message);
			}
			break;	
		}
		case MessageType::DRAG_AND_DROP:
		{
			DragAndDropMessage * dadm = (DragAndDropMessage*) message;
			// Hover to where the drop is to take place.
			InputMan.MouseMove(HoverWindow(), dadm->position);
			/// Check cursor location, can we drop stuff?
			UIElement * e = InputMan.HoverElement();
			if (e)
				e->ProcessMessage(message);
			break;
		}
		case MessageType::PASTE:
		{
			if (msg.Contains("Paste:"))
			{
				// Check for active ui element.
				UserInterface * ui = ActiveUI();
				if (ui){
					UIElement * element = ui->GetActiveElement();
					if (element)
					{
						element->ProcessMessage(message);
					}
				}
			}	
		}
		case MessageType::STRING:
		{
			msg.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (msg.StartsWith("SetLight"))
			{
				Light::ProcessMessageStatic(message);
			}
			else if (msg == "ToggleMute")
			{
				// Mute?
				QueueAudio(new AudioMessage(AM_TOGGLE_MUTE));
			}
			else if (msg.StartsWith("SetGravity"))
			{
				String gravStr = msg.Tokenize("()")[1];
				Vector3f grav;
				grav.ReadFrom(gravStr);
				PhysicsQueue.Add(new PMSet(PT_GRAVITY, grav));
			}
			else if (msg.StartsWith("AdjustMasterVolume("))
			{
				float diff = msg.Tokenize("()")[1].ParseFloat();
				QueueAudio(new AMSet(AT_MASTER_VOLUME, AudioMan.MasterVolume() + diff));
			}
			else if (msg == "CreateEditorCamera")
			{
				CreateEditorCamera();
			}
			else if (msg.Contains("CreateNormalMapTestEntities"))
			{
				// Create some entities.
				Entity * entity = MapMan.CreateEntity("NormalMapSprite", ModelMan.GetModel("sprite.obj"), TexMan.GetTexture("0x77"), Vector3f(0,0,0));
				GraphicsQueue.Add(new GMSetEntityTexture(entity, NORMAL_MAP, "normalMapTest2"));
			}
			if (msg == "AcceptInput:false")
				inputState->acceptInput = false;
			else if (msg.Contains("InputMan.printHoverElement"))
				InputMan.printHoverElement = !InputMan.printHoverElement;
			else if (msg == "SetGlobalState:NULL")
				StateMan.SetGlobalState(NULL);
			else if (msg == "SetActiveState:NULL")
				StateMan.SetActiveState(NULL);
			else if (msg == "StateMan.DeleteStates")
				StateMan.DeleteStates();
			else if (msg == "NetworkMan.Shutdown")
				NetworkMan.Shutdown();
			else if (msg == "StateMan.Shutdown")
				StateMan.shouldLive = false;
			else if (msg == "MultimediaMan.Shutdown")
				MultimediaMan.Shutdown();
			else if (msg == "AudioMan.Shutdown")
				AudioMan.QueueMessage(new AudioMessage(AM_SHUTDOWN));
			else if (msg == "GraphicsMan.Shutdown")
				Graphics.QueueMessage(new GraphicsMessage(GM_SHUTDOWN));
			else if (msg == "PrintScreenshot")
			{
				Graphics.QueueMessage(new GraphicsMessage(GM_PRINT_SCREENSHOT));
			}
			else if (msg.Contains("SetOutOfFocusSleepThread("))
			{
				int sleepTime = msg.Tokenize("()")[1].ParseInt();
				GraphicsMan.QueueMessage(new GMSeti(GM_SET_OUT_OF_FOCUS_SLEEP_TIME, sleepTime));
			}
			else if (msg.StartsWith("RenderGrid"))
			{
				// Disable it, everywhere?
				for (int i = 0; i < WindowMan.GetWindows().Size(); ++i)
				{
					AppWindow * w = WindowMan.GetWindows()[i];
					w->RenderGrid(false);
				}
			}
			else if (msg.Contains("PrintHttpOutput("))
			{
				bool value = msg.Tokenize("()")[1].ParseBool();
				printHttpOutput = value;
			}
			else if (msg.Contains("SetHttpTool:"))
			{
				httpTool = msg.Tokenize(":")[1].ParseInt();
			}
			else if (msg.Contains("HttpGet:"))
			{
				String url = msg - "HttpGet:";
				HttpGet(url);
			}
			else if (msg == "ResumePhysics")
			{
 				PhysicsMan.Resume();
			}
			else if (msg.Contains("PlayBGM:"))
			{
				String source = msg - "PlayBGM:";
				AudioMan.QueueMessage(new AMPlayBGM(source, 1.f));
			}
			else if (msg.Contains("NavigateUI(")){
				bool toggle = msg.Tokenize("()")[1].ParseBool();
				InputMan.NavigateUI(toggle);
				return;
			}
			else if (msg == "IgnoreMouseInput")
			{
				bool & ignore = InputMan.ignoreMouse;
				ignore = !ignore;
			}
			else if (msg == "DisableLogging")
			{
				extern bool loggingEnabled;
				loggingEnabled = false;
			}
			else if (msg == "EnableLogging")
			{
				extern bool loggingEnabled;
				loggingEnabled = true;
			}
			else if (msg == "PrintExpressionSymbols")
			{
				Expression::printExpressionSymbols = true;
			}
			else if (msg == "List cameras")
			{
				CameraMan.ListCameras();
			}
			else if (msg == "mute")
			{
				AudioMan.QueueMessage(new AudioMessage(AM_DISABLE_AUDIO));
		//		AudioMan.DisableAudio();
			}
			else if (msg == "CreateMainWindow")
			{	
				// Creates the main application AppWindow. A message is sent upon startup from the initializer thread for this.
				if (!WindowMan.MainWindow())
				{
					AppWindow * mainWindow = WindowMan.CreateMainWindow();
					// Optionally set more user-related stuff to the options before creating it.
			
					// Then create it!
					mainWindow->Create();
					/// Create default UI and globalUI that may later on be replaced as needed.
					mainWindow->CreateUI();
					mainWindow->CreateGlobalUI();
					mainWindow->backgroundColor = Vector4f(1,0,0,1);
				}
				// Reveal the main AppWindow to the user now that all managers are allocated.
				WindowMan.MainWindow()->Show();
			}
			else if (msg == "HideWindows")
			{
				if (MainWindow())
				{
					MainWindow()->Hide();
				}
			}
			else if (msg == "DestroyMainWindow" || 
				msg == "DeleteWindows" || 
				msg == "DeleteMainWindow")
			{
				if (WindowMan.MainWindow())
				{
					AppWindow * mainWindow = WindowMan.MainWindow();
					mainWindow->Hide();
					mainWindow->Destroy();
				}
			}
			else if (msg.Contains("MaximizeWindow("))
			{
				String windowName = msg.Tokenize("()")[1];
				AppWindow * window = WindowMan.GetWindowByName(windowName);
				if (window)
				{
					if (!window->IsFullScreen())
						window->ToggleFullScreen();
				}
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
				UserInterface * ui = RelevantUI();
				UIElement * e = ui->GetElementByName(name);
				if (!e)
					return;
				e->Proceed();
				return;
			}
			else if (msg.Contains("UITextureInput("))
			{
				String name = msg.Tokenize("()")[1];
				UserInterface * ui = RelevantUI();
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
				UserInterface * ui = RelevantUI();
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
				UserInterface * ui = RelevantUI();
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
				UserInterface * ui = RelevantUI();
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
				UserInterface * ui = RelevantUI();
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
			else if (msg.Contains("setVisibility"))
			{
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
				InputMan.cyclicY = msg.Tokenize("()")[1].ParseBool();
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
				Graphics.QueueMessage(new GMPushUI(dialog, GlobalUI()));
				return;
			}
			else if (msg.Contains("SetFileBrowserDirectory("))
			{
				List<String> params = msg.Tokenize(",()");
				String uiName = params[1];
				UIElement * ui = RelevantUI()->GetElementByName(uiName);
				if (!ui)
					return;
				assert(ui->type == UIType::FILE_BROWSER);
				UIFileBrowser * fb = (UIFileBrowser*)ui;
				String path = params[2];
				if (path == "this")
					path = message->element->text;
				fb->SetPath(path, false);
				return;
			}
			else if (msg.Contains("UpdateFileBrowserDirectory("))
			{
				List<String> params = msg.Tokenize(",()");
				String uiName = params[1];
				UIElement * ui = RelevantUI()->GetElementByName(uiName);
				if (!ui)
					return;
				assert(ui->type == UIType::FILE_BROWSER);
				UIFileBrowser * fb = (UIFileBrowser*)ui;
				String path = params[2];
				if (path == "this")
					path = message->element->text;
				fb->UpdatePath(path, false);
				return;
			}
			else if (msg.Contains("EvaluateFileBrowserSelection("))
			{
				List<String> params = msg.Tokenize("()");
				String uiName = params[1];
				UIElement * ui = RelevantUI()->GetElementByName(uiName);
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
				UIElement * ui = RelevantUI()->GetElementByName(uiName);
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
				UserInterface * ui = RelevantUI();
				assert(ui);
				Graphics.QueueMessage(new GMAddUI(fileBrowser, "root", ui));
				Graphics.QueueMessage(new GMPushUI(fileBrowser, ui));
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
				{
					LogMain("Bad arguments in message: "+msg, ERROR);
					return;
				}
				PushUI(params[1]);
				return;
			}
			else if (msg.Contains("PopFromStack(") || msg.Contains("PopUI("))
			{
//				std::cout<<"\nPopFromStack/PopUI received.";
				// Fetch target.
				List<String> params = msg.Tokenize("(),");
				assert(params.Size() >= 2 && "Invalid amount of arguments to PopFromStack UI command!");
				if (params.Size() < 2){
					std::cout<<"\nToo few parameters.";
					return;
				}
				String uiName = params[1];
				if (uiName == "this")
					uiName = message->element->name;
				PopUI(uiName);
				return;
			}
			else if (msg == "Back")
			{
				UserInterface * ui = RelevantUI();
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

			break;
		}
	}

	
	// First send it to global state
	StateManager * stateMan = StateManager::Instance();
	if (stateMan)
	{
		AppState * global = StateMan.GlobalState();
		if (global)
			global->ProcessMessage(message);
		// Send it to the state for processing
		if (StateMan.ActiveState())
			StateMan.ActiveState()->ProcessMessage(message);
	}
}
