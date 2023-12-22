#include "events.h"

#include <string>

DropEvent::DropEvent(std::string pathAndFilename)
{
	m_Type = EVENT_TYPE_DROP;
	m_pathAndFilename = pathAndFilename;
}
