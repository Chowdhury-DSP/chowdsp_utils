#pragma once

namespace chowdsp
{
/** Abstract class to allow the derived class to listen for changes to a file. */
class FileListener : public juce::Timer
{
public:
    /**
     * Initialize this FileListener for a given file and update time.
     *
     * If the given update time is less than or equal to zero, then
     * the timer will not be started.
     */
    FileListener (const juce::File& file, int timerSeconds);

    ~FileListener() override;

    /** Override this class to do something when the file has changed. */
    virtual void listenerFileChanged() = 0;

    /** Returns the file that is currently being listened to. */
    [[nodiscard]] const juce::File& getListenerFile() const noexcept { return fileToListenTo; }

private:
    void timerCallback() override;

    const juce::File fileToListenTo;
    juce::int64 fileModificationTime = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListener)
};
} // namespace chowdsp
