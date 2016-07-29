/// Emil Hedemalm
/// 2016-07-29
/// Message for request/response to the waypoint- and path-managing system.

#include "PathMessage.h"

/// Request constructor.
PathMessage::PathMessage(Entity * entity, Waypoint * from, Waypoint * to)
	: Message(MessageType::PATHFINDING_MESSAGE), from(from), to(to)
{
	recipientEntity = entity;
	requestResponse = REQUEST;
}
/// Response type. Assumes path has already been written to.
PathMessage::PathMessage(Entity * entity)
	: Message(MessageType::PATHFINDING_MESSAGE)
{
	recipientEntity = entity;
	requestResponse = RESPONSE;
}


PathMessage::~PathMessage(){}
