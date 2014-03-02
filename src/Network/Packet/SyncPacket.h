/// Emil Hedemalm
/// 2014-01-24
/// Packet type meant to synchronize or request synchronization of entities or other objects.

#ifndef SYNC_PACK_H
#define SYNC_PACK_H

#include "Packet.h"

class Entity;

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