/// Emil Hedemalm
/// 2014-01-24
/// SessionData class made to store session-related data for each peer.

#ifndef SESSION_DATA_H
#define SESSION_DATA_H

/// Subclass this to include any variables needed.
class SessionData {
public:
	/// See SessionTypes in Session.h for valid types.
	SessionData(int type, int subType = -1);
	/// Type should be same as the Session type, See SessionTypes.h for a full list of session-types.
	int type;
	/// See e.g. GameSessionTypes.h
	int subType;
};

#endif