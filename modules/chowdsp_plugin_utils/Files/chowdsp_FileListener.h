#pragma once

namespace chowdsp
{
class FileListener : private juce::Timer
{
public:
    FileListener (const juce::File& file, int timerSeconds);

    virtual void fileChanged (const juce::File& /*file*/) = 0;

    const juce::File& getFile() const noexcept { return fileToListenTo; }

private:
    void timerCallback() override;

    const juce::File fileToListenTo;
    juce::int64 fileModificationTime = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListener)
};
}
