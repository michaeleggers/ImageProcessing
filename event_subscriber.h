#ifndef _EVENT_SUBSCRIBER_H_
#define _EVENT_SUBSCRIBER_H_

#include "ievent_subscriber.h"
#include "ievent.h" 
#include "event_handler.h"

// Example for a class that subscribes to events

class EventSubscriber : public IEventSubscriber {
public:
	EventSubscriber(EventHandler* eventHandler);
	virtual ~EventSubscriber() override;

	virtual void Update(IEvent* event) override;

private:
	EventHandler* m_EventHandler;
};

#endif
