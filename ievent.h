#ifndef _I_EVENT_H_
#define _I_EVENT_H_

enum EventType {
	EVENT_TYPE_DROP,
	EVENT_TYPE_RENDER_UPDATE,
	MAX_EVENT_TYPES
};

class IEvent {
public:
	virtual ~IEvent() {};
	EventType m_Type;
};

#endif

