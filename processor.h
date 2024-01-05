#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include <vector>
#include <thread>

#include "image.h"
#include "ievent_subscriber.h"
#include "ievent.h" 
#include "events.h"
#include "event_handler.h"


class Processor : public IEventSubscriber {
public:
	Processor(EventHandler* eventHandler);
	Processor(Processor &&) = default;
	virtual ~Processor();

	virtual void Update(IEvent* event) override;

	void StartRenderThread(RenderStartEvent* rse);
	void CheckRenderThread();

private:
	EventHandler* m_EventHandler;

	std::vector<Image> m_sourceToDestMorphs;
	std::vector<Image> m_destToSourceMorphs;

	float m_sourceRenderPctDone;
	float m_destRenderPctDone;
	bool m_sourceRenderDone;
	bool m_destRenderDone;
	std::thread m_sourceImageThread;
	std::thread m_destImageThread;	
	bool m_stop;
	bool m_isRendering;
};

#endif

