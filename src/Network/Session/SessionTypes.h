/// Emil Hedemalm
/// 2014-01-24
/// Simple enum

#ifndef SESSION_TYPES_H
#define SESSION_TYPES_H

namespace SessionType {
enum sessionTypes {
	NULL_TYPE,
	SIP, // SIP session. Base session to facilitate all other session initiations.
	GAME, // Session type that handles both a chat and a game, including spectators.
	VOIP, // Voice over IP.
	VIDEO, // For transmitting a live-stream, camera, or similar.
};};

#endif
