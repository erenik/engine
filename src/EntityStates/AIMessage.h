/// Emil Hedemalm
/// 2013-02-08

#ifndef AIMESSAGE
#define AIMESSAGE

struct AIMessage {
	AIMessage(int type) : type(type), target(NULL), sender(NULL), data(NULL) {};
	AIMessage(int type, Entity * target, Entity * sender, void * data) : type(type), target(target), sender(sender), data(data) {};
	int type;
	Entity * target;
	Entity * sender;
	void * data;
};

#endif