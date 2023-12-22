#ifndef _I_EVENT_SUBSCRIBER_H_
#define _I_EVENT_SUBSCRIBER_H_

#include "ievent.h" 

class IEventSubscriber {
public:	
	virtual ~IEventSubscriber() {};
	virtual void Update(IEvent* event) = 0;

private:

};

#endif
