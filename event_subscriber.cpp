#include "event_subscriber.h"

EventSubscriber::EventSubscriber(EventHandler* eventHandler)
{
	m_EventHandler = eventHandler;
	m_EventHandler->Attach(this);
}

EventSubscriber::~EventSubscriber()
{
}

void EventSubscriber::Update(IEvent* event)
{
}
