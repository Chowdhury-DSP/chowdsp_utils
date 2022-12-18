#include "chowdsp_ParameterListeners.h"

namespace chowdsp
{
ParameterListeners::ParameterListenersBackgroundTask::ParameterListenersBackgroundTask (ParameterListeners& paramListeners, int timeSliceInterval)
    : listeners (paramListeners),
      interval (timeSliceInterval)
{
}

int ParameterListeners::ParameterListenersBackgroundTask::useTimeSlice()
{
    for (const auto [index, paramInfo] : enumerate (listeners.paramInfoList))
    {
        if (paramInfo.paramCookie->getValue() == paramInfo.value)
            continue;

        paramInfo.value = paramInfo.paramCookie->getValue();

        listeners.messageThreadBroadcastQueue.enqueue ([this, i = index]
                                                       { listeners.callMessageThreadBroadcaster (i); });
        listeners.audioThreadBroadcastQueue.try_enqueue ([this, i = index]
                                                         { listeners.callAudioThreadBroadcaster (i); });
        listeners.triggerAsyncUpdate();
    }

    return interval;
}

//=============================================================================
ParameterListeners::ParameterListeners (ParamHolder& parameters, juce::TimeSliceThread* backgroundTimeSliceThread, int timeSliceInterval)
    : backgroundTask (*this, timeSliceInterval),
      totalNumParams ((size_t) parameters.count())
{
    parameters.doForAllParameters (
        [this] (auto& param, size_t index)
        {
            const auto* rangedParam = static_cast<juce::RangedAudioParameter*> (&param);
            paramInfoList[index] = ParamInfo { rangedParam, rangedParam->getValue() };
        });

    if (backgroundTimeSliceThread == nullptr)
        backgroundThread = OptionalPointer<juce::TimeSliceThread> ("ChowDSP Parameter Listeners Thread");
    else
        backgroundThread.setNonOwning (backgroundTimeSliceThread);

    backgroundThread->addTimeSliceClient (&backgroundTask);
    if (! backgroundThread->isThreadRunning())
        backgroundThread->startThread();
}

ParameterListeners::~ParameterListeners()
{
    backgroundThread->removeTimeSliceClient (&backgroundTask);
    if (backgroundThread->getNumClients() == 0)
        backgroundThread->stopThread (100);
}

void ParameterListeners::callAudioThreadBroadcasters()
{
    AudioThreadAction action;
    while (audioThreadBroadcastQueue.try_dequeue (action))
        action();
}

void ParameterListeners::callMessageThreadBroadcasters()
{
    MessageThreadAction action;
    while (messageThreadBroadcastQueue.try_dequeue (action))
        action();
}

void ParameterListeners::callMessageThreadBroadcaster (size_t index)
{
    messageThreadBroadcasters[index]();
}

void ParameterListeners::callAudioThreadBroadcaster (size_t index)
{
    audioThreadBroadcasters[index]();
}

void ParameterListeners::handleAsyncUpdate()
{
    callMessageThreadBroadcasters();
}
} // namespace chowdsp
