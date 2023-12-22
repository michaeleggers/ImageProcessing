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

#endif