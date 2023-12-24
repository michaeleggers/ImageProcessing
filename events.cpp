#include "events.h"

#include <string>
#include <vector>

#include "render_common.h"
#include "image.h"

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

RenderStartEvent::RenderStartEvent(
	std::vector<Line> sourceLines, std::vector<Line> destLines,
	Image sourceImage, Image destImage, 
	int numIterations, float A, float B, float P)
{
	m_Type = EVENT_TYPE_RENDER_START;
	m_sourceLines = sourceLines;
	m_destLines = destLines;
	m_sourceImage = sourceImage;
	m_destImage = destImage;
	m_numIterations = numIterations;
	m_A = A;
	m_B = B;
	m_P = P;
}

RenderDoneEvent::RenderDoneEvent(std::vector<Image> sourceToDestMorphs, std::vector<Image> destToSourceMorphs)
{
	m_Type = EVENT_TYPE_RENDER_DONE;
	m_sourceToDestMorphs = sourceToDestMorphs;
	m_destToSourceMorphs = destToSourceMorphs;
}
