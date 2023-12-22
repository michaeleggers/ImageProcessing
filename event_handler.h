#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include <list>

#include "ievent_subscriber.h"
#include "ievent.h"

class EventHandler {

public:
	EventHandler() {};
	~EventHandler() {};

	void Attach(IEventSubscriber* evntSubscr);
	void Notify(IEvent* event);

private:
	std::list<IEventSubscriber*> m_EventSubscribers;


};

#endif