/// Emil Hedemalm
/// 2014-01-14
/// Command-line arg evaluator

#include "CommandLine.h"
#include "Audio/AudioManager.h"
#include "Network/NetworkManager.h"
#include "Message/MessageManager.h"

String CommandLine::argString;
List<String> CommandLine::args;

/// Evaluates the whole command-line, feeding its arguments as messages to the Message manager.
void CommandLine::Evaluate()
{
    List<String> tokens = argString.Tokenize(" ");
	for (int i = 0; i < args.Size(); ++i){
		String arg = args[i];
		if (arg == "mute")
		{
			AudioMan.QueueMessage(new AudioMessage(AM_DISABLE_AUDIO));
//			AudioMan.DisableAudio();
		}
		else if (arg == "host"){
			NetworkMan.StartSIPServer();
		}
		else if (arg.Contains("ip="))
		{
			List<String> ip = arg.Tokenize("=");
			NetworkMan.targetIP = ip[1];
		}
		/// Post a message to the message-manager too.
		MesMan.QueueMessages(arg);
		std::cout<<"\nMessage queued: "<<arg;

	}
    std::cout<<"\nMessage queued: ";
}
