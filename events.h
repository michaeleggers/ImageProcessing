#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <string>

#include "ievent.h" 

class DropEvent : public IEvent {
public:
	DropEvent(std::string pathAndFilename);
	~DropEvent() override;

	std::string m_pathAndFilename;	
};

class RenderUpdateEvent : public IEvent {
public:
	RenderUpdateEvent(std::string message, float pctDone);
	~RenderUpdateEvent() override;

	std::string m_Message;
	float m_pctDone;
};

#endif