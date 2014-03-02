/// Emil Hedemalm
/// 2014-01-28
/// Event structure for handling subscribe/notify mechanism

#ifndef SIP_EVENT_H
#define SIP_EVENT_H

class SIPEvent {
public:
	/// Type of the event.
	String name;
	/// Current state of this event.
	String state;
};

class SIPEventSubscription {
public:
	/// Name of the event the subscription applies to.
	String name;
	/// Duration in seconds.
	int duration;
	/// MS using Timer:: class
	long long startTime;
};

#endif
