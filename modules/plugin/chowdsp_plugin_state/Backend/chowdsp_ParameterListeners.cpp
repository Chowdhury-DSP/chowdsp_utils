#include "chowdsp_ParameterListeners.h"

namespace chowdsp
{
ParameterListeners::ParameterListeners (ParamHolder& parameters, int interval)
    : totalNumParams ((size_t) parameters.count())
{
    parameters.doForAllParameters (
        [this] (auto& param, size_t index)
        {
            const auto* rangedParam = static_cast<juce::RangedAudioParameter*> (&param);
            paramInfoList[index] = ParamInfo { rangedParam, rangedParam->getValue() };
        });

    startTimer (interval);
}

void ParameterListeners::timerCallback()
{
    updateBroadcastersFromMessageThread();
}

void ParameterListeners::updateBroadcastersFromMessageThread()
{
    jassert (juce::MessageManager::existsAndIsCurrentThread());
    for (const auto [index, paramInfo] : enumerate (paramInfoList))
    {
        if (juce::approximatelyEqual (paramInfo.paramCookie->getValue(), paramInfo.value))
            continue;

        paramInfo.value = paramInfo.paramCookie->getValue();
        if( audioThreadBroadcasters.size() > index) {
            audioThreadBroadcastQueue.try_enqueue([this, i = index] { callAudioThreadBroadcaster(i); });
        }
        callMessageThreadBroadcaster (index);
    }
}

void ParameterListeners::callAudioThreadBroadcasters()
{
    AudioThreadAction action;
    while (audioThreadBroadcastQueue.try_dequeue (action))
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
} // namespace chowdsp
