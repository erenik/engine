/// Emil Hedemalm
/// 2014-01-24
/// Packet type meant to synchronize or request synchronization of entities or other objects.

#include "SyncPacket.h"
#include "PacketTypes.h"
#include "Entity/Entity.h"
#include <sstream>

SyncPacket::SyncPacket()
: Packet(PacketType::SYNCHRONIZATION)
{

}

void SyncPacket::AddEntity(Entity *entity)
{
	entities.Add(entity);
}

/// Re-writes the data to be sent.
void SyncPacket::UpdateData()
{
	data.PopAll();
	std::stringstream ss;
	ss << propertiesToSync + "\n";
	for (int i = 0; i < entities.Size(); ++i)
	{
		if (propertiesToSync.Contains("position"))
			ss << entities[i]->position;
	}
	String text = ss.str().c_str();
	data.Push(text);
}

#ifndef SYNC_PACK_H
#define SYNC_PACK_H

class SyncPacket : public Packet {
public:
	SyncPacket();
	/// For adding entity to sync
	void AddEntity(Entity * entity);
	/// String with properties to sync.
	String propertiesToSync;
private:
	/// Re-writes the data to be sent.
	void UpdateData();
	List<Entity*> entities;
	
};

#endif