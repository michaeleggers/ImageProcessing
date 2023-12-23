#include "events.h"

#include <string>

DropEvent::DropEvent(std::string pathAndFilename)
{
	m_Type = EVENT_TYPE_DROP;
	m_pathAndFilename = pathAndFilename;
}

RenderUpdateEvent::RenderUpdateEvent(std::string message, float pctDone)
{
	m_Type = EVENT_TYPE_RENDER_UPDATE;
	m_Message = message;
	m_pctDone = pctDone;
}
