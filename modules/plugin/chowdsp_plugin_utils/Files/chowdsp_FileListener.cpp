#include "chowdsp_FileListener.h"

namespace chowdsp
{
FileListener::FileListener (const juce::File& file, int timerSeconds) : fileToListenTo (file)
{
    fileModificationTime = fileToListenTo.getLastModificationTime().toMilliseconds();
    startTimer (timerSeconds * 1000);
}

void FileListener::timerCallback()
{
    auto newModificationTime = fileToListenTo.getLastModificationTime().toMilliseconds();

    if (newModificationTime < fileModificationTime)
    {
        jassertfalse; // file modification time moved backwards?
        return;
    }

    if (newModificationTime == fileModificationTime)
        return; // everything up-to-date

    // needs update!
    fileModificationTime = newModificationTime;
    listenerFileChanged();
}
} // namespace chowdsp
