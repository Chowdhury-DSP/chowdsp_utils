#include <TimedUnitTest.h>
#include <chowdsp_visualizers/chowdsp_visualizers.h>
#include "VizTestUtils.h"

class SpectrumPlotBaseTest : public TimedUnitTest
{
public:
    SpectrumPlotBaseTest() : TimedUnitTest ("Spectrum Plot Base Test") {}

    void coordinatesTest()
    {
        const auto checkCoordinates = [this] (chowdsp::SpectrumPlotParams params)
        {
            static constexpr int width = 400;
            static constexpr int height = 300;

            const auto xMinFreq = params.minFrequencyHz;
            const auto xMaxFreq = params.maxFrequencyHz;
            const auto xCenterFreq = std::sqrt (params.maxFrequencyHz * params.minFrequencyHz);

            const auto yMinMag = params.minMagnitudeDB;
            const auto yMaxMag = params.maxMagnitudeDB;
            const auto yCenterMag = 0.5f * (params.minMagnitudeDB + params.maxMagnitudeDB);

            chowdsp::SpectrumPlotBase component { std::move (params) };
            component.setSize (width, height);

            expectEquals (component.getXCoordinateForFrequency (xMinFreq), 0.0f, "Min freq. x-coord. is incorrect!");
            expectEquals (component.getXCoordinateForFrequency (xMaxFreq), (float) width, "Max freq. x-coord. is incorrect!");
            expectEquals (component.getXCoordinateForFrequency (xCenterFreq), (float) width * 0.5f, "Center freq. x-coord. is incorrect!");

            expectEquals (component.getYCoordinateForDecibels (yMinMag), (float) height, "Min mag. y-coord. is incorrect!");
            expectEquals (component.getYCoordinateForDecibels (yMaxMag), 0.0f, "Max mag. y-coord. is incorrect!");
            expectEquals (component.getYCoordinateForDecibels (yCenterMag), (float) height * 0.5f, "Center mag. y-coord. is incorrect!");

            expectWithinAbsoluteError (component.getFrequencyForXCoordinate (0.0f), xMinFreq, 1.0e-2f, "Min x-coord. frequency is incorrect!");
            expectWithinAbsoluteError (component.getFrequencyForXCoordinate ((float) width), xMaxFreq, 1.0e-2f, "Max x-coord. frequency is incorrect!");
            expectWithinAbsoluteError (component.getFrequencyForXCoordinate ((float) width * 0.5f), xCenterFreq, 1.0e-2f, "Center x-coord. frequency is incorrect!");
        };

        checkCoordinates ({ 20.0f, 20000.0f, -10.0f, 10.0f });
        checkCoordinates ({ 50.0f, 10000.0f, -30.0f, 30.0f });
        checkCoordinates ({ 100.0f, 5000.0f, -60.0f, 10.0f });
    }

    void drawCoordinatesTest()
    {
        struct TestComp : chowdsp::SpectrumPlotBase
        {
            TestComp() : chowdsp::SpectrumPlotBase ({ 20.0f, 20000.0f, -30.0f, 30.0f })
            {
                setSize (500, 300);
            }

            void paint (juce::Graphics& g) override
            {
                g.fillAll (juce::Colours::black);

                g.setColour (juce::Colours::white);
                drawFrequencyLines (g, { 100.0f, 1000.0f, 10000.0f });
                drawFrequencyLines (g, { 20.0f, 200.0f, 2000.0f, 20000.0f }, 1.0f, { 3.0f, 2.0f });

                drawMagnitudeLines (g, { -20.0f, -10.0f, 0.0f, 10.0f, 20.0f });
                drawMagnitudeLines (g, { -25.0f, -15.0f, -5.0f, 5.0f, 15.0f, 25.0f }, 1.0f, { 3.0f, 2.0f });
            }
        };

        TestComp comp {};
        const auto testScreenshot = comp.createComponentSnapshot ({ 500, 300 });
//        VizTestUtils::saveImage (testScreenshot, "freq_grid_plot.png");

        const auto refScreenshot = VizTestUtils::loadImage ("freq_grid_plot.png");
        VizTestUtils::compareImages (*this, testScreenshot, refScreenshot);
    }

    void runTestTimed() override
    {
        beginTest ("Coordinates Test");
        coordinatesTest();

        beginTest ("Draw Coordinates Test");
        drawCoordinatesTest();
    }
};

static SpectrumPlotBaseTest spectrumPlotBaseTest;
