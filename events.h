#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <string>

#include "ievent.h" 

class DropEvent : public IEvent {
public:
	DropEvent(std::string pathAndFilename);
	virtual ~DropEvent() {};

	std::string m_pathAndFilename;	
};

class RenderUpdateEvent : public IEvent {
public:
	RenderUpdateEvent(std::string message, float pctDone);
	virtual ~RenderUpdateEvent() {};

	std::string m_Message;
	float m_pctDone;
};

#endif