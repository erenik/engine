// Emil Hedemalm
// 2014-03-05
// 
#include "GlobalState.h"
#include "Actions.h"
#include "Input/InputManager.h"
// Don't include all managers. Ever.

/// Creates bindings that are used for debugging purposes only
void GlobalState::CreateDefaultBindings(){
	std::cout<<"\nGlobalState::CreateDefaultBindings() called";

	/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	/// Get pointer to this mapping
	InputMapping * mapping = &inputMapping;
	/// Create default bindings

	/// C = Create, L = List

	// Edit Simulation
	mapping->CreateBinding(START_HOSTING, KEY::CTRL, KEY::N, KEY::H, "CTRL+N+H : Hosts a server on default port (50333)");
	mapping->CreateBinding(START_JOINING, KEY::CTRL, KEY::N, KEY::J, "CTRL+N+J : Joins a server on 127.0.0.1 default port (50333)");
	mapping->CreateBinding(START_JOINING_MY_WINDOWS_MACHINE, KEY::CTRL, KEY::N, KEY::W, "CTRL+N+W : Joins a server on 130.240.132.45 default port (50333)");
	mapping->CreateBinding(STOP_NETWORK, KEY::CTRL, KEY::N, KEY::S, "CTRL+N+S : Shut downs current Network connection");

	/// For viewing network statistics
	Binding * b = mapping->CreateBinding("ShowPeerUdpStatus", KEY::U);
//	b->stringStopAction = "HidePeerUdpStatus";

	// Send some packets
	mapping->CreateBinding(SEND_PACKET_1, KEY::CTRL, KEY::SPACE, "CTRL+SPACE : Sends('Message 1')");
	// Make FTP request
	mapping->CreateBinding(MAKE_FTP_REQUEST, KEY::CTRL, KEY::N, KEY::F, "CTRL+N+F : Makes a FTP request");

	mapping->SetBlockingKeys(mapping->CreateBinding(OPEN_CONSOLE, KEY::CTRL, KEY::ENTER, "CTRL + ENTER : Open Console"), KEY::ALT);

	/// Input-options
	mapping->CreateBinding(TOGGLE_MOUSE_LOCK, KEY::CTRL, KEY::L, KEY::M);


};

