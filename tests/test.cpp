/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>

//==============================================================================
class ConsoleLogger : public Logger
{
    void logMessage (const String& message) override
    {
        std::cout << message << std::endl;

#if JUCE_WINDOWS
        Logger::outputDebugString (message);
#endif
    }
};

//==============================================================================
class ConsoleUnitTestRunner : public UnitTestRunner
{
public:
    void runTestWithName (const String& name, int64 randomSeed)
    {
        Array<UnitTest*> unitTests;
        for (auto* test : UnitTest::getAllTests())
        {
            if (test->getName() == name)
                unitTests.add (test);
        }

        if (unitTests.isEmpty())
        {
            logMessage ("Test with name " + name + " not found!");
            return;
        }

        runTests (unitTests, randomSeed);
    }

    void logMessage (const String& message) override
    {
        Logger::writeToLog (message);
    }
};

//==============================================================================
int main (int argc, char** argv)
{
    ArgumentList args (argc, argv);

    if (args.containsOption ("--help|-h"))
    {
        std::cout << argv[0] << " [--help|-h] [--list-categories] [--single-test test] [--category category] [--seed seed]" << std::endl;
        return 0;
    }

    if (args.containsOption ("--list-categories"))
    {
        for (auto& category : UnitTest::getAllCategories())
            std::cout << category << std::endl;

        return 0;
    }

    ScopedJuceInitialiser_GUI scopedJuce;

    ConsoleLogger logger;
    Logger::setCurrentLogger (&logger);

    ConsoleUnitTestRunner runner;

    auto seed = [&args] {
        if (args.containsOption ("--seed"))
        {
            auto seedValueString = args.getValueForOption ("--seed");

            if (seedValueString.startsWith ("0x"))
                return seedValueString.getHexValue64();

            return seedValueString.getLargeIntValue();
        }

        return Random::getSystemRandom().nextInt64();
    }();

    if (args.containsOption ("--single-test"))
        runner.runTestWithName (args.getValueForOption ("--single-test"), seed);
    else if (args.containsOption ("--category"))
        runner.runTestsInCategory (args.getValueForOption ("--category"), seed);
    else
        runner.runAllTests (seed);

    Logger::setCurrentLogger (nullptr);

    for (int i = 0; i < runner.getNumResults(); ++i)
        if (runner.getResult (i)->failures > 0)
            return 1;

    return 0;
}
