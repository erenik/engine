/// Emil Hedemalm
/// 2013-02-08

#ifndef AI_MESSAGES_H
#define AI_MESSAGES_H

namespace AIMsg{
enum AIMessages{
	CHARACTER_DEAD,
	SOCIALIZE_QUERY,
	SOCIALIZE_CONFIRMED,
	SOCIALIZE_IGNORE,		// If sleeping or stuff
	SOCIALIZE_DECLINE,		// If declined during conversation
	SOCIALIZE_ATLOCATION,	
	SOCIALIZE_BYE,
};};

#endif
