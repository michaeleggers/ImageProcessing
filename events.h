#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <string>
#include <vector>

#include "ievent.h" 
#include "image.h"
#include "render_common.h"

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

class RenderStartEvent : public IEvent {
public:
	RenderStartEvent(
		std::vector<Line> sourceLines, std::vector<Line> destLines, 
		Image sourceImage, Image destImage, 
		int numIterations, 
		float A, float B, float P);
	virtual ~RenderStartEvent() {};

	std::vector<Line> m_sourceLines, m_destLines;
	Image m_sourceImage, m_destImage;
	int m_numIterations;
	float m_A, m_B, m_P;
};

class RenderDoneEvent : public IEvent {
public:
	RenderDoneEvent(std::vector<Image> sourceToDestMorphs, std::vector<Image> destToSourceMorphs);
	virtual ~RenderDoneEvent() {};

	std::vector<Image> m_sourceToDestMorphs;
	std::vector<Image> m_destToSourceMorphs;
};

#endif
