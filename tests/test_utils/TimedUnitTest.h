#pragma once

#include <juce_core/juce_core.h>

class TimedUnitTest : public juce::UnitTest
{
public:
    explicit TimedUnitTest (const juce::String& _name, const juce::String& _category = {}) : UnitTest (_name, _category)
    {
    }

    virtual void runTestTimed() = 0;

    void runTest() override
    {
        auto startTime = juce::Time::getMillisecondCounter();

        runTestTimed();

        const auto relTime = juce::RelativeTime::milliseconds (static_cast<int> (juce::Time::getMillisecondCounter() - startTime));
        logMessage ("Time taken to run " + getName() + ": " + relTime.getDescription());
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimedUnitTest)
};
