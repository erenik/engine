// Emil Hedemalm
// 2013-08-09

#include "ChatMessage.h"
#include "Timer/Timer.h"
#include "Network/Peer.h"

#include <ctime>
#include <cstring>

ChatMessage::ChatMessage(Peer * playerWhoSentIt, String text, int i_type)
: from(playerWhoSentIt), text(text), type(i_type)
{
    std::cout<<"\nCreating chat message.. if possible...";
    /// No name if no player
    if (from == NULL){
        type = GLOBAL_ANNOUNCEMENT;
  //      std::cout<<"\nPlayer null, type deemed global announcement.";
    }
    else{
    //    std::cout<<"\nFrom a player?!";
        playerName = from->name;
        //  fromIP = from->stuff
    }

  //  std::cout<<"\nGetting time.";
    timeCreated = Timer::GetCurrentTimeMs();
	/// Convert it to seconds, yo.
	timeCreated *= 0.001f;
   // std::cout<<"\nTiming time.";
    tm * cTime = NULL;
    time_t tTime = timeCreated;
    cTime = localtime(&tTime);

	time_t currentTime = time(NULL);
	assert(cTime != NULL);

  //  std::cout<<"\nFormatting time..";
    char buf[50];
    memset(buf, 0, 50);
	// Got an error while compiling on Windows:
	// %R and some other formats are not supported by VS, see http://msdn.microsoft.com/en-us/library/fe06s4ak%28v=vs.71%29.aspx
	// Custom formatting might be better anyway.. :)
	strftime(buf, 50, "%H:%M", cTime);
    timeString = buf;
}
