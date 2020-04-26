/// Emil Hedemalm
/// 2016-07-29
/// Message for request/response to the waypoint- and path-managing system.

#ifndef PATH_MESSAGE_H
#define PATH_MESSAGE_H

#include "Message/Message.h"
#include "Pathfinding/Path.h"
#include "Entity/Entity.h"
#include "Pathfinding/Waypoint.h"

class SetPathDestinationMessage : public Message 
{
public:
	SetPathDestinationMessage(ConstVec3fr position) : Message(MessageType::SET_PATH_DESTINATION_MESSAGE), pos(position){};
	virtual ~SetPathDestinationMessage(){};
	Vector3f pos;
};

class PathMessage : public Message
{
public:
	/// Request constructor.
	PathMessage(Entity * entity, Waypoint * from, Waypoint * to);
	/// Response type. Assumes path has already been written to.
	PathMessage(Entity * entity);
	virtual ~PathMessage();
	enum 
	{
		REQUEST,
		RESPONSE,
	};
	int requestResponse;
	Waypoint * to; // Destination wp.
	Waypoint * from; // Starting point.
	Path path; // Stored path as reply.
	Entity* entity;
};

/// Sent for other properties to interpret. -> Start walking, running, etc.
class MoveToMessage : public Message 
{
public:
	MoveToMessage(ConstVec3fr toLoc): Message(MessageType::MOVE_TO_MESSAGE), pos(toLoc){};
	virtual ~MoveToMessage(){};
	Vector3f pos;
};

#endif