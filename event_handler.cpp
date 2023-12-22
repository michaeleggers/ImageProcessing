#include "event_handler.h"
#include "ievent.h"

void EventHandler::Attach(IEventSubscriber* evntSubscr)
{
	m_EventSubscribers.push_back(evntSubscr);
}

void EventHandler::Notify(IEvent* event)
{
	for (auto& subscriber : m_EventSubscribers) {
		subscriber->Update(event);
	}
}
