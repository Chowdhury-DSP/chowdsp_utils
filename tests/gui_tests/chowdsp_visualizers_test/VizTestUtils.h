#include <CatchUtils.h>
#include <juce_graphics/juce_graphics.h>

namespace VizTestUtils
{
inline juce::File getImagesDir()
{
    auto imagesDir = juce::File::getCurrentWorkingDirectory();
    while (imagesDir.getFileName() != "chowdsp_utils")
        imagesDir = imagesDir.getParentDirectory();
    return imagesDir.getChildFile ("tests/gui_tests/chowdsp_visualizers_test/Images");
}

[[maybe_unused]] inline void saveImage (const juce::Image& testImage, const juce::String& fileName)
{
    juce::File testPNGFile = getImagesDir().getChildFile (fileName);
    testPNGFile.deleteFile();
    testPNGFile.create();
    auto pngStream = testPNGFile.createOutputStream();
    if (pngStream->openedOk())
    {
        juce::PNGImageFormat pngImage;
        pngImage.writeImageToStream (testImage, *pngStream);
    }
}

inline juce::Image loadImage (const juce::String& fileName)
{
    return juce::PNGImageFormat::loadFrom (VizTestUtils::getImagesDir()
                                               .getChildFile (fileName));
}

inline void compareImages (const juce::Image& testImage, const juce::Image& refImage)
{
    const auto width = testImage.getWidth();
    const auto height = testImage.getHeight();

    REQUIRE_MESSAGE (width == refImage.getWidth(), "Incorrect image width!");
    REQUIRE_MESSAGE (height == refImage.getHeight(), "Incorrect image height!");

    const auto testImageData = juce::Image::BitmapData { testImage, juce::Image::BitmapData::readOnly };
    const auto refImageData = juce::Image::BitmapData { refImage, juce::Image::BitmapData::readOnly };

    for (int w = 0; w < width; ++w)
    {
        for (int h = 0; h < height; ++h)
        {
            const auto testColour = testImageData.getPixelColour (w, h);
            const auto refColour = refImageData.getPixelColour (w, h);
            REQUIRE (juce::isWithin (testColour.getRed(), refColour.getRed(), (uint8_t) 25));
            REQUIRE (juce::isWithin (testColour.getBlue(), refColour.getBlue(), (uint8_t) 25));
            REQUIRE (juce::isWithin (testColour.getGreen(), refColour.getGreen(), (uint8_t) 25));
            REQUIRE (testColour.getAlpha() == refColour.getAlpha());
        }
    }
}
} // namespace VizTestUtils
