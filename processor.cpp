#include "processor.h"

#include <thread>

#include <SDL2/SDL.h>

#include "events.h"
#include "beierneely.h"

Processor::Processor(EventHandler* eventHandler)
{
    m_EventHandler = eventHandler;
    m_EventHandler->Attach(this);
    m_sourceRenderPctDone = 0.0f;
    m_destRenderPctDone = 0.0f;
    m_sourceRenderDone = true;
    m_destRenderDone = true;
    m_stop = false;
    m_isRendering = false;
    m_sourceImageThread = std::thread();
    m_destImageThread = std::thread();
}

Processor::~Processor()
{
}

void Processor::Update(IEvent* event)
{
    switch (event->m_Type) {
    case EVENT_TYPE_RENDER_START: {
        SDL_Log("Processor: Received render start event.\n");
        // TODO: How to kill a running thread???
        if (!m_sourceRenderDone || !m_destRenderDone) { // already rendering
            SDL_Log("Processor: Received render start event. Already rendering. Wait for finish...\n");
        }
        else {
            m_stop = false;
            RenderStartEvent* rse = (RenderStartEvent*)event;
            StartRenderThread(rse);
            SDL_Log("Processor: Main Render thread started...\n");
        }
    } break;
    case EVENT_TYPE_RENDER_STOP: {              
        if (m_isRendering) {
            m_stop = true;
            while (!m_sourceImageThread.joinable() || !m_destImageThread.joinable()) {} // Wait for threads to finish

            // Event could have been fired more than once so we first have to check if it is save to join

            if (m_sourceImageThread.joinable()) {
                m_sourceImageThread.join();
            }
            if (m_destImageThread.joinable()) {
                m_destImageThread.join();
            }

            m_isRendering = false;
        }
    } break;
    default: {};
    }
}

void Processor::StartRenderThread(RenderStartEvent* rse)
{
    m_sourceRenderDone = false;
    m_destRenderDone = false;
    m_sourceToDestMorphs.clear();
    m_destToSourceMorphs.clear();

    m_sourceImageThread = std::thread(&BeierNeely,
        rse->m_sourceLines, rse->m_destLines,
        rse->m_sourceImage, rse->m_destImage,
        rse->m_numIterations,
        rse->m_A, rse->m_B, rse->m_P,
        std::ref(m_sourceToDestMorphs), &m_sourceRenderPctDone, &m_sourceRenderDone, &m_stop);

    m_destImageThread = std::thread(&BeierNeely,
        rse->m_destLines, rse->m_sourceLines,
        rse->m_destImage, rse->m_sourceImage,
        rse->m_numIterations,
        rse->m_A, rse->m_B, rse->m_P,
        std::ref(m_destToSourceMorphs), &m_destRenderPctDone, &m_destRenderDone, &m_stop);

    m_isRendering = true;
}

void Processor::CheckRenderThread()
{
    if (!m_sourceRenderDone || !m_destRenderDone) {
        float totalRenderPct = (m_sourceRenderPctDone + m_destRenderPctDone) / 2.0f;
        RenderUpdateEvent rue("Message (Render update)", totalRenderPct);
        m_EventHandler->Notify(&rue);

        return;
    }
    else if (!m_sourceImageThread.joinable() || !m_destImageThread.joinable()) {
        return;
    }

    // Hack: render done is received and final pct-age is not captured.
    //       So do it again here to get 100%
    float totalRenderPct = (m_sourceRenderPctDone + m_destRenderPctDone) / 2.0f;
    RenderUpdateEvent rue("Message (Render update)", totalRenderPct);
    m_EventHandler->Notify(&rue);

    m_sourceImageThread.join();
    m_destImageThread.join();
    m_sourceRenderDone = true;
    m_destRenderDone = true;

    RenderDoneEvent rde(m_sourceToDestMorphs, m_destToSourceMorphs);
    m_EventHandler->Notify(&rde);
}
