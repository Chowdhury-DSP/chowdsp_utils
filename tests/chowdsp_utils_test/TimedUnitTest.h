#pragma once

#include <JuceHeader.h>

class TimedUnitTest : public UnitTest
{
public:
    TimedUnitTest (const String& name, const String& category = {}) : UnitTest (name, category)
    {
    }

    virtual void runTestTimed() = 0;

    void runTest() override
    {
        auto startTime = Time::getMillisecondCounter();

        runTestTimed();

        const auto relTime = RelativeTime::milliseconds (static_cast<int> (Time::getMillisecondCounter() - startTime));
        logMessage ("Time taken to run " + getName() + ": " + relTime.getDescription());
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimedUnitTest)
};
